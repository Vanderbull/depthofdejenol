# Roadmap v5.2.0 — Savegame revamp

**Status:** IN PROGRESS — slices 5.1–5.2 done
**Version:** v5.2.0 (minor — party building from individual characters, autosave fix, inventory preservation)

## Goal

The savegame system has three critical bugs:
1. Autosave silently fails — `handleAutosave()` calls `saveCharacterToFile(0)` which reads stale `m_gameStateData["Party"]`
2. `CharacterListDialog::onSelectClicked()` calls `loadCharacterFromFile()` which replaces the party each time — only the last selected character survives
3. `saveCharacterToFile()` doesn't save inventory, equipped items, hunger, or guild data

This version fixes all three and adds proper party building from individual character files.

## Slices

### 5.1 Autosave fix `S` ✅ DONE

**Files:** `gameStateManager.cpp`, `gameStateManager.h`, `test/selftest.cpp`

**What changed:**
- `handleAutosave()` now calls `saveFullGameState("autosave")` instead of `saveCharacterToFile(0)`
- `handleAutosave()` is now public (was private)
- Removed the duplicate `saveFullGameState()` call at the top of `handleAutosave()`

**Verified:** test [232] — autosave saves party with inventory. Non-vacuous: reverting to `saveCharacterToFile(0)` fails the test.

### 5.2 Party building from individual characters `S` ✅ DONE

**Files:** `gameStateManager.cpp`, `gameStateManager.h`, `src/characterlist_dialog/characterlistdialog.cpp`, `test/selftest.cpp`

**What changed:**
- New `addCharacterFromFile()` method — loads a character and ADDS to the party (doesn't replace)
- `CharacterListDialog::onSelectClicked()` now uses `addCharacterFromFile()` instead of `loadCharacterFromFile()`
- `saveCharacterToFile()` now saves: hunger, maxHunger, gold, mana, maxMana, playTimeSeconds, experience, inventory (JSON), equipped (JSON), bankInventory (JSON)
- `loadCharacterFromFile()` for .txt files now parses: hunger, maxHunger, gold, mana, maxMana, playTimeSeconds, experience, inventory (JSON), equipped (JSON), bankInventory (JSON)
- `saveCharacterToFile()` reads from `PartyManager` (single source of truth) instead of stale `m_gameStateData["Party"]`

**Verified:** tests [230]–[239] — 4-character party round-trip, party save/load, autosave, bank inventory, equipped items, guild data, play time/NG+, party building, MAX_PARTY_SIZE, independent inventories. Non-vacuous: reverting `onSelectFromFile()` to `loadCharacterFromFile()` fails test [237].

### 5.3 CharacterListDialog UI update `S`

**Files:** `src/characterlist_dialog/characterlistdialog.cpp`, `src/characterlist_dialog/characterlistdialog.h`, `test/selftest.cpp`

**Verify:** dialog shows selected characters with preview, party size indicator, clear selection button.

### 5.4 LoadPartyDialog preview `S`

**Files:** `src/load_party_dialog/LoadPartyDialog.cpp`, `src/load_party_dialog/LoadPartyDialog.h`, `test/selftest.cpp`

**Verify:** dialog shows party preview with member names, levels, inventory count.

### 5.5 Save format migration `S`

**Files:** `gameStateManager.cpp`, `gameStateManager.h`, `test/selftest.cpp`

**Verify:** old .txt files without inventory/hunger/guild data load with defaults, new .txt files with full data load correctly.

### 5.6 Release `S`

**Files:** `blacklands.pro`, `src/core/ReleaseInfo.cpp`, `RELEASE_NOTES.md`

**Verify:** SEMVER_MINOR bumped to 2, version history updated, release notes written.

## Dependency Graph

```
5.1 Autosave fix → 5.2 Party building → 5.3 CharacterListDialog UI → 5.4 LoadPartyDialog preview → 5.5 Save format migration → 5.6 Release
```

## Release Criteria

- [ ] Autosave saves party with inventory (test [232])
- [ ] Party building from individual characters works (test [237])
- [ ] All tests pass (target: 1861+)
- [ ] `make check` is green
- [ ] Non-vacuous verification for every fix
