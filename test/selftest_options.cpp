#include "selftest_harness.h"
#include "src/theCity.h"

#include <QCheckBox>
#include <QFile>
#include <QFont>
#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QDate>
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QSettings>
#include <QSlider>
#include <QTemporaryDir>

#include "src/audio/audioManager.h"
#include "src/blacklands.h"
#include "src/core/gameStateManager.h"
#include "src/fontManager.h"
#include "src/core/GameSettings.h"
#include "src/dungeon_dialog/MiniMapDialog.h"
#include "src/options_dialog/optionsdialog.h"

// ---------------------------------------------------------------
// Non-vacuous tests for the Options dialog rework (v6.6.0).
// Every check FAILS when the wiring it covers is reverted: the
// store, the startup apply, the autosave gate, the dialog's live
// controls, Cancel/Default, the backup scope, and the minimap link.
// ---------------------------------------------------------------

namespace {

void testSettingsReset() {
    section("[O1] GameSettings reset restores every default");

    GameSettings& s = GameSettings::instance();
    s.setMusicVolume(0.11f);
    s.setSfxVolume(0.22f);
    s.setAudioMuted(true);
    s.setAutosaveEnabled(false);
    s.setRevealAutomap(true);
    s.setCombatSpeedMs(400);
    s.setFullscreen(false);

    s.resetToDefaults();

    check(s.musicVolume() == GameSettings::defaultMusicVolume(),
          "reset restores the music volume", QString::number(s.musicVolume()));
    check(s.sfxVolume() == GameSettings::defaultSfxVolume(),
          "reset restores the sfx volume", QString::number(s.sfxVolume()));
    check(!s.audioMuted(), "reset clears mute");
    check(s.autosaveEnabled(), "reset re-enables autosave");
    check(!s.revealAutomap(), "reset clears reveal-automap");
    check(s.combatSpeedMs() == GameSettings::defaultCombatSpeedMs(),
          "reset restores the combat speed", QString::number(s.combatSpeedMs()));
    check(s.fullscreen(), "reset restores fullscreen");
}

void testSettingsPersist() {
    section("[O2] Settings persist to game_settings.ini");

    GameSettings& s = GameSettings::instance();
    s.setMusicVolume(0.33f);
    s.setCombatSpeedMs(100);
    s.setRevealAutomap(true);
    s.sync();

    // Read the file back through an independent handle: proves the value hit
    // the disk under [Game], not just the in-memory accessor.
    QSettings raw("game_settings.ini", QSettings::IniFormat);
    check(qAbs(raw.value("Game/musicVolume").toFloat() - 0.33f) < 0.001f,
          "music volume written to [Game]", raw.value("Game/musicVolume").toString());
    check(raw.value("Game/combatSpeedMs").toInt() == 100,
          "combat speed written to [Game]");
    check(raw.value("Game/revealAutomap").toBool() == true,
          "reveal-automap written to [Game]");

    s.setRevealAutomap(false);
    s.sync();
}

void testLegacyMigration() {
    section("[O3] Legacy sections migrate into [Game] and are dropped");

    QTemporaryDir tmp;
    check(tmp.isValid(), "temp dir for the migration probe");
    const QString path = tmp.path() + "/legacy.ini";

    {
        QSettings legacy(path, QSettings::IniFormat);
        legacy.setValue("Graphics/Fullscreen", false);
        legacy.setValue("Audio/MusicVolume", 40);
        legacy.setValue("Audio/SFXVolume", 20);
        legacy.setValue("Gameplay/AutoSave", false);
        legacy.sync();
    }

    QSettings legacy(path, QSettings::IniFormat);
    GameSettings::migrateLegacyKeys(&legacy);

    check(qAbs(legacy.value("Game/musicVolume").toFloat() - 0.40f) < 0.001f,
          "legacy music volume migrated", legacy.value("Game/musicVolume").toString());
    check(qAbs(legacy.value("Game/sfxVolume").toFloat() - 0.20f) < 0.001f,
          "legacy sfx volume migrated");
    check(legacy.value("Game/autosaveEnabled").toBool() == false,
          "legacy AutoSave migrated");
    check(legacy.value("Game/fullscreen").toBool() == false,
          "legacy Fullscreen migrated");
    check(!legacy.childGroups().contains("Graphics") &&
          !legacy.childGroups().contains("Audio") &&
          !legacy.childGroups().contains("Gameplay"),
          "legacy sections dropped", legacy.childGroups().join(","));
}

void testAudioApply() {
    section("[O4] Audio settings reach the audio manager");

    GameSettings& s = GameSettings::instance();
    auto* audio = audioManager::instance();

    s.setAudioMuted(false);
    s.setMusicVolume(0.5f);
    s.setSfxVolume(0.25f);
    s.applyAll();
    check(qAbs(audio->getMusicVolume() - 0.5f) < 0.01f,
          "applyAll sets the music volume", QString::number(audio->getMusicVolume()));
    check(qAbs(audio->getSfxVolume() - 0.25f) < 0.01f,
          "applyAll sets the sfx volume", QString::number(audio->getSfxVolume()));

    s.setAudioMuted(true);
    s.applyAll();
    check(audio->getMusicVolume() == 0.0f, "mute zeroes the music volume");
    check(audio->getSfxVolume() == 0.0f, "mute zeroes the sfx volume");

    s.setAudioMuted(false);
    s.applyAll();
    check(qAbs(audio->getMusicVolume() - 0.5f) < 0.01f,
          "unmute restores the stored music volume");
}

void testAutosaveGate() {
    section("[O5] Autosave switch stops the periodic timer");

    GameSettings& s = GameSettings::instance();
    auto* gsm = gameStateManager::instance();

    s.setAutosaveEnabled(false);
    gsm->startAutosave(50);
    check(!gsm->isAutosaveRunning(), "autosave off -> timer not armed");

    s.setAutosaveEnabled(true);
    gsm->startAutosave(60000);
    check(gsm->isAutosaveRunning(), "autosave on -> timer armed");
    gsm->stopAutosave();
}

void testDialogControls() {
    section("[O6] Options dialog exposes only live controls");

    OptionsDialog dlg;
    check(dlg.findChild<QCheckBox*>("autosaveCheckBox") != nullptr, "Autosave control exists");
    check(dlg.findChild<QCheckBox*>("muteAllCheckBox") != nullptr, "Mute control exists");
    check(dlg.findChild<QCheckBox*>("revealAutomapCheckBox") != nullptr, "Reveal-automap control exists");
    check(dlg.findChild<QCheckBox*>("fullscreenCheckBox") != nullptr, "Fullscreen control exists");
    check(dlg.findChild<QComboBox*>("combatSpeedComboBox") != nullptr, "Combat speed control exists");
    check(dlg.findChild<QSlider*>("musicVolumeSlider") != nullptr, "Music volume slider exists");
    check(dlg.findChild<QSlider*>("sfxVolumeSlider") != nullptr, "SFX volume slider exists");

    // The inherited MORDOR controls are gone — none of them had a feature.
    const QList<QCheckBox*> boxes = dlg.findChildren<QCheckBox*>();
    check(boxes.size() == 4, "exactly four checkboxes remain", QString::number(boxes.size()));
    for (QCheckBox* b : boxes) {
        const QString t = b->text();
        check(!t.contains("3-D") && !t.contains("StretchBlt") && !t.contains("Toolbar") &&
              !t.contains("HELP") && !t.contains("Party Sound") && !t.contains("Blink") &&
              !t.contains("Load Automap"),
              QString("no dead control left: %1").arg(t));
    }
}

void testCombatSpeedCombo() {
    section("[O7] Combat speed combo maps to the FX timer interval");

    OptionsDialog dlg;
    auto* combo = dlg.findChild<QComboBox*>("combatSpeedComboBox");
    check(combo != nullptr, "combo exists");
    if (!combo) return;
    check(combo->count() == 3, "three speed choices", QString::number(combo->count()));

    combo->setCurrentIndex(0);
    check(GameSettings::instance().combatSpeedMs() == 100, "Fast -> 100 ms");
    combo->setCurrentIndex(2);
    check(GameSettings::instance().combatSpeedMs() == 400, "Slow -> 400 ms");
    combo->setCurrentIndex(1);
    check(GameSettings::instance().combatSpeedMs() == 200, "Normal -> 200 ms");
}

void testCancelRestores() {
    section("[O8] Cancel restores the pre-dialog state");

    GameSettings& s = GameSettings::instance();
    s.resetToDefaults();
    s.setMusicVolume(0.5f);
    s.setCombatSpeedMs(200);
    s.sync();

    {
        OptionsDialog dlg;
        auto* slider = dlg.findChild<QSlider*>("musicVolumeSlider");
        auto* combo = dlg.findChild<QComboBox*>("combatSpeedComboBox");
        check(slider && combo, "controls found");
        if (slider && combo) {
            slider->setValue(95);
            combo->setCurrentIndex(0);
            check(qAbs(s.musicVolume() - 0.95f) < 0.01f, "slider change applied live");
            check(s.combatSpeedMs() == 100, "combo change applied live");
        }
        dlg.findChild<QPushButton*>("cancelButton")->click();
    }

    check(qAbs(s.musicVolume() - 0.5f) < 0.01f,
          "Cancel restored the music volume", QString::number(s.musicVolume()));
    check(s.combatSpeedMs() == 200, "Cancel restored the combat speed");
}

void testDefaultButton() {
    section("[O9] Default button resets every control");

    GameSettings& s = GameSettings::instance();
    s.setMusicVolume(0.05f);
    s.setCombatSpeedMs(400);
    s.setFullscreen(false);

    {
        OptionsDialog dlg;
        dlg.findChild<QPushButton*>("defaultButton")->click();
    }

    check(s.musicVolume() == GameSettings::defaultMusicVolume(), "Default restores the music volume");
    check(s.combatSpeedMs() == GameSettings::defaultCombatSpeedMs(), "Default restores the combat speed");
    check(s.fullscreen() == GameSettings::defaultFullscreen(), "Default restores fullscreen");
}

void testRevealAutomapWiring() {
    section("[O10] Reveal-automap setting drives the minimap");

    GameSettings::instance().setRevealAutomap(true);
    {
        MinimapDialog mm;
        check(mm.isRevealAllEnabled(), "minimap honours the reveal setting on open");
        QCheckBox* box = mm.findChild<QCheckBox*>();
        check(box != nullptr, "minimap has the reveal checkbox");
        if (box) box->setChecked(false);
    }
    check(!GameSettings::instance().revealAutomap(),
          "toggling the minimap persists the setting");
    GameSettings::instance().setRevealAutomap(false);
}

void testBackupScope() {
    section("[O11] Backup writes into data/backup/<date>");

    const QString stamp = QDate::currentDate().toString("yyyy-MM-dd");
    const QString dir = "data/backup/" + stamp;
    QDir(dir).removeRecursively();  // start clean so the check is meaningful

    {
        OptionsDialog dlg;
        dlg.findChild<QPushButton*>("backupButton")->click();
        QLabel* label = dlg.findChild<QLabel*>("lastBackupLabel");
        check(label && label->text().contains(stamp), "label shows the backup date",
              label ? label->text() : QString());
    }

    check(QDir(dir).exists(), "backup folder created under data/backup");
    check(QDir(dir + "/saves").exists(), "saves backed up");
    check(QDir(dir + "/characters").exists(), "characters backed up");

    // A fresh dialog reads the newest backup on open, not only after a click.
    {
        OptionsDialog dlg;
        QLabel* label = dlg.findChild<QLabel*>("lastBackupLabel");
        check(label && label->text().contains(stamp),
              "last-backup label is populated on open",
              label ? label->text() : QString());
    }

    QDir(dir).removeRecursively();
}

void testDefaultRestoresFont() {
    section("[O13] Default button restores the game's font");

    GameSettings& s = GameSettings::instance();

    // A picked font has to take effect first, or the reset below proves nothing.
    s.setProportionalFont("Courier New", 9);
    s.applyAll();
    check(QApplication::font().family() == QStringLiteral("Courier New"),
          "picked font is applied", QApplication::font().family());

    // "Default" is resetToDefaults() + reload in the dialog. It has to put the
    // game's own font back - not Qt's platform default, which is what the
    // original bug loaded.
    s.resetToDefaults();
    check(QApplication::font().family() == QStringLiteral("MS Sans Serif") &&
          QApplication::font().pointSize() == 8,
          "Default restores the game's font (MS Sans Serif 8)",
          QApplication::font().toString());
    check(fontManager::instance()->proportionalFont().family() == QStringLiteral("MS Sans Serif"),
          "Default restores the font manager's proportional font");
}

void testFontLeavesWidgetFontsAlone() {
    section("[O14] A picked font must not clobber a widget's own font");

    GameSettings& s = GameSettings::instance();
    s.resetToDefaults();

    // Two siblings: one with a font of its own (the menu's 28pt title), one
    // that merely inherits the application font.
    QWidget host;
    // The real dialog inherits MainMenu.qss from GameMenu. Apply the same
    // sheet here - without it a widget's font carries a resolve mask and behaves
    // differently from what the player sees.
    if (QFile qss(QStringLiteral("src/MainMenu.qss"));
            qss.open(QFile::ReadOnly | QFile::Text))
        host.setStyleSheet(QString::fromUtf8(qss.readAll()));

    QLabel title(QStringLiteral("Title"), &host);
    QFont own = title.font();
    own.setPointSize(28);
    own.setBold(true);
    title.setFont(own);
    const QString ownBefore = title.font().toString();

    QLabel plain(QStringLiteral("Plain"), &host);
    const QString defaultFont = GameSettings::defaultProportionalFont().toString();

    s.setProportionalFont("Courier New", 9);
    s.applyAll();

    check(title.font().toString() == ownBefore,
          "picked font leaves the widget's own font alone", title.font().toString());
    check(plain.font().family() == QStringLiteral("Courier New"),
          "picked font reaches an inheriting widget", plain.font().family());

    s.resetToDefaults();
    check(title.font().toString() == ownBefore,
          "Default leaves the widget's own font alone", title.font().toString());
    check(plain.font().toString() == defaultFont,
          "Default restores the game's font", plain.font().toString());
}

void testFullscreenSetting() {
    section("[O12] Fullscreen setting is honoured by the main menu");

    GameSettings& s = GameSettings::instance();
    s.setFullscreen(false);
    s.sync();
    {
        GameMenu menu;
        check(!menu.isFullScreen(), "windowed when the setting is off");
    }

    s.setFullscreen(true);
    s.sync();
    {
        GameMenu menu;
        check(menu.isFullScreen(), "fullscreen when the setting is on");
    }
}


void testMenuWidgetsFollowTheFont() {
    section("[O15] Menu widgets follow the font chosen in Options");

    GameSettings& s = GameSettings::instance();
    s.resetToDefaults();
    const QString defaultFamily = QApplication::font().family();

    GameMenu menu;
    QPushButton *newBtn = menu.findChild<QPushButton*>(QStringLiteral("newGameButton"));
    QLabel *title = menu.findChildren<QLabel*>().value(0);
    check(newBtn && title, "menu widgets found");
    if (!newBtn || !title) return;

    check(newBtn->font().family() == defaultFamily,
          "menu button uses the game's font before any change", newBtn->font().family());

    s.setProportionalFont("Courier New", 9);
    s.applyAll();
    QApplication::processEvents();

    check(newBtn->font().family() == QStringLiteral("Courier New"),
          "menu button follows a picked font", newBtn->font().family());

    s.resetToDefaults();
    QApplication::processEvents();
    check(newBtn->font().family() == defaultFamily,
          "menu button follows Default back", newBtn->font().family());
}

void testTheCityButtonsFollowFont() {
    section("[O16] theCity buttons follow the font chosen in Options");

    GameSettings& s = GameSettings::instance();
    s.resetToDefaults();

    theCity city;
    QToolButton *storeBtn = city.findChild<QToolButton*>(QStringLiteral("generalStoreButton"));
    QPushButton *sendBtn = city.findChild<QPushButton*>(QStringLiteral("sendButton"));
    check(storeBtn && sendBtn, "theCity buttons found");
    if (!storeBtn || !sendBtn) return;

    const QString appFontStr = QApplication::font().toString();
    check(storeBtn->font().toString() == appFontStr,
          "city button uses the font selected in Options",
          QString("store: %1 (app: %2)").arg(storeBtn->font().toString(), appFontStr));

    s.setProportionalFont("Courier New", 9);
    s.applyAll();
    QApplication::processEvents();

    check(storeBtn->font().family() == QStringLiteral("Courier New"),
          "city button follows a picked font", storeBtn->font().family());
    check(sendBtn->font().family() == QStringLiteral("Courier New"),
          "city send button follows a picked font", sendBtn->font().family());

    s.resetToDefaults();
    QApplication::processEvents();
    check(storeBtn->font().toString() == appFontStr,
          "city button follows Default back", storeBtn->font().toString());
}

} // namespace

void runOptionsTests() {
    testSettingsReset();
    testSettingsPersist();
    testLegacyMigration();
    testAudioApply();
    testAutosaveGate();
    testDialogControls();
    testCombatSpeedCombo();
    testCancelRestores();
    testDefaultButton();
    testRevealAutomapWiring();
    testBackupScope();
    testDefaultRestoresFont();
    testFontLeavesWidgetFontsAlone();
    testFullscreenSetting();
    testMenuWidgetsFollowTheFont();
    testTheCityButtonsFollowFont();
}


