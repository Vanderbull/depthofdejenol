#include "optionsdialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFontDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QUrl>
#include <QVBoxLayout>

#include "src/audio/audioManager.h"
#include "../core/GameSettings.h"

namespace {

// Where the game keeps its own data, and where backups of it go.
const char* kSavesDir      = "data/saves";
const char* kCharactersDir = "data/characters";
const char* kBackupRoot    = "data/backup";

QString todayStamp() {
    return QDate::currentDate().toString("yyyy-MM-dd");
}

// Copy every file directly inside `from` into `to` (created if needed).
// A missing source is not a failure — an empty data dir is fine to back up.
bool copyDirFlat(const QString& from, const QString& to) {
    QDir src(from);
    if (!src.exists()) return true;
    if (!QDir().mkpath(to)) return false;
    bool ok = true;
    const QStringList files = src.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString& f : files) {
        const QString dst = QDir(to).absoluteFilePath(f);
        if (QFile::exists(dst) && !QFile::remove(dst)) { ok = false; continue; }
        if (!QFile::copy(src.absoluteFilePath(f), dst)) ok = false;
    }
    return ok;
}

// Newest backup folder name under data/backup, or "" when there is none.
// The yyyy-MM-dd names sort lexicographically, so last() is the newest.
QString newestBackup() {
    QDir root(kBackupRoot);
    if (!root.exists()) return QString();
    const QStringList dirs =
        root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    return dirs.isEmpty() ? QString() : dirs.last();
}

// Combat animation speed -> the combat FX timer interval in DungeonDialog.
const int kSpeeds[] = { 100, 200, 400 };  // Fast / Normal / Slow
const int kSpeedCount = 3;

} // namespace

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Options"));
    setupUi();
    loadFromSettings();
    snapshot();
    refreshLastBackupLabel();
}

OptionsDialog::~OptionsDialog() = default;

// ------------------------------------------------------------------- UI ------

void OptionsDialog::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    auto *grid = new QGridLayout();
    grid->setSpacing(12);

    // ---- Audio ----
    auto *audioBox = new QGroupBox(tr("Audio"), this);
    auto *audioLayout = new QGridLayout(audioBox);

    m_musicSlider = new QSlider(Qt::Horizontal, audioBox);
    m_musicSlider->setObjectName("musicVolumeSlider");
    m_musicSlider->setRange(0, 100);
    m_musicValue = new QLabel(audioBox);
    m_musicValue->setMinimumWidth(40);
    audioLayout->addWidget(new QLabel(tr("Music volume"), audioBox), 0, 0);
    audioLayout->addWidget(m_musicSlider, 0, 1);
    audioLayout->addWidget(m_musicValue, 0, 2);

    m_sfxSlider = new QSlider(Qt::Horizontal, audioBox);
    m_sfxSlider->setObjectName("sfxVolumeSlider");
    m_sfxSlider->setRange(0, 100);
    m_sfxValue = new QLabel(audioBox);
    m_sfxValue->setMinimumWidth(40);
    audioLayout->addWidget(new QLabel(tr("Sound FX volume"), audioBox), 1, 0);
    audioLayout->addWidget(m_sfxSlider, 1, 1);
    audioLayout->addWidget(m_sfxValue, 1, 2);

    m_muteBox = new QCheckBox(tr("Mute all audio"), audioBox);
    m_muteBox->setObjectName("muteAllCheckBox");
    audioLayout->addWidget(m_muteBox, 2, 0, 1, 3);
    grid->addWidget(audioBox, 0, 0);

    // ---- Gameplay ----
    auto *gameBox = new QGroupBox(tr("Gameplay"), this);
    auto *gameLayout = new QGridLayout(gameBox);

    m_autosaveBox = new QCheckBox(tr("Autosave"), gameBox);
    m_autosaveBox->setObjectName("autosaveCheckBox");
    gameLayout->addWidget(m_autosaveBox, 0, 0, 1, 2);

    m_revealBox = new QCheckBox(tr("Reveal entire automap"), gameBox);
    m_revealBox->setObjectName("revealAutomapCheckBox");
    gameLayout->addWidget(m_revealBox, 1, 0, 1, 2);

    m_speedBox = new QComboBox(gameBox);
    m_speedBox->setObjectName("combatSpeedComboBox");
    m_speedBox->addItem(tr("Fast"));
    m_speedBox->addItem(tr("Normal"));
    m_speedBox->addItem(tr("Slow"));
    gameLayout->addWidget(new QLabel(tr("Combat animation speed"), gameBox), 2, 0);
    gameLayout->addWidget(m_speedBox, 2, 1);
    grid->addWidget(gameBox, 0, 1);

    // ---- Display ----
    auto *displayBox = new QGroupBox(tr("Display"), this);
    auto *displayLayout = new QGridLayout(displayBox);

    m_fullscreenBox = new QCheckBox(tr("Fullscreen"), displayBox);
    m_fullscreenBox->setObjectName("fullscreenCheckBox");
    displayLayout->addWidget(m_fullscreenBox, 0, 0, 1, 2);

    m_propFontButton = new QPushButton(displayBox);
    m_propFontButton->setObjectName("proportionalFontButton");
    m_fixedFontButton = new QPushButton(displayBox);
    m_fixedFontButton->setObjectName("fixedFontButton");
    displayLayout->addWidget(new QLabel(tr("Proportional font"), displayBox), 1, 0);
    displayLayout->addWidget(m_propFontButton, 1, 1);
    displayLayout->addWidget(new QLabel(tr("Non-proportional font"), displayBox), 2, 0);
    displayLayout->addWidget(m_fixedFontButton, 2, 1);
    grid->addWidget(displayBox, 1, 0);

    // ---- Data ----
    auto *dataBox = new QGroupBox(tr("Data"), this);
    auto *dataLayout = new QVBoxLayout(dataBox);

    m_lastBackupLabel = new QLabel(dataBox);
    m_lastBackupLabel->setObjectName("lastBackupLabel");
    dataLayout->addWidget(m_lastBackupLabel);

    m_backupButton = new QPushButton(tr("Backup now"), dataBox);
    m_backupButton->setObjectName("backupButton");
    m_restoreButton = new QPushButton(tr("Restore"), dataBox);
    m_restoreButton->setObjectName("restoreButton");
    auto *openFolderButton = new QPushButton(tr("Open save folder"), dataBox);
    openFolderButton->setObjectName("openSaveFolderButton");

    dataLayout->addWidget(m_backupButton);
    dataLayout->addWidget(m_restoreButton);
    dataLayout->addWidget(openFolderButton);
    dataLayout->addStretch();
    grid->addWidget(dataBox, 1, 1);

    mainLayout->addLayout(grid);
    mainLayout->addStretch();

    // ---- Actions ----
    auto *buttonLayout = new QHBoxLayout();
    m_defaultButton = new QPushButton(tr("Default"), this);
    m_defaultButton->setObjectName("defaultButton");
    m_okButton = new QPushButton(tr("OK"), this);
    m_okButton->setObjectName("okButton");
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    m_cancelButton->setObjectName("cancelButton");
    buttonLayout->addWidget(m_defaultButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    // ---- Wiring: every control drives GameSettings; audio previews live ----
    connect(m_musicSlider, &QSlider::valueChanged, this, [this](int v) {
        m_musicValue->setText(QString::number(v));
        GameSettings::instance().setMusicVolume(v / 100.0f);
        applyLiveAudio();
    });
    connect(m_sfxSlider, &QSlider::valueChanged, this, [this](int v) {
        m_sfxValue->setText(QString::number(v));
        GameSettings::instance().setSfxVolume(v / 100.0f);
        applyLiveAudio();
    });
    connect(m_muteBox, &QCheckBox::toggled, this, [this](bool on) {
        GameSettings::instance().setAudioMuted(on);
        applyLiveAudio();
    });
    connect(m_autosaveBox, &QCheckBox::toggled, this, [](bool on) {
        GameSettings::instance().setAutosaveEnabled(on);
    });
    connect(m_revealBox, &QCheckBox::toggled, this, [](bool on) {
        GameSettings::instance().setRevealAutomap(on);
    });
    connect(m_speedBox, &QComboBox::currentIndexChanged, this, [](int idx) {
        if (idx >= 0 && idx < kSpeedCount)
            GameSettings::instance().setCombatSpeedMs(kSpeeds[idx]);
    });
    connect(m_fullscreenBox, &QCheckBox::toggled, this, [](bool on) {
        GameSettings::instance().setFullscreen(on);
    });
    connect(m_propFontButton, &QPushButton::clicked, this, &OptionsDialog::onSelectProportionalFont);
    connect(m_fixedFontButton, &QPushButton::clicked, this, &OptionsDialog::onSelectFixedFont);
    connect(m_backupButton, &QPushButton::clicked, this, &OptionsDialog::onBackupClicked);
    connect(m_restoreButton, &QPushButton::clicked, this, &OptionsDialog::onRestoreClicked);
    connect(openFolderButton, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir(kSavesDir).absolutePath()));
    });
    connect(m_okButton, &QPushButton::clicked, this, &OptionsDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &OptionsDialog::onCancelClicked);
    connect(m_defaultButton, &QPushButton::clicked, this, &OptionsDialog::onDefaultClicked);
}

void OptionsDialog::loadFromSettings() {
    // Block signals so loading the stored values does not look like the player
    // editing them (which would write them straight back and restart audio).
    QSignalBlocker blockMusic(m_musicSlider);
    QSignalBlocker blockSfx(m_sfxSlider);
    QSignalBlocker blockMute(m_muteBox);
    QSignalBlocker blockAutosave(m_autosaveBox);
    QSignalBlocker blockReveal(m_revealBox);
    QSignalBlocker blockSpeed(m_speedBox);
    QSignalBlocker blockFull(m_fullscreenBox);

    GameSettings& s = GameSettings::instance();

    m_musicSlider->setValue(qRound(s.musicVolume() * 100.0f));
    m_sfxSlider->setValue(qRound(s.sfxVolume() * 100.0f));
    m_musicValue->setText(QString::number(m_musicSlider->value()));
    m_sfxValue->setText(QString::number(m_sfxSlider->value()));
    m_muteBox->setChecked(s.audioMuted());

    m_autosaveBox->setChecked(s.autosaveEnabled());
    m_revealBox->setChecked(s.revealAutomap());

    int speedIdx = 1;  // Normal
    for (int i = 0; i < kSpeedCount; ++i)
        if (kSpeeds[i] == s.combatSpeedMs()) speedIdx = i;
    m_speedBox->setCurrentIndex(speedIdx);

    m_fullscreenBox->setChecked(s.fullscreen());

    m_propFamily  = s.proportionalFontFamily();
    m_propSize    = s.proportionalFontSize();
    m_fixedFamily = s.fixedFontFamily();
    m_fixedSize   = s.fixedFontSize();
    refreshFontButtons();
}

void OptionsDialog::refreshFontButtons() {
    const QFont appFont = QApplication::font();
    m_propFontButton->setText(m_propFamily.isEmpty()
        ? tr("%1 (default)").arg(appFont.family())
        : tr("%1 (%2)").arg(m_propFamily).arg(m_propSize));
    m_fixedFontButton->setText(m_fixedFamily.isEmpty()
        ? tr("Courier New (default)")
        : tr("%1 (%2)").arg(m_fixedFamily).arg(m_fixedSize));
}

void OptionsDialog::applyLiveAudio() {
    GameSettings& s = GameSettings::instance();
    auto *audio = audioManager::instance();
    const bool muted = m_muteBox->isChecked();
    audio->setMusicVolume(muted ? 0.0f : s.musicVolume());
    audio->setSfxVolume(muted ? 0.0f : s.sfxVolume());
}

// ------------------------------------------------------- snapshot / restore ---

void OptionsDialog::snapshot() {
    GameSettings& s = GameSettings::instance();
    m_snapshot.music       = s.musicVolume();
    m_snapshot.sfx         = s.sfxVolume();
    m_snapshot.muted       = s.audioMuted();
    m_snapshot.autosave    = s.autosaveEnabled();
    m_snapshot.reveal      = s.revealAutomap();
    m_snapshot.fullscreen  = s.fullscreen();
    m_snapshot.speed       = s.combatSpeedMs();
    m_snapshot.propFamily  = s.proportionalFontFamily();
    m_snapshot.propSize    = s.proportionalFontSize();
    m_snapshot.fixedFamily = s.fixedFontFamily();
    m_snapshot.fixedSize   = s.fixedFontSize();
}

void OptionsDialog::restoreSnapshot() {
    GameSettings& s = GameSettings::instance();
    s.setMusicVolume(m_snapshot.music);
    s.setSfxVolume(m_snapshot.sfx);
    s.setAudioMuted(m_snapshot.muted);
    s.setAutosaveEnabled(m_snapshot.autosave);
    s.setRevealAutomap(m_snapshot.reveal);
    s.setFullscreen(m_snapshot.fullscreen);
    s.setCombatSpeedMs(m_snapshot.speed);
    s.setProportionalFont(m_snapshot.propFamily, m_snapshot.propSize);
    s.setFixedFont(m_snapshot.fixedFamily, m_snapshot.fixedSize);
    s.applyAll();
}

// ----------------------------------------------------------------- actions ---

void OptionsDialog::onOkClicked() {
    GameSettings::instance().sync();
    accept();
}

void OptionsDialog::onCancelClicked() {
    // Undo every change made while the dialog was open, then close.
    restoreSnapshot();
    GameSettings::instance().sync();
    reject();
}

void OptionsDialog::onDefaultClicked() {
    GameSettings::instance().resetToDefaults();
    loadFromSettings();
}

void OptionsDialog::onSelectProportionalFont() {
    const QFont initial = m_propFamily.isEmpty() ? QApplication::font()
                                                 : QFont(m_propFamily, m_propSize);
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, initial, this,
                                            tr("Select Proportional Font"));
    if (!ok) return;
    m_propFamily = font.family();
    m_propSize = font.pointSize();
    GameSettings::instance().setProportionalFont(m_propFamily, m_propSize);
    GameSettings::instance().applyAll();
    refreshFontButtons();
}

void OptionsDialog::onSelectFixedFont() {
    const QFont initial = m_fixedFamily.isEmpty() ? QFont("Courier New", 9)
                                                  : QFont(m_fixedFamily, m_fixedSize);
    QFontDialog dialog(initial, this);
    dialog.setWindowTitle(tr("Select Non-Proportional Font"));
    if (dialog.exec() != QDialog::Accepted) return;
    const QFont font = dialog.selectedFont();
    m_fixedFamily = font.family();
    m_fixedSize = font.pointSize();
    GameSettings::instance().setFixedFont(m_fixedFamily, m_fixedSize);
    GameSettings::instance().applyAll();
    refreshFontButtons();
}

// -------------------------------------------------------------------- data ---

void OptionsDialog::onBackupClicked() {
    const QString dest = QString(kBackupRoot) + "/" + todayStamp();
    bool ok = copyDirFlat(kSavesDir, dest + "/saves");
    ok = copyDirFlat(kCharactersDir, dest + "/characters") && ok;
    if (ok) {
        m_lastBackupLabel->setText(tr("Last backup: %1").arg(todayStamp()));
    } else {
        m_lastBackupLabel->setText(tr("Backup failed"));
        qWarning() << "OptionsDialog: backup to" << dest << "failed";
    }
}

void OptionsDialog::onRestoreClicked() {
    const QString newest = newestBackup();
    if (newest.isEmpty()) {
        m_lastBackupLabel->setText(tr("No backup found"));
        return;
    }
    bool ok = copyDirFlat(QString(kBackupRoot) + "/" + newest + "/saves", kSavesDir);
    ok = copyDirFlat(QString(kBackupRoot) + "/" + newest + "/characters", kCharactersDir) && ok;
    m_lastBackupLabel->setText(ok ? tr("Restored backup from %1").arg(newest)
                                  : tr("Restore failed"));
}

void OptionsDialog::refreshLastBackupLabel() {
    const QString newest = newestBackup();
    m_lastBackupLabel->setText(newest.isEmpty()
        ? tr("No backup yet")
        : tr("Last backup: %1").arg(newest));
}
