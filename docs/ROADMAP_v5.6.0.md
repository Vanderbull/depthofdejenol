# v5.6.0 — Wire & Cleanup Round 2

## Status: IN PROGRESS

### 6.0a — Remove dead DungeonMap ✅
**Why:** `src/dungeonmap/dungeonmap.{cpp,h}` is never included or instantiated by any production code. DungeonMinimap and AutomapDialog handle their own rendering.
**Do:** Delete `src/dungeonmap/dungeonmap.cpp` and `src/dungeonmap/dungeonmap.h`.
**Verify:** Build succeeds, tests pass.

### 6.0b — Helplesson dwarf image ✅ DONE
**Why:** Label always shows "Dwarf Image\nPlaceholder" — no real image is loaded.
**Do:** Hide the label since no dwarf image resource exists.
**Verify:** Non-vacuous: restoring the placeholder fails the test.

### 6.1 — Options backup timestamp ✅ DONE
**Why:** `lastBackupLabel` was set to "Placeholder date" and never updated after backup.
**Do:** Update label with actual timestamp after backup and restore.
**Verify:** Non-vacuous: removing the update fails the test.

### 6.2 — Release ✅ DONE
**Why:** Final verification.
**Do:** Clean build, run tests, tag v5.6.0.
**Verify:** 1762 tests pass, game runs clean.
