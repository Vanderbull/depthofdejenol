#include "selftest_harness.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QTemporaryDir>

#include "audioManager.h"
#include "blacklands.h"
#include "gameStateManager.h"
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
    testFullscreenSetting();
}
