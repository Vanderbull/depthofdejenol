# v5.9.0 — Dead Code Removal

## Status: IN PROGRESS

### 9.0a — Remove dead code files
**Why:** 17 .cpp/.h files in src/ are not in blacklands.pro and not included by any other file:
- `src/automap/automap_dialog_old.h` — old automap header, never included
- `src/compass/` (3 files) — standalone compass app with own main()
- `src/map_dialog/Map.cpp` — only included by MapWidget.cpp (also dead)
- `src/map_dialog/MapWidget.cpp` — never included by production code
- `src/map_widget/` (3 files) — duplicate map widget, never included
- `src/server/main.cpp` — standalone server prototype, never integrated
- `src/server/moc_predefs.h` — generated file, not source
- `src/wav_manager/` (3 files) — standalone WAV manager with own main()

**Do:** Delete all dead files. Remove empty directories.
**Verify:** Build succeeds, tests pass.

### 9.0b — Release
**Why:** Final verification.
**Do:** Clean build, run tests, tag v5.9.0.
**Verify:** All tests pass.
