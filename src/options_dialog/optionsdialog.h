#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>

#include "GameStateManager.h" // Include the GameStateManager header

class QCheckBox;
class QSlider;
class QPushButton;
// No need to forward declare QLabel if it's included above, but it doesn't hurt.

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    ~OptionsDialog();

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onDefaultClicked();

private:
    void setupUi();

    // Game Settings
    QCheckBox *noPartySoundCheckBox;
    QCheckBox *noMsgDingCheckBox;
    QCheckBox *noHelpSoundCheckBox;
    QCheckBox *noToolbarCheckBox;
    QCheckBox *noAutomapCursorBlinkCheckBox;
    QCheckBox *no3DCheckBox;
    QCheckBox *noStretchBltCheckBox;
    QCheckBox *noLoadAutomapCheckBox;
    QCheckBox *noAutosaveCheckBox;

    // Music Settings
    QSlider *musicVolSlider;
    QCheckBox *noMusicCheckBox;
    QSlider *sfxVolSlider;
    QCheckBox *noSoundFxCheckBox;
    QCheckBox *useSoundCanvasMIDICheckBox;
    
    // Font Settings
    QCheckBox *nonProportionalFontCheckBox;
    // ... (Add other font widgets if needed)

    // File Backups
    QLabel *lastBackupLabel; // Now this will be recognized
    QPushButton *backupButton;
    QPushButton *restoreButton;
    
    // Action Buttons
    QPushButton *okButton;
    QPushButton *cancelButton;
    QPushButton *defaultButton;
};

#endif // OPTIONSDIALOG_H
