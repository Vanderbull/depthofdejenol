#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QString>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;

// Options dialog, rebuilt for the game as it is (v6.6.0).
//
// Every control drives a real system through GameSettings, previews live where
// that makes sense (audio), and persists so the choice survives a restart.
// The inherited MORDOR checkboxes (No Party Sound, No 3-D, No StretchBlt, …)
// are gone — none of them had a feature behind them.
class OptionsDialog : public QDialog {
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    ~OptionsDialog() override;

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onDefaultClicked();
    void onSelectProportionalFont();
    void onSelectFixedFont();
    void onBackupClicked();
    void onRestoreClicked();
    void onExportClicked();
    void onImportClicked();

private:
    void setupUi();
    void loadFromSettings();
    void refreshFontButtons();
    void refreshLastBackupLabel();
    void applyLiveAudio();

    // Snapshot taken when the dialog opens; Cancel restores it exactly.
    void snapshot();
    void restoreSnapshot();

    // --- Audio ---
    QSlider   *m_musicSlider = nullptr;
    QLabel    *m_musicValue  = nullptr;
    QSlider   *m_sfxSlider   = nullptr;
    QLabel    *m_sfxValue    = nullptr;
    QCheckBox *m_muteBox     = nullptr;

    // --- Gameplay ---
    QCheckBox *m_autosaveBox = nullptr;
    QCheckBox *m_revealBox   = nullptr;
    QComboBox *m_speedBox    = nullptr;

    // --- Display ---
    QCheckBox   *m_fullscreenBox   = nullptr;
    QPushButton *m_propFontButton  = nullptr;
    QPushButton *m_fixedFontButton = nullptr;

    // --- Data ---
    QLabel      *m_lastBackupLabel = nullptr;
    QPushButton *m_backupButton    = nullptr;
    QPushButton *m_restoreButton   = nullptr;
    QPushButton *m_exportButton    = nullptr;
    QPushButton *m_importButton    = nullptr;

    // --- Actions ---
    QPushButton *m_okButton      = nullptr;
    QPushButton *m_cancelButton  = nullptr;
    QPushButton *m_defaultButton = nullptr;

    // Font selection currently shown in the dialog.
    QString m_propFamily;
    int     m_propSize  = 0;
    QString m_fixedFamily;
    int     m_fixedSize = 0;

    struct Snapshot {
        float music = 0.0f, sfx = 0.0f;
        bool  muted = false, autosave = true, reveal = false, fullscreen = true;
        int   speed = 200;
        QString propFamily, fixedFamily;
        int   propSize = 0, fixedSize = 0;
    } m_snapshot;
};

#endif // OPTIONSDIALOG_H
