# v5.6.0 — Wire & Cleanup Round 2

## Status: IN PROGRESS

### 6.0a — Remove dead DungeonMap ✅
**Why:** `src/dungeonmap/dungeonmap.{cpp,h}` is never included or instantiated by any production code. DungeonMinimap and AutomapDialog handle their own rendering.
**Do:** Delete `src/dungeonmap/dungeonmap.cpp` and `src/dungeonmap/dungeonmap.h`.
**Verify:** Build succeeds, tests pass.

### 6.0b — Helplesson dwarf image ✅
**Why:** Label always shows "Dwarf Image\nPlaceholder" — no real image is loaded.
**Do:** Load a real image from resources, or hide the label if no image exists.
**Verify:** Test proves image is loaded or label hidden.

### 6.1 — Options backup timestamp ✅
**Why:** `lastBackupLabel` is set to "Placeholder date" and never updated after backup.
**Do:** Update label with actual timestamp after backup/restore.
**Verify:** Test proves label updates.

### 6.2 — Release ✅
**Why:** Final verification.
**Do:** Clean build, run tests, tag v5.6.0.
**Verify:** All tests pass.
