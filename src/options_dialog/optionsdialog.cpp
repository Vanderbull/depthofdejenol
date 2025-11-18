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
    setFixedSize(550, 450); // Adjust size to match the screenshot

    setupUi();
    
    // Connect the buttons to their slots
    connect(okButton, &QPushButton::clicked, this, &OptionsDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &OptionsDialog::onCancelClicked);
    connect(defaultButton, &QPushButton::clicked, this, &OptionsDialog::onDefaultClicked);
}

OptionsDialog::~OptionsDialog() {}

void OptionsDialog::setupUi() {
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
    // Add font-related widgets here
    fontSettingsLayout->addWidget(new QLabel("Non-Proportional Font"));
    fontSettingsLayout->addWidget(new QLabel("Courier New (9)")); // Placeholder
    fontSettingsLayout->addWidget(new QLabel("Proportional Font"));
    fontSettingsLayout->addWidget(new QLabel("MS Sans Serif (8)")); // Placeholder
    defaultButton = new QPushButton("Default");
    fontSettingsLayout->addWidget(defaultButton, 0, Qt::AlignCenter);
    topGrid->addWidget(fontSettingsBox, 0, 1);

    // Music Settings Group
    QGroupBox *musicSettingsBox = new QGroupBox("Music Settings");
    QVBoxLayout *musicSettingsLayout = new QVBoxLayout(musicSettingsBox);
    musicSettingsLayout->addWidget(new QLabel("Music Vol."));
    musicVolSlider = new QSlider(Qt::Horizontal);
    noMusicCheckBox = new QCheckBox("No Music");
    QHBoxLayout *musicSliderLayout = new QHBoxLayout();
    musicSliderLayout->addWidget(musicVolSlider);
    musicSliderLayout->addWidget(noMusicCheckBox);
    musicSettingsLayout->addLayout(musicSliderLayout);

    musicSettingsLayout->addWidget(new QLabel("Sfx Vol."));
    sfxVolSlider = new QSlider(Qt::Horizontal);
    noSoundFxCheckBox = new QCheckBox("No Sound FX");
    QHBoxLayout *sfxSliderLayout = new QHBoxLayout();
    sfxSliderLayout->addWidget(sfxVolSlider);
    sfxSliderLayout->addWidget(noSoundFxCheckBox);
    musicSettingsLayout->addLayout(sfxSliderLayout);
    
    useSoundCanvasMIDICheckBox = new QCheckBox("Huse Sound Canvas MIDI");
    musicSettingsLayout->addWidget(useSoundCanvasMIDICheckBox);
    topGrid->addWidget(musicSettingsBox, 1, 0);

    // File Backups Group
    QGroupBox *fileBackupsBox = new QGroupBox("File Backups");
    QVBoxLayout *fileBackupsLayout = new QVBoxLayout(fileBackupsBox);
    lastBackupLabel = new QLabel("Last Backup was on 8/9/2014"); // Placeholder date
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
    // Logic to save settings
    accept(); // Closes the dialog with QDialog::Accepted
}

void OptionsDialog::onCancelClicked() {
    reject(); // Closes the dialog with QDialog::Rejected
}

void OptionsDialog::onDefaultClicked() {
    // Logic to reset settings to default
    // For example: noPartySoundCheckBox->setChecked(false);
// IMPORTANT: Replace this with the actual path to the folder you want to backup
    // For this example, we'll use a relative path that points to a folder 
    // named "my_source_folder" in the application's current working directory.
    QString folderToBackup = "."; 

    // Note: For a real-world application, you should use absolute paths 
    // (e.g., QDir::homePath() + "/Documents/Data") or ensure the relative path is correct.

    // --- You would call the function here in a real application ---
    // backupFilesToSubfolder(folderToBackup);

    // --- Self-Contained Example Setup (for testing/demonstration) ---
    // This section creates a dummy folder and some files to show the function working.
    QDir testDir(folderToBackup);
    if (!testDir.exists()) {
        testDir.mkpath("."); // Create the test folder
    }
//    QFile file1(testDir.absoluteFilePath("document1.txt"));
//    file1.open(QIODevice::WriteOnly); file1.write("Content 1"); file1.close();
//    QFile file2(testDir.absoluteFilePath("image.png"));
//    file2.open(QIODevice::WriteOnly); file2.write("Content 2"); file2.close();
//    QDir subDir(testDir.absoluteFilePath("a_sub_dir"));
//    subDir.mkpath("."); // Create an extra sub directory to show it's ignored by QDir::Files

    qDebug() << "--- Starting Backup Operation ---";
    bool success = backupFilesToDateSubfolder(folderToBackup);
    qDebug() << "Operation result:" << (success ? "SUCCESS" : "FAILURE");
    qDebug() << "--- Verification ---";
    QDir finalBackupDir(testDir.absoluteFilePath("backup"));
    qDebug() << "Files in backup folder:" << finalBackupDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
}
