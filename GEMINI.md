## Coding Standards
*   **Types (Classes/Structs/Enums):** Use `camelCase` (e.g., `dungeonMap`).
*   **Methods & Functions:** Use `camelCase` (e.g., `loadLevel()`).
*   **Member Variables:** Use `m_camelCase` for private members (e.g., `m_currentFloor`) and `camelCase` for public members.
*   **File Names:** Match the class name (e.g., `GameStateManager.cpp`).

## Build and Verify

    make -j$(nproc)      # build; expect zero new warnings
    make check           # build, then run the headless self-test suite

`make check` runs `./blacklands --selftest`, which executes `test/selftest.cpp`
without a GUI. It exits non-zero on failure, so it is the regression gate: any
change that is supposed to guarantee a behaviour gets a check there. Run the
game manually with `./blacklands`.

`blacklands.pro` declares the `check` target explicitly. qmake only generates its
own `check: first` when the project has not defined one, so declaring it here
survives `qmake` regeneration. Edit the `.pro`, never `Makefile` directly —
`Makefile` is generated.

Note: the game logs heavily during startup (full data dumps), so the self-test
silences Qt logging while it runs; results go straight to stdout.

## State Layer

`PartyManager` is the single source of truth for the party. `gameStateManager`
holds no party copy — read through `getParty()`, `getPartyGold()`,
`getCurrentCharacter()`, `getPartyMember()`, and write through the same paths.
There used to be a second `m_currentParty` mirror; it was removed because the two
drifted and caused stale-gold and stale-inventory bugs.

Party data serialises via `Party::toMap()` / `Party::loadFromMap()`.
`loadFromMap()` replaces the member list, it does not append. Any new field that
must survive a save has to be added to **both** `toMap()` and `loadFromMap()` —
`isAlive` was silently lost this way for a long time.
