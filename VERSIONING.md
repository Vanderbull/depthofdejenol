# Save Versioning and Migration Policy

## Schema version

`SaveVersion` is an integer written into every save file. The current schema
version lives in `src/core/savegameVersion.h` as `CURRENT_SAVE_VERSION`.

| Version | Release | What changed |
|---------|---------|--------------|
| 0       | pre-v2.0.0 | No `SaveVersion` field written (legacy saves). |
| 1       | v2.0.0     | `SaveVersion` written for the first time. |
| 2       | v3.0.0     | Added `StatPoints`, `PlayTimeSeconds`, `Row` on characters. |

## Rules

1. **Writing.** Every `saveFullGameState()` writes `CURRENT_SAVE_VERSION` into
   `m_gameStateData["SaveVersion"]`.
2. **Reading.** On `loadFullGameState()` the field is read. A missing field is
   treated as version 0.
3. **Migration.** If the loaded version is older than `CURRENT_SAVE_VERSION`,
   migrations run in order, each bringing the save one step up. New fields are
   filled with safe defaults. After migration the version field is bumped.
4. **Refusal.** If the loaded version is *newer* than `CURRENT_SAVE_VERSION`
   the load is refused with a clear message. A newer build must not silently
   truncate fields it does not know about.

## Adding a new version

When a slice persists a new field:

1. Bump `CURRENT_SAVE_VERSION` in `savegameVersion.h`.
2. Add a `migrateFrom<X>()` function in `savegameVersion.cpp`.
3. Add a call to it in the migration chain inside `loadFullGameState()`.
4. Write a test that loads a hand-written save at the old version and checks the
   new field's default.
