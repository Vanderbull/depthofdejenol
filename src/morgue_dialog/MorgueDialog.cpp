#include "MorgueDialog.h"
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
    setMinimumWidth(380);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    // Header styling
    m_welcomeLabel = new QLabel(tr("Welcome to the Morgue!"));
    m_welcomeLabel->setStyleSheet("color: #2c3e50; font-weight: bold; font-size: 18px;");
    m_welcomeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_welcomeLabel);

    // Action Buttons Grid
    auto *buttonGrid = new QGridLayout();
    m_raiseBtn = new QPushButton(tr("Raise Character"));
    m_hireBtn = new QPushButton(tr("Hire Rescuers"));
    m_grabBtn = new QPushButton(tr("Grab Body"));

    buttonGrid->addWidget(m_raiseBtn, 0, 0);
    buttonGrid->addWidget(m_hireBtn, 0, 1);
    buttonGrid->addWidget(m_grabBtn, 1, 0, 1, 2); 

    mainLayout->addLayout(buttonGrid);
    mainLayout->addStretch();

    // Exit Button
    m_exitBtn = new QPushButton(tr("Exit"));
    mainLayout->addWidget(m_exitBtn, 0, Qt::AlignRight);

    // Signal/Slot Connections
    connect(m_raiseBtn, &QPushButton::clicked, this, &MorgueDialog::onActionClicked);
    connect(m_hireBtn, &QPushButton::clicked, this, &MorgueDialog::onActionClicked);
    connect(m_grabBtn, &QPushButton::clicked, this, &MorgueDialog::onActionClicked);
    
    // Direct connection to close() fixes the "undefined reference to onExitClicked" error
    connect(m_exitBtn, &QPushButton::clicked, this, &MorgueDialog::close);
}

QStringList MorgueDialog::fetchDeadCharacterFiles() const
{
    QSettings settings("MyCompany", "MyApp");
    QString subfolder = settings.value("Paths/SubfolderName", "data/characters").toString();
    QString basePath = QCoreApplication::applicationDirPath();
    QString fullPath = QDir::cleanPath(basePath + QDir::separator() + subfolder);

    QDir dir(fullPath);
    QStringList allFiles = dir.entryList({"*.txt"}, QDir::Files);
    QStringList deadCharacters;

    for (const QString &fileName : allFiles) {
        QFile file(dir.absoluteFilePath(fileName));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            bool characterIsDead = false;

            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                // Specifically matches the savegame format: isAlive: 0
                if (line == "isAlive: 0") {
                    characterIsDead = true;
                    break;
                }
            }
            if (characterIsDead) deadCharacters << fileName;
            file.close();
        }
    }
    return deadCharacters;
}

bool MorgueDialog::updateCharacterVitality(const QString &fileName, bool alive)
{
    QSettings settings("MyCompany", "MyApp");
    QString subfolder = settings.value("Paths/SubfolderName", "data/characters").toString();
    QString fullPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + 
                                     QDir::separator() + subfolder + 
                                     QDir::separator() + fileName);
    
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) return false;

    QString content = QString::fromUtf8(file.readAll());
    QString oldStatus = alive ? "isAlive: 0" : "isAlive: 1";
    QString newStatus = alive ? "isAlive: 1" : "isAlive: 0";

    if (!content.contains(oldStatus)) {
        file.close();
        return false; 
    }

    content.replace(oldStatus, newStatus);
    
    file.resize(0); // Clear file before rewriting 
    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}

void MorgueDialog::onActionClicked()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString actionName = btn->text();
    QStringList deadChars = fetchDeadCharacterFiles();

    if (deadChars.isEmpty()) {
        QMessageBox::warning(this, tr("Morgue Empty"), tr("No dead character files were found."));
        return;
    }

    bool ok;
    QString selected = QInputDialog::getItem(this, actionName, 
                                            tr("Select a character's body:"), 
                                            deadChars, 0, false, &ok);

    if (ok && !selected.isEmpty()) {
        m_selectedCharacter = selected;

        if (actionName == tr("Raise Character")) {
            if (updateCharacterVitality(m_selectedCharacter, true)) {
                QMessageBox::information(this, tr("Success"), 
                    tr("**%1** has been returned to the land of the living.").arg(m_selectedCharacter));
            } else {
                QMessageBox::critical(this, tr("Error"), tr("Could not update character file."));
            }
        } else {
            // Simulated response for other actions
            QMessageBox::information(this, tr("Morgue Action"), 
                tr("Action **%1** performed on **%2**.").arg(actionName, m_selectedCharacter));
        }
    }
}
