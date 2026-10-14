# Roadmap v6.6.0 — Options dialog, rebuilt for the game as it is

**Goal:** replace the inherited MORDOR option screen with an Options dialog whose
every control drives a system that actually exists in this game, persists, and
survives a restart.

Status: PLANNED

---

## 1. Inventory — what the game actually has

Everything below was verified in the source, not assumed.

| System | Where | Exposed today? |
|---|---|---|
| Music volume / play / stop | `audioManager::setMusicVolume`, `playMusic`, `stopAllAudio` | yes (slider, not persisted) |
| SFX volume | `audioManager::setSfxVolume` | yes (slider, not persisted) |
| Autosave | `AssemblePartyDialog.cpp:247`, `blacklands.cpp:472` | no flag honoured |
| Automap reveal-all | `MiniMapDialog::isRevealAllEnabled()` (labelled "Debug") | in the map dialog only |
| Combat FX timing | `DungeonDialog` `m_combatFxTimer` (200 ms) | no |
| Fullscreen | `GameMenu::showFullScreen()` (hard-coded) | no |
| Proportional / fixed font | `fontManager` + `gameStateManager::getProportionalFont()` | stored, never applied |
| Saves | `data/saves/*.json` (autosave + named) | via Load/Save menu |
| Backup / restore | `backupFilesToDateSubfolder` (copies **CWD top-level files**) | yes, wrong scope |

### Systems that do NOT exist (so their checkboxes cannot be "made to work")
No party-sound routing · no message "ding" · no Help dialog (removed v6.5.2) ·
no toolbar (plan dropped) · no automap cursor blink · no 3-D view ·
no StretchBlt · no automap state save/load · no player-selectable difficulty
(`MonsterBalance::difficultyLabel` is a per-floor display string, not a setting).

The current 9 Game Settings checkboxes are exactly these — 8 are vestigial and
1 (No Autosave) points at a real system. They are **removed**, not faked.

---

## 2. The new dialog

Four groups, every control wired to a real system, one store, real Cancel.

### Audio → `audioManager`
- **Music volume** slider (0–100) → `setMusicVolume`, persisted
- **SFX volume** slider (0–100) → `setSfxVolume`, persisted
- **Mute all audio** checkbox → `stopAllAudio()` / restore, persisted

### Gameplay
- **Autosave** checkbox → gates the two autosave sites above
- **Reveal entire automap** checkbox → drives `MiniMapDialog::isRevealAllEnabled()`
  (promoted from the map's "Debug" toggle to an honest accessibility option)
- **Combat animation speed** (Fast / Normal / Slow) → the `m_combatFxTimer`
  interval (100 / 200 / 400 ms)

### Display
- **Fullscreen** checkbox → the main menu and story dialog stop hard-coding
  `showFullScreen()` and read this instead
- **Proportional font** chooser → applied via `QApplication::setFont()`
- **Fixed (monospace) font** chooser → applied to the text-heavy views

### Data
- **Backup now** → snapshots `data/saves/` + `data/characters/` into
  `data/backup/<date>/` (not the top-level files of the working directory)
- **Restore** → restores the newest backup
- **Open save folder** → `QDesktopServices::openUrl(data/saves)`
- **Last backup: <date>** label — read on open, not only after a click

### Buttons
- **OK** — commit + persist
- **Cancel** — restore the snapshot taken when the dialog opened (including
  volume/font changes, which preview live)
- **Default** — reset every control to its default and re-apply

---

## 3. Settings store + startup apply

New `src/core/GameSettings.{h,cpp}` — one file, one schema, typed accessors:

```cpp
class GameSettings {
public:
    static GameSettings& instance();
    float musicVolume() const;      void setMusicVolume(float);
    float sfxVolume() const;        void setSfxVolume(float);
    bool  audioMuted() const;       void setAudioMuted(bool);
    bool  autosaveEnabled() const;  void setAutosaveEnabled(bool);
    bool  revealAutomap() const;    void setRevealAutomap(bool);
    int   combatSpeedMs() const;    void setCombatSpeedMs(int);
    bool  fullscreen() const;       void setFullscreen(bool);
    QString proportionalFontFamily() const; int proportionalFontSize() const;
    QString fixedFontFamily() const;        int fixedFontSize() const;
    void applyAll();   // push every value into the running game (startup)
    void sync();       // flush to disk
    void resetToDefaults();
};
```

- Single `QSettings("game_settings.ini")`, single `[Game]` section.
- **Migrate** the legacy `[Graphics]/[Audio]/[Gameplay]` keys on first run, then
  remove them.
- `applyAll()` is called from `main()` after `gameStateManager` init — this is
  what makes a setting stick across restarts.

---

## 4. Phases

### Phase 1 — Settings store + startup apply
`GameSettings`, defaults, legacy migration, `sync()`, `applyAll()` wired at
startup. **Tests:** round-trip (set→sync→reload); defaults when the file is
absent; legacy keys migrated then gone.

### Phase 2 — Audio + Gameplay
Wire volumes/mute/autosave/reveal/combat-speed. **Tests:** mute → volume 0 and
unmute restores; autosave off → no `autosave.json` written by the party dialog;
combat speed → the FX timer interval matches the setting.

### Phase 3 — Display
Fullscreen read by the menu/story dialog; fonts applied on selection and at
startup. **Tests:** fullscreen flag → `isFullScreen()` matches after apply; font
setter → `QApplication::font().family()` changes; `Default` restores.

### Phase 4 — Data + dialog semantics
Backup/Restore re-scoped; last-backup read on open; OK/Cancel/Default with a real
snapshot. **Tests:** change + Cancel → value unchanged; Backup writes into
`data/backup/<date>/`; last-backup label populated on open.

### Phase 5 — Remove the vestigial controls
Delete the 8 dead checkboxes and their persistence; remove or rewire the dead
`ConfigIntegrityOK` path (`checkSettingsFile()` reads `Graphics/ResolutionWidth`
which nothing writes and nothing consumes). **Tests:** the dialog no longer
exposes those object names; the settings file has no `[Game] No*` keys.

---

## 5. Explicitly out of scope (would be features, not settings)
- **Difficulty levels** — needs a real scaling system in `MonsterBalance` first.
  Tracked separately, not faked with a dropdown.
- **Message/combat text speed** — no message-speed system exists.
- **Party-sound routing** — no per-category SFX routing exists.

## 6. Definition of done
- Every remaining control changes real behaviour, persists, and re-applies at
  startup; nothing is write-only.
- Cancel restores exactly the pre-dialog state; Default resets everything.
- No setting writes to a key nothing reads.
- `make check` green, each wired setting covered by a non-vacuous test that fails
  when the wiring is reverted.
