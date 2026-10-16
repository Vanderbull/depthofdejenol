#include "MorgueDialog.h"
#include "src/core/GoldSinks.h"
#include "src/core/gameStateManager.h"
#include "src/core/DeathRecovery.h"
#include <QtWidgets>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

MorgueDialog::MorgueDialog(QWidget *parent) : QDialog(parent)
{
    setupUi();
}

MorgueDialog::~MorgueDialog() = default;

void MorgueDialog::setupUi()
{
    setWindowTitle(tr("The Morgue"));
    setMinimumWidth(450);

    auto *mainLayout = new QVBoxLayout(this);
    m_welcomeLabel = new QLabel(tr("Morgue Services"));
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    m_welcomeLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2c3e50;");
    mainLayout->addWidget(m_welcomeLabel);

    auto *buttonGrid = new QGridLayout();
    m_hireBtn = new QPushButton(tr("Hire Rescuers"));
    m_grabBtn = new QPushButton(tr("Grab Body"));
    m_raiseBtn = new QPushButton(tr("Raise Character"));

    buttonGrid->addWidget(m_hireBtn, 0, 0);
    buttonGrid->addWidget(m_grabBtn, 0, 1);
    buttonGrid->addWidget(m_raiseBtn, 1, 0, 1, 2);
    mainLayout->addLayout(buttonGrid);

    m_exitBtn = new QPushButton(tr("Exit"));
    mainLayout->addWidget(m_exitBtn, 0, Qt::AlignRight);

    connect(m_hireBtn, &QPushButton::clicked, this, &MorgueDialog::onActionClicked);
    connect(m_grabBtn, &QPushButton::clicked, this, &MorgueDialog::onActionClicked);
    connect(m_raiseBtn, &QPushButton::clicked, this, &MorgueDialog::onActionClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &MorgueDialog::close);
}

QList<DeadCharacterInfo> MorgueDialog::fetchDeadCharacterData() const
{
    QDir dir("data/characters");
    QStringList allFiles = dir.entryList({"*.json"}, QDir::Files);
    QList<DeadCharacterInfo> deadList;

    for (const QString &fileName : allFiles) {
        QFile file(dir.absoluteFilePath(fileName));
        if (!file.open(QIODevice::ReadOnly)) continue;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (!doc.isObject()) continue;

        QVariantMap c = doc.toVariant().toMap();
        DeadCharacterInfo info = {fileName, false, 0};
        info.inCity = c.value("inCity", false).toBool();
        info.dungeonLevel = c.value("DungeonLevel").toInt();

        if (!c.value("isAlive", true).toBool()) deadList.append(info);
    }
    return deadList;
}

bool MorgueDialog::moveBodyToCityInFile(const QString &fileName)
{
    QString filePath = "data/characters/" + fileName;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) return false;

    QVariantMap c = doc.toVariant().toMap();
    c["inCity"] = true;
    c["isAlive"] = true;
    c["DungeonLevel"] = 0;
    c["DungeonX"] = 17;
    c["DungeonY"] = 12;
    // A revived character must not stay at 0 HP, or it loads as dead again.
    if (c.value("MaxHP").toInt() <= 0) c["MaxHP"] = 10;
    if (c.value("HP").toInt() <= 0) c["HP"] = c.value("MaxHP");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    file.write(QJsonDocument(QJsonObject::fromVariantMap(c)).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool MorgueDialog::updateCharacterFile(const QString &fileName)
{
    return moveBodyToCityInFile(fileName);
}

int MorgueDialog::calculateRescueCost(int level) const {
    // Rescue cost scales superlinearly with depth (see DeathRecovery).
    return GoldSinks::rescueCost(level);
}

int MorgueDialog::raiseCost(int characterLevel, bool bodyInCity) {
    // GoldSinks::resurrectionCost takes bodyInDungeon, which is the inverse.
    return GoldSinks::resurrectionCost(characterLevel, !bodyInCity);
}

void MorgueDialog::onActionClicked()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    QString action = btn->text();
    auto* gsm = gameStateManager::instance();

    QList<DeadCharacterInfo> deadChars = fetchDeadCharacterData();
    if (deadChars.isEmpty()) {
        QMessageBox::information(this, tr("Morgue"), tr("No dead heroes found."));
        return;
    }
    QStringList names;
    for (const auto& info : deadChars) names << info.fileName;
    bool ok;
    QString selected = QInputDialog::getItem(this, action, tr("Select Hero:"), names, 0, false, &ok);
    if (!ok || selected.isEmpty()) return;

    DeadCharacterInfo currentInfo;
    for (const auto& info : deadChars) if (info.fileName == selected) { currentInfo = info; break; }

    if (action == tr("Hire Rescuers")) {
        int cost = calculateRescueCost(currentInfo.dungeonLevel);

        if (!currentInfo.inCity) {
            cost += 500;
        }

        qulonglong currentGold = gsm->getPC().at(0).gold;

        if (currentGold < (qulonglong)cost) {
            QMessageBox::warning(this, tr("Funds"), tr("Rescue requires %1 gold.").arg(cost));
            return;
        }
        if (QMessageBox::Yes == QMessageBox::question(this, tr("Hire"), tr("Pay %1 to rescue %2?").arg(cost).arg(selected))) {
            gsm->updateCharacterGold(0, (qulonglong)cost, false);
            if (moveBodyToCityInFile(selected)) {
                QMessageBox::information(this, tr("Success"), tr("The body is now at the city gates."));
            }
        }
    }
    else if (action == tr("Grab Body")) {
        if (!moveBodyToCityInFile(selected)) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to update character save file."));
            return;
        }

        QString nameOnly = selected;
        if (nameOnly.endsWith(".json")) nameOnly.chop(5);

        if (gsm->loadCharacterFromFile(nameOnly)) {
            QMessageBox::information(this, tr("Success"), tr("%1 has been brought to the city, revived, and placed on the altar.").arg(nameOnly));
        }
    }
    else if (action == tr("Raise Character")) {
        QString activeName = gsm->getPC().at(0).name;
        if (activeName + ".json" != selected) {
            QMessageBox::warning(this, tr("Not Ready"), tr("You must 'Grab Body' for %1 first.").arg(selected));
            return;
        }
        if (!gsm->isActiveCharacterInCity()) {
            QMessageBox::critical(this, tr("Error"), tr("%1 is not in the city!").arg(selected));
            return;
        }

        // Raising the dead costs gold via GoldSinks; the body is in town here,
        // so the cheaper in-city rate applies.
        int level = gsm->getPC().at(0).level;
        int raiseFee = raiseCost(level, true);
        if (gsm->getPartyGold() < raiseFee) {
            QMessageBox::warning(this, tr("Funds"),
                tr("Raising %1 costs %2 gold (you have %3).")
                    .arg(selected).arg(raiseFee).arg(gsm->getPartyGold()));
            return;
        }
        if (QMessageBox::Yes != QMessageBox::question(this, tr("Raise"),
                tr("Pay %1 gold to raise %2?").arg(raiseFee).arg(selected))) {
            return;
        }
        gsm->spendPartyGold(raiseFee);

        if (updateCharacterFile(selected)) {
            QString nameOnly = selected;
            if (nameOnly.endsWith(".json")) nameOnly.chop(5);
            gsm->loadCharacterFromFile(nameOnly);
            QMessageBox::information(this, tr("Success"), tr("%1 has returned to life!").arg(nameOnly));
        }
    }
}
