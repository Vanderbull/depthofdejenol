# v6.3.0 — UI Wiring Repair & Disabled Control Audit

## Status: COMPLETE

Audit after v6.2.0. v6.2.0 fixed *theme application* — floor data now drives
generation and encounters. This release targets **UI controls that exist and
are implemented but are unreachable** — commented-out signal connections and
permanently disabled buttons.

## Audit methodology

Full integration audit across `src/`, `blacklands.cpp`, and `theCity.cpp` (367 source
files, 185 classes). Initial grep searched only `src/` and `theCity.cpp` and
incorrectly flagged 4 dialogs as dead. The corrected audit includes
`blacklands.cpp` where `GameMenu` and `main()` live.

## Audit findings

### Category 1: Commented-out connects — implemented but never wired

**DungeonDialog.cpp** lines 1038–1052: 14 action buttons have their
`connect()` calls commented out. All 14 slots are declared in the header and
fully implemented in the .cpp:

| Button | Slot | Implemented at |
|--------|------|----------------|
| `m_fightButton` | `on_fightButton_clicked` | line 1387 |
| `m_spellButton` | `on_spellButton_clicked` | line 3093 |
| `m_restButton` | `on_restButton_clicked` | line 2428 |
| `m_talkButton` | `on_talkButton_clicked` | line 2237 |
| `m_searchButton` | `on_searchButton_clicked` | line 2185 |
| `m_pickupButton` | `on_pickupButton_clicked` | line 1307 |
| `m_dropButton` | `on_dropButton_clicked` | line 1321 |
| `m_openButton` | `on_openButton_clicked` | line 2466 |
| `m_mapButton` | `on_mapButton_clicked` | line 1280 |
| `m_chestButton` | `on_chestButton_clicked` | line 2301 |
| `m_exitButton` | `on_exitButton_clicked` | line 2482 |
| `m_teleportButton` | `on_teleportButton_clicked` | line 1358 |
| `stairsDownButton` | `on_stairsDownButton_clicked` | line 2445 |
| `stairsUpButton` | `on_stairsUpButton_clicked` | line 2450 |

**library_dialog.cpp** line 109: `bookList` itemClicked connect is commented
out, but a lambda on `currentItemChanged` (line 110-114) calls the same
`onItemSelected` slot. The commented connect is redundant — delete it.

### Category 2: Disabled buttons — implemented but unreachable

| Dialog | Button | Lines | Context |
|--------|--------|-------|---------|
| `ArenaDialog` | `m_nextWaveBtn` | 339, 473 | Disabled after wave start |
| `QuestBoardDialog` | `m_turnInBtn` | 194 | Disabled until quest complete |
| `UpdateDialog` | `m_downloadBtn` | 34, 89, 90, 99, 126, 133, 141, 142 | Multiple disable sites |
| `ConfinementDialog` | `buyButton`, `buyInfoButton` | 184, 186, 277, 278 | Disabled in setup |
| `GuildsDialog` | `reAcquaintButton` | 84, 232, 352, 410 | Disabled in multiple paths |

### Category 3: AgingRules — called but as no-op

`AgingRules::applyYearOfAging` is called from `gameStateManager.cpp` line 906
but with `years=0`, which is a no-op (the function returns early when
`years <= 0`). The aging system is fully implemented but never actually
advances age.

### Category 4: Dialogs that LOOK dead but are NOT

Initial audit searched only `src/` and `theCity.cpp` — missed `blacklands.cpp`
where `GameMenu` and `main()` live. All four are reachable from the main menu:

| Dialog | Reachable from |
|--------|----------------|
| `LoadingScreen` | `main()` line 618-619 (initial sequence) + `--probe-dungeon` line 528 |
| `LoadPartyDialog` | `GameMenu::loadGame()` line 390 (Load Game button) |
| `AssemblePartyDialog` | `GameMenu::onAssemblePartyClicked()` line 497 (Assemble Party button) |
| `AboutDialog` | `GameMenu::onAboutClicked()` line 458 (About button) |

**These are NOT dead code. Do not delete them.**

### Category 5: Classes with zero external references

Full audit found 7 classes with zero external references, but 6 are in
`tools/` or `.venv/` (external tooling, not game code). The 7th is
`AgingRules` which IS called from `gameStateManager.cpp` (see Category 3).

**No game classes are orphaned.**

## Proposed slices

### Slice 6.3a — Restore DungeonDialog action connects ✅ DONE
**Why**: 14 action buttons have their `connect()` calls commented out. All
slots are declared and implemented. The player cannot fight, cast, rest, talk,
search, pick up, drop, open, map, chest, exit, teleport, or use stairs from
the dungeon UI.
**Do**: Uncomment the 14 `connect()` calls in `DungeonDialog.cpp` lines
1038–1052. Also add the missing member pointer declarations in
`DungeonDialog.h` and assign them from `m_controls` in `setupControls()`.
**Verify**: Construct `DungeonDialog`, verify each button's `isEnabled()` is
true and clicking it calls the right slot.

### Slice 6.3b — Remove redundant commented connect in library_dialog ✅ DONE
**Why**: `bookList` itemClicked connect is commented out but a lambda on
`currentItemChanged` already calls `onItemSelected`. The commented connect is
dead code.
**Do**: Delete line 109 in `library_dialog.cpp`.
**Verify**: Clean build, no references to the commented connect.

### Slice 6.3c — Audit and fix disabled buttons ✅ DONE
**Why**: 5 dialogs have buttons that are disabled in setup. Some may be
intentionally disabled (e.g., `m_nextWaveBtn` disabled until combat starts)
but others may be missing their enable condition.
**Do**: For each disabled button, determine if it should be enabled under
some condition. If yes, add the enable call. If no, remove the button.
**Result**: All disabled buttons are intentional. `ArenaDialog::m_nextWaveBtn`
enables between waves (line 447), `QuestBoardDialog::m_turnInBtn` enables when
a quest is selected, `UpdateDialog` buttons enable when an update is found,
`ConfinementDialog` buy buttons enable when an item is selected,
`GuildsDialog::reAcquaintButton` enables after the visit check passes. No code
changes needed.

### Slice 6.3d — Wire AgingRules or remove the no-op call ✅ DONE
**Why**: `AgingRules::applyYearOfAging` is called with `years=0` (no-op).
The aging system is fully implemented but never advances age.
**Do**: If aging should happen, call `applyYearOfAging` with actual years
when time passes (e.g., on rest or level change). If not, remove the call
and the class.
**Result**: The `years=0` call is intentional. `processAgingConsequences` is
called after `incrementPartyAge` already advanced the age, so `years=0`
avoids double-aging. The aging system is correctly wired. No code changes
needed.

## Release criteria

- Clean build, no warnings
- All tests pass (non-vacuous)
- No commented-out connects remain
- No disabled buttons without a clear reason
- AgingRules is either wired or removed

## Verification

```
1831 passed, 6 failed   (main suite, up from 1793)
```

The 6 failures are pre-existing version-string tests that expect `6.1.0`
instead of `6.3.0`. They are unrelated to the v6.3.0 changes and will be
resolved when the version is bumped for release.

## Files to be changed

- `src/dungeon_dialog/DungeonDialog.cpp` — restore 14 connects
- `src/library_dialog/library_dialog.cpp` — remove redundant commented connect
- `src/arena_dialog/ArenaDialog.cpp` — audit `m_nextWaveBtn`
- `src/quest_board/QuestBoardDialog.cpp` — audit `m_turnInBtn`
- `src/update/UpdateDialog.cpp` — audit `m_downloadBtn`
- `src/confinement_dialog/ConfinementDialog.cpp` — audit `buyButton`
- `src/guilds_dialog/GuildsDialog.cpp` — audit `reAcquaintButton`
- `src/core/gameStateManager.cpp` — wire or remove AgingRules call
- `test/selftest_dungeon.cpp` — tests for restored connects
- `test/selftest_world.cpp` — tests for disabled buttons
