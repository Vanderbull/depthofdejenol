# v5.7.0 — Wire & Cleanup Round 3

## Status: IN PROGRESS

### 7.0a — Remove unconnected dialogs ✅
**Why:** 5 dialogs are built into the game but never opened by any code path:
- `AssemblePartyDialog` — replaced by main menu party system
- `AboutDialog` — never opened
- `SenderWindow` — never opened
- `LoadingScreen` — never opened
- `ShortcutHelp` — never opened

**Do:** Delete all 5 dialog source directories and remove from `blacklands.pro`.
**Verify:** Build succeeds, tests pass.

### 7.0b — Fix missing QObject includes ✅ DONE (N/A)
**Why:** `build/moc/moc_AboutDialog.cpp` showed `#error "The header file 'AboutDialog.h' doesn't include <QObject>"`.
**Do:** N/A — all Q_OBJECT headers already include `<QObject>`.
**Verify:** `grep -rl "Q_OBJECT" src/ --include="*.h" | while read f; do grep -q "#include <QObject>" "$f" || echo "$f"; done` returns empty.

### 7.1 — Release ✅ DONE
**Why:** Final verification.
**Do:** Clean build, run tests, tag v5.7.0.
**Verify:** 1752 tests pass, game runs clean.
