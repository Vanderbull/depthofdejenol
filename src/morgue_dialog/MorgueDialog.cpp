#include "MorgueDialog.h"
#include "GameStateManager.h"
#include <QtWidgets>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QTextStream>

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
    QStringList allFiles = dir.entryList({"*.txt"}, QDir::Files);
    QList<DeadCharacterInfo> deadList;

    for (const QString &fileName : allFiles) {
        QFile file(dir.absoluteFilePath(fileName));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            DeadCharacterInfo info = {fileName, false, 0};
            bool isDead = false;
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.startsWith("isAlive: 0")) isDead = true;
                else if (line.startsWith("inCity: 1")) info.inCity = true;
                else if (line.startsWith("DungeonLevel:")) info.dungeonLevel = line.split(":")[1].trimmed().toInt();
            }
            if (isDead) deadList.append(info);
            file.close();
        }
    }
    return deadList;
}

bool MorgueDialog::moveBodyToCityInFile(const QString &fileName)
{
    QString filePath = "data/characters/" + fileName;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("inCity:")) line = "inCity: 1";
        else if (line.startsWith("DungeonLevel:")) line = "DungeonLevel: 0";
        else if (line.startsWith("DungeonX:")) line = "DungeonX: 17";
        else if (line.startsWith("DungeonY:")) line = "DungeonY: 12";
        lines.append(line);
    }
    file.close();

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return false;
    QTextStream out(&file);
    for (const QString &l : lines) out << l << "\n";
    file.close();
    return true;
}

bool MorgueDialog::updateCharacterFile(const QString &fileName, bool resurrect)
{
    QString filePath = "data/characters/" + fileName;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (resurrect && line.startsWith("isAlive:")) line = "isAlive: 1";
        if (line.startsWith("inCity:")) line = "inCity: 1";
        if (line.startsWith("DungeonLevel:")) line = "DungeonLevel: 0";
        lines.append(line);
    }
    file.close();

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return false;
    QTextStream out(&file);
    for (const QString &l : lines) out << l << "\n";
    file.close();
    return true;
}

int MorgueDialog::calculateRescueCost(int level) const { 
    return (level <= 0) ? 1000 : level * 1000; 
}

void MorgueDialog::onActionClicked()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    QString action = btn->text();
    auto* gsm = GameStateManager::instance();
    // 1. Refresh data from files immediately on click
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
        qulonglong currentGold = gsm->getPC().at(0).Gold;

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
        // Validation using refreshed file data
        if (!currentInfo.inCity && currentInfo.dungeonLevel != 0) {
            QMessageBox::critical(this, tr("Error"), tr("The body of %1 is still in the dungeon!").arg(selected));
            return;
        }
        QString nameOnly = selected;
        if (nameOnly.endsWith(".txt")) nameOnly.chop(4);

        if (gsm->loadCharacterFromFile(nameOnly)) {
            QMessageBox::information(this, tr("Ready"), tr("%1 has been placed on the altar.").arg(nameOnly));
        }
    }
    else if (action == tr("Raise Character")) {
        // Compare active character name to selection
        QString activeName = gsm->getPC().at(0).name;
        if (activeName + ".txt" != selected) {
            QMessageBox::warning(this, tr("Not Ready"), tr("You must 'Grab Body' for %1 first.").arg(selected));
            return;
        }
        // Use GameStateManager live memory to check level
        if (!gsm->isActiveCharacterInCity()) {
            QMessageBox::critical(this, tr("Error"), tr("%1 is not in the city!").arg(selected));
            return;
        }
        if (updateCharacterFile(selected, true)) {
            QString nameOnly = selected;
            nameOnly.remove(".txt");
            gsm->loadCharacterFromFile(nameOnly);
            QMessageBox::information(this, tr("Success"), tr("%1 has returned to life!").arg(nameOnly));
        }
    }
}
