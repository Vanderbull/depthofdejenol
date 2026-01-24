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
    setWindowTitle(tr("Morgue"));
    setMinimumWidth(400);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    m_welcomeLabel = new QLabel(tr("Welcome to the Morgue!"));
    m_welcomeLabel->setStyleSheet("color: #2c3e50; font-weight: bold; font-size: 18px;");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_welcomeLabel);

    auto *buttonGrid = new QGridLayout();
    m_raiseBtn = new QPushButton(tr("Raise Character"));
    m_hireBtn = new QPushButton(tr("Hire Rescuers"));
    m_grabBtn = new QPushButton(tr("Grab Body"));

    buttonGrid->addWidget(m_raiseBtn, 0, 0);
    buttonGrid->addWidget(m_hireBtn, 0, 1);
    buttonGrid->addWidget(m_grabBtn, 1, 0, 1, 2); 

    mainLayout->addLayout(buttonGrid);
    mainLayout->addStretch();

    m_exitBtn = new QPushButton(tr("Exit"));
    mainLayout->addWidget(m_exitBtn, 0, Qt::AlignRight);

    connect(m_raiseBtn, &QPushButton::clicked, this, &MorgueDialog::onActionClicked);
    connect(m_hireBtn, &QPushButton::clicked, this, &MorgueDialog::onActionClicked);
    connect(m_grabBtn, &QPushButton::clicked, this, &MorgueDialog::onActionClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &MorgueDialog::close);
}

QList<DeadCharacterInfo> MorgueDialog::fetchDeadCharacterData() const
{
    QSettings settings("MyCompany", "MyApp");
    QString subfolder = settings.value("Paths/SubfolderName", "data/characters").toString();
    QString fullPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + subfolder);

    QDir dir(fullPath);
    QStringList allFiles = dir.entryList({"*.txt"}, QDir::Files);
    QList<DeadCharacterInfo> deadList;

    for (const QString &fileName : allFiles) {
        QFile file(dir.absoluteFilePath(fileName));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            DeadCharacterInfo info = {fileName, false, 1};
            bool isDead = false;

            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line == "isAlive: 0") isDead = true;
                else if (line.startsWith("inCity:")) info.inCity = (line.split(":")[1].trimmed().toInt() == 1);
                else if (line.startsWith("DungeonLevel:")) info.dungeonLevel = line.split(":")[1].trimmed().toInt();
            }
            if (isDead) deadList.append(info);
            file.close();
        }
    }
    return deadList;
}

int MorgueDialog::calculateRescueCost(int level) const
{
    return level * 1000; // 1000 Gold per level deep
}

bool MorgueDialog::moveBodyToCity(const QString &fileName)
{
    QSettings settings("MyCompany", "MyApp");
    QString subfolder = settings.value("Paths/SubfolderName", "data/characters").toString();
    QString filePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + subfolder + QDir::separator() + fileName);
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return false;

    QString content = QString::fromUtf8(file.readAll());
    content.replace("inCity: 0", "inCity: 1");
    
    file.resize(0);
    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}

bool MorgueDialog::resurrectCharacter(const QString &fileName)
{
    QSettings settings("MyCompany", "MyApp");
    QString subfolder = settings.value("Paths/SubfolderName", "data/characters").toString();
    QString filePath = QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + subfolder + QDir::separator() + fileName);
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return false;

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) lines.append(in.readLine());

    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].startsWith("isAlive:")) lines[i] = "isAlive: 1";
        else if (lines[i].startsWith("DungeonX:")) lines[i] = "DungeonX: 17";
        else if (lines[i].startsWith("DungeonY:")) lines[i] = "DungeonY: 12";
        else if (lines[i].startsWith("DungeonLevel:")) lines[i] = "DungeonLevel: 1";
        else if (lines[i].startsWith("inCity:")) lines[i] = "inCity: 1";
    }

    file.resize(0);
    QTextStream out(&file);
    for (const QString &line : lines) out << line << "\n";
    file.close();
    return true;
}

void MorgueDialog::onActionClicked()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString actionName = btn->text();
    QList<DeadCharacterInfo> deadChars = fetchDeadCharacterData();

    if (deadChars.isEmpty()) {
        QMessageBox::information(this, tr("Morgue"), tr("No fallen heroes found."));
        return;
    }

    QStringList names;
    for (const auto& info : deadChars) names << info.fileName;

    bool ok;
    QString selected = QInputDialog::getItem(this, actionName, tr("Select a Body:"), names, 0, false, &ok);
    if (!ok || selected.isEmpty()) return;

    DeadCharacterInfo selectedInfo;
    for (const auto& info : deadChars) if (info.fileName == selected) selectedInfo = info;

    if (actionName == tr("Hire Rescuers")) {
        int cost = calculateRescueCost(selectedInfo.dungeonLevel);
        // Using Party index 0 as the active player paying the fee
        auto* gsm = GameStateManager::instance();
        
        // This assumes your GameStateManager tracks gold via m_gameStateData or PC index 0
        // Adjust "Gold" key to match your specific implementation
        qulonglong currentGold = gsm->instance()->getPC()[0].gold; 

        if (currentGold < (qulonglong)cost) {
            QMessageBox::warning(this, tr("Insufficient Gold"), 
                tr("Retrieving a body from Level %1 costs %2 Gold. You only have %3.")
                .arg(selectedInfo.dungeonLevel).arg(cost).arg(currentGold));
            return;
        }

        if (QMessageBox::Yes == QMessageBox::question(this, tr("Confirm"), tr("Pay %1 Gold to rescue %2?").arg(cost).arg(selected))) {
            gsm->updateCharacterGold(0, (qulonglong)cost, false); // Deduct gold
            if (moveBodyToCity(selected)) {
                QMessageBox::information(this, tr("Success"), tr("The body has been recovered and is now in the city."));
            }
        }
    } 
    else {
        // Raise or Grab logic: Character MUST be in city
        if (!selectedInfo.inCity) {
            QMessageBox::critical(this, tr("Body Missing"), tr("That body is still in the dungeon! You must hire rescuers first."));
            return;
        }

        if (actionName == tr("Raise Character")) {
            if (resurrectCharacter(selected)) {
                QMessageBox::information(this, tr("Raised"), tr("%1 has returned to life at the city gates.").arg(selected));
            }
        } else {
            QMessageBox::information(this, tr("Morgue"), tr("Body of %1 grabbed.").arg(selected));
        }
    }
}
