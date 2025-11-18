#include "MorgueDialog.h"
#include <QMessageBox>
#include <QComboBox>
#include <QDir>
#include <QFileInfoList>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>

// --- Helper function to find a list of 'dead' character files ---
// This mimics the file-finding logic from GameMenu::loadGame
QStringList findDeadCharacterFiles() {
    // 1. Simulate finding the character data path
    QSettings settings("MyCompany", "MyApp");
    QString subfolderName = settings.value("Paths/SubfolderName", "data/characters").toString();
    QString basePath = QCoreApplication::applicationDirPath();
    QString fullPath = QDir::cleanPath(basePath + QDir::separator() + subfolderName);
    QDir currentDir(fullPath);

    if (!currentDir.exists()) {
        qDebug() << "Character subfolder not found for morgue file listing.";
        return {}; // Return empty list
    }

    // 2. Filter for .txt files (Simulating character files)
    QStringList nameFilters;
    nameFilters << "*.txt"; // Assume dead characters are also .txt files
    QFileInfoList fileList = currentDir.entryInfoList(nameFilters, QDir::Files);
    
    QStringList characterFiles;
    for (const QFileInfo &fileInfo : fileList) {
        // In a real game, you would check a flag in the file 
        // to see if the character is *actually* dead.
        // For this example, we just list all found files.
        characterFiles << fileInfo.fileName();
    }
    return characterFiles;
}

// --- Helper function to show the character selection dialog ---
bool MorgueDialog::showCharacterSelectionDialog(const QString &actionName) {
    QStringList characterFiles = findDeadCharacterFiles();

    if (characterFiles.isEmpty()) {
        QMessageBox::warning(this, tr("No Bodies Found"), 
                             tr("No dead character files (*.txt) were found in the current directory."));
        return false;
    }
    
    // Create Temporary Selection Dialog
    QDialog selectionDialog(this);
    selectionDialog.setWindowTitle(actionName + " Character");
    
    QVBoxLayout *layout = new QVBoxLayout(&selectionDialog);
    
    QLabel *promptLabel = new QLabel(QString("Select a character's body for: **%1**").arg(actionName));
    layout->addWidget(promptLabel);
    
    QComboBox *charComboBox = new QComboBox();
    charComboBox->addItems(characterFiles);
    layout->addWidget(charComboBox);
    
    QPushButton *confirmButton = new QPushButton(actionName);
    layout->addWidget(confirmButton);
    
    QPushButton *cancelButton = new QPushButton("Cancel");
    layout->addWidget(cancelButton);

    // Connect dialog buttons
    QObject::connect(confirmButton, &QPushButton::clicked, &selectionDialog, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &selectionDialog, &QDialog::reject);

    // Execute the dialog modally
    int result = selectionDialog.exec();
    
    if (result == QDialog::Accepted) {
        // User clicked the action button
        m_selectedDeadCharacter = charComboBox->currentText();
        qDebug() << "Character selected for" << actionName << ":" << m_selectedDeadCharacter;
        return true;
    } else {
        // User cancelled
        qDebug() << "Morgue action" << actionName << "cancelled by user.";
        m_selectedDeadCharacter.clear();
        return false;
    }
}


MorgueDialog::MorgueDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("Morgue");

    // Welcome Label
    welcomeLabel = new QLabel("Welcome to the Morgue!");
    welcomeLabel->setStyleSheet("color: blue; font-weight: bold; font-size: 16px;");
    welcomeLabel->setAlignment(Qt::AlignCenter);

    // Buttons
    raiseCharacterButton = new QPushButton("Raise Character");
    hireRescuersButton = new QPushButton("Hire Rescuers");
    grabBodyButton = new QPushButton("Grab Body");
    exitButton = new QPushButton("Exit");

    // Connect signals to slots
    connect(raiseCharacterButton, &QPushButton::clicked, this, &MorgueDialog::onRaiseCharacterClicked);
    connect(hireRescuersButton, &QPushButton::clicked, this, &MorgueDialog::onHireRescuersClicked);
    connect(grabBodyButton, &QPushButton::clicked, this, &MorgueDialog::onGrabBodyClicked);
    connect(exitButton, &QPushButton::clicked, this, &MorgueDialog::onExitClicked);

    // Layouts
    QHBoxLayout *topButtonLayout = new QHBoxLayout();
    topButtonLayout->addWidget(raiseCharacterButton);
    topButtonLayout->addWidget(hireRescuersButton);

    QHBoxLayout *middleButtonLayout = new QHBoxLayout();
    middleButtonLayout->addWidget(grabBodyButton);
    //middleButtonLayout->addStretch(); // Pushes "Grab Body" to the left

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addLayout(topButtonLayout);
    mainLayout->addLayout(middleButtonLayout);
    mainLayout->addStretch(); // Pushes everything upwards
    mainLayout->addWidget(exitButton, 0, Qt::AlignRight); // Align Exit button to the right

    setLayout(mainLayout);

    // Set a fixed size to mimic the image's appearance more closely
    //setFixedSize(300, 200);
}

MorgueDialog::~MorgueDialog()
{
}

void MorgueDialog::onRaiseCharacterClicked()
{
    // 1. Show the selection dialog
    if (showCharacterSelectionDialog("Raise Character")) {
        // 2. Execute the action if a character was selected
        QMessageBox::information(this, "Morgue Action", 
                                 QString("Attempting to **Raise** character: **%1** (Action simulated).").arg(m_selectedDeadCharacter));
        // Real logic (e.g., character data modification) would go here
    }
}

void MorgueDialog::onHireRescuersClicked()
{
    // 1. Show the selection dialog
    if (showCharacterSelectionDialog("Hire Rescuers")) {
        // 2. Execute the action if a character was selected
        QMessageBox::information(this, "Morgue Action", 
                                 QString("Hiring rescuers for body of: **%1** (Action simulated).").arg(m_selectedDeadCharacter));
        // Real logic (e.g., cost calculation, character data modification) would go here
    }
}

void MorgueDialog::onGrabBodyClicked()
{
    // 1. Show the selection dialog
    if (showCharacterSelectionDialog("Grab Body")) {
        // 2. Execute the action if a character was selected
        QMessageBox::information(this, "Morgue Action", 
                                 QString("Body of **%1** grabbed (Action simulated).").arg(m_selectedDeadCharacter));
        // Real logic (e.g., body retrieval, inventory update) would go here
    }
}

void MorgueDialog::onExitClicked()
{
    close(); // Close the dialog
}
