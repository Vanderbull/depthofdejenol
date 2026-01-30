#include "optionsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <QDateTime>

void OptionsDialog::onNoMusicToggled(bool checked) 
{
    if (checked) {
        GameStateManager::instance()->stopMusic();
        qDebug() << "Music muted. Sound FX slider remains at current level.";
    } else {
        GameStateManager::instance()->playMusic("resources/waves/main.wav");
        qDebug() << "Music restored";
    }
}
/**
 * @brief Copies all files from a source directory to a "backup/YYYY-MM-DD" subfolder within it.
 * * It ignores subdirectories within the source folder and only copies individual files.
 * If a file with the same name already exists in the backup destination, it is overwritten.
 * * @param sourceDirPath The absolute or relative path to the directory to backup.
 * @return true if the operation was completely successful, false if errors occurred.
 */
bool backupFilesToDateSubfolder(const QString& sourceDirPath) {
    QDir sourceDir(sourceDirPath);
    // 1. Validate the source directory
    if (!sourceDir.exists()) {
        qDebug() << "Error: Source directory does not exist:" << sourceDirPath;
        return false;
    }
    // 2. Get the current date formatted as "YYYY-MM-DD"
    QString dateString = QDate::currentDate().toString("yyyy-MM-dd");
    // 3. Define the full destination path: sourceDir/backup/YYYY-MM-DD
    QString backupBaseDirPath = sourceDir.absoluteFilePath("backup");
    QString finalBackupDirPath = backupBaseDirPath + QDir::separator() + dateString;
    QDir finalBackupDir(finalBackupDirPath);
    // 4. Create the full directory path (backup/YYYY-MM-DD)
    // QDir::mkpath will create the entire path structure recursively.
    if (!finalBackupDir.mkpath(".")) {
        qDebug() << "Error: Could not create date-stamped backup directory:" << finalBackupDirPath;
        return false;
    }
    qDebug() << "Destination folder path:" << finalBackupDirPath;
    // 5. Get a list of files from the source directory
    // QDir::Files | QDir::NoDotAndDotDot ensures we only process files and ignore "." and ".."
    QStringList files = sourceDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    bool allSuccessful = true;
    if (files.isEmpty()) {
        qDebug() << "No files found to backup in:" << sourceDirPath;
    }
    // 6. Iterate through the files and copy them
    for (const QString& fileName : files) {
        QString sourceFilePath = sourceDir.absoluteFilePath(fileName);
        QString destFilePath = finalBackupDir.absoluteFilePath(fileName);

        // Remove existing file in destination before copying to ensure a fresh copy (overwrite)
        if (QFile::exists(destFilePath)) {
            if (!QFile::remove(destFilePath)) {
                qDebug() << "Warning: Could not remove existing file (will not copy):" << destFilePath;
                allSuccessful = false;
                continue; 
            }
        }
        // The core copying operation
        if (QFile::copy(sourceFilePath, destFilePath)) {
            qDebug() << "Copied:" << fileName;
        } else {
            qDebug() << "Error copying file:" << fileName;
            allSuccessful = false;
        }
    }
    // 7. Final status message
    if (allSuccessful) {
        qDebug() << "\n✅ File backup operation completed successfully to " << dateString << " folder!";
    } else {
        qDebug() << "\n⚠️ File backup operation completed with some errors.";
    }
    return allSuccessful;
}

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Options");
    setFixedSize(850, 550);
    // Set the default font values
    currentPropFont = QFont("MS Sans Serif", 8); 
    currentFixedFont = QFont("Courier New", 9); // Keeping your original Courier default
    setupUi();
    // Connect the buttons to their slots
    connect(okButton, &QPushButton::clicked, this, &OptionsDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &OptionsDialog::onCancelClicked);
    connect(noMusicCheckBox, &QCheckBox::toggled, this, &OptionsDialog::onNoMusicToggled);
    connect(defaultButton, &QPushButton::clicked, this, &OptionsDialog::onDefaultClicked);
    // Connect the new Backup/Restore buttons
    connect(backupButton, &QPushButton::clicked, this, &OptionsDialog::onBackupClicked);
    connect(restoreButton, &QPushButton::clicked, this, &OptionsDialog::onRestoreClicked);
}

void OptionsDialog::setupUi() 
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QGridLayout *topGrid = new QGridLayout();
    // Game Settings Group
    QGroupBox *gameSettingsBox = new QGroupBox("Game Settings");
    QGridLayout *gameSettingsLayout = new QGridLayout(gameSettingsBox);
    noPartySoundCheckBox = new QCheckBox("No Party Sound");
    noMsgDingCheckBox = new QCheckBox("No Msg. Ding");
    noHelpSoundCheckBox = new QCheckBox("No HELP Sound");
    noToolbarCheckBox = new QCheckBox("No Toolbar");
    noAutomapCursorBlinkCheckBox = new QCheckBox("No Automap Cursor Blink");
    no3DCheckBox = new QCheckBox("No 3-D");
    noStretchBltCheckBox = new QCheckBox("No StretchBlt");
    noLoadAutomapCheckBox = new QCheckBox("No Load Automap");
    noAutosaveCheckBox = new QCheckBox("No Autosave");
    
    gameSettingsLayout->addWidget(noPartySoundCheckBox, 0, 0);
    gameSettingsLayout->addWidget(no3DCheckBox, 0, 1);
    gameSettingsLayout->addWidget(noMsgDingCheckBox, 1, 0);
    gameSettingsLayout->addWidget(noStretchBltCheckBox, 1, 1);
    gameSettingsLayout->addWidget(noHelpSoundCheckBox, 2, 0);
    gameSettingsLayout->addWidget(noLoadAutomapCheckBox, 2, 1);
    gameSettingsLayout->addWidget(noToolbarCheckBox, 3, 0);
    gameSettingsLayout->addWidget(noAutosaveCheckBox, 3, 1);
    gameSettingsLayout->addWidget(noAutomapCursorBlinkCheckBox, 4, 0);
    topGrid->addWidget(gameSettingsBox, 0, 0);
    
    // Font Settings Group
    QGroupBox *fontSettingsBox = new QGroupBox("Font Settings");
    QVBoxLayout *fontSettingsLayout = new QVBoxLayout(fontSettingsBox);

    fontSettingsLayout->addWidget(new QLabel("Proportional Font"));
    
    // Create the button and set its text to the default font name
    propFontButton = new QPushButton(currentPropFont.family() + " (" + QString::number(currentPropFont.pointSize()) + ")");
    fontSettingsLayout->addWidget(propFontButton);

    fontSettingsLayout->addWidget(new QLabel("Non-Proportional Font"));
    fixedFontButton = new QPushButton(currentFixedFont.family() + " (" + QString::number(currentFixedFont.pointSize()) + ")");
    fontSettingsLayout->addWidget(fixedFontButton);

    defaultButton = new QPushButton("Default");
    fontSettingsLayout->addWidget(defaultButton, 0, Qt::AlignCenter);
    
    topGrid->addWidget(fontSettingsBox, 0, 1);
    // Connect font buttons
    connect(propFontButton, &QPushButton::clicked, this, &OptionsDialog::onSelectProportionalFont);
    connect(fixedFontButton, &QPushButton::clicked, this, &OptionsDialog::onSelectFixedFont); 

    // --- Combined Audio Settings Group ---
    QGroupBox *audioBox = new QGroupBox("Music & Sound Settings");
    QVBoxLayout *audioLayout = new QVBoxLayout(audioBox);
    // 1. Music Volume Section
    audioLayout->addWidget(new QLabel("Music Vol."));
    musicVolSlider = new QSlider(Qt::Horizontal);
    musicVolSlider->setRange(0, 100);
    musicVolSlider->setValue(static_cast<int>(GameStateManager::instance()->getVolume() * 100.0f));
    
    connect(musicVolSlider, &QSlider::valueChanged, this, [](int value) {
        float vol = static_cast<float>(value) / 100.0f;
        GameStateManager::instance()->setVolume(vol);
    });

    noMusicCheckBox = new QCheckBox("No Music");
    connect(noMusicCheckBox, &QCheckBox::toggled, this, &OptionsDialog::onNoMusicToggled);

    QHBoxLayout *musicRow = new QHBoxLayout();
    musicRow->addWidget(musicVolSlider);
    musicRow->addWidget(noMusicCheckBox);
    audioLayout->addLayout(musicRow);
    // 2. SFX Volume Section (Now inside the same group)
    audioLayout->addWidget(new QLabel("Sfx Vol."));
    sfxVolSlider = new QSlider(Qt::Horizontal);
    sfxVolSlider->setRange(0, 100);
    sfxVolSlider->setValue(50); // Default

    connect(sfxVolSlider, &QSlider::valueChanged, this, [](int value) {
        float vol = static_cast<float>(value) / 100.0f;
        // Placeholder for GameStateManager SFX logic
    });

    noSoundFxCheckBox = new QCheckBox("No Sound FX");
    QHBoxLayout *sfxRow = new QHBoxLayout();
    sfxRow->addWidget(sfxVolSlider);
    sfxRow->addWidget(noSoundFxCheckBox);
    audioLayout->addLayout(sfxRow);

    mainLayout->addWidget(audioBox);
    topGrid->addWidget(audioBox, 1, 0);
    // File Backups Group
    QGroupBox *fileBackupsBox = new QGroupBox("File Backups");
    QVBoxLayout *fileBackupsLayout = new QVBoxLayout(fileBackupsBox);
    lastBackupLabel = new QLabel(""); // Placeholder date
    backupButton = new QPushButton("Backup");
    restoreButton = new QPushButton("Restore");
    fileBackupsLayout->addWidget(lastBackupLabel, 0, Qt::AlignCenter);
    fileBackupsLayout->addWidget(backupButton);
    fileBackupsLayout->addWidget(restoreButton);
    topGrid->addWidget(fileBackupsBox, 1, 1);
    mainLayout->addLayout(topGrid);
    // OK and Cancel Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    okButton = new QPushButton("OK");
    cancelButton = new QPushButton("Cancel");
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
}

void OptionsDialog::onOkClicked() {
    accept();
}

void OptionsDialog::onCancelClicked() {
    reject();
}
void OptionsDialog::onDefaultClicked() {
    // 1. Reset the font variables to the requested defaults
    currentPropFont = QFont("MS Sans Serif", 8);
    currentFixedFont = QFont("Courier New", 9);

    // 2. Update the button labels to reflect the reset
    propFontButton->setText(currentPropFont.family() + " (" + QString::number(currentPropFont.pointSize()) + ")");
    fixedFontButton->setText(currentFixedFont.family() + " (" + QString::number(currentFixedFont.pointSize()) + ")");

    // 3. (Optional) Apply the default font to the application immediately
    // QGuiApplication::setFont(currentPropFont);

    qDebug() << "Fonts reset to default: MS Sans Serif and Courier New";
}

/**
 * @brief Restores files from the "backup/YYYY-MM-DD" subfolder back to the source directory.
 */
bool restoreFilesFromDateSubfolder(const QString& sourceDirPath) {
    QString dateString = QDate::currentDate().toString("yyyy-MM-dd");
    QString backupPath = sourceDirPath + "/backup/" + dateString;
    QDir backupDir(backupPath);

    if (!backupDir.exists()) {
        qDebug() << "Error: No backup found for today at" << backupPath;
        return false;
    }

    QStringList files = backupDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    bool allSuccessful = true;

    for (const QString& fileName : files) {
        QString src = backupDir.absoluteFilePath(fileName);
        QString dest = QDir(sourceDirPath).absoluteFilePath(fileName);

        if (QFile::exists(dest)) QFile::remove(dest);
        if (QFile::copy(src, dest)) {
            qDebug() << "Restored:" << fileName;
        } else {
            allSuccessful = false;
        }
    }
    return allSuccessful;
}

// --- New Slot Implementations ---

void OptionsDialog::onBackupClicked() {
    qDebug() << "--- Starting Backup ---";
    if (backupFilesToDateSubfolder(".")) {
        lastBackupLabel->setText("Last Backup: " + QDate::currentDate().toString("yyyy-MM-dd"));
    }
}

void OptionsDialog::onRestoreClicked() {
    qDebug() << "--- Starting Restore ---";
    if (restoreFilesFromDateSubfolder(".")) {
        qDebug() << "Restore complete.";
    }
}
void OptionsDialog::onSelectProportionalFont() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, currentPropFont, this, "Select Proportional Font");
    if (ok) {
        currentPropFont = font;
        propFontButton->setText(font.family() + " (" + QString::number(font.pointSize()) + ")");
        
        // Apply to Game - Assuming GameStateManager handles global styling
        // If you want to change the whole app:
        // QGuiApplication::setFont(font); 
        
        qDebug() << "Proportional font updated to:" << font.family();
    }
}

void OptionsDialog::onSelectFixedFont() {
    bool ok;
    // We add the 'Monospace' filter to help users pick a fixed-width font
    QFontDialog dialog(currentFixedFont, this);
    dialog.setWindowTitle("Select Fixed-Width Font");
    
    if (dialog.exec() == QDialog::Accepted) {
        QFont font = dialog.selectedFont();
        currentFixedFont = font;
        fixedFontButton->setText(font.family() + " (" + QString::number(font.pointSize()) + ")");
        
        // Placeholder: GameStateManager::instance()->setGameFont(font);
        qDebug() << "Fixed font updated to:" << font.family();
    }
}


OptionsDialog::~OptionsDialog() {}

