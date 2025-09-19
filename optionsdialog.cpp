#include "optionsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

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
}
