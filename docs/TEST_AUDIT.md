# Test Audit — v6.5.0 → next

**Date:** 2026-10-14
**Scope:** every test section in `test/` — find and remove tests that no longer
test anything (vacuous assertions, exact duplicates, stubs).

**Method:** extracted all 278 `section(...)` blocks across the six suite files,
then scanned each block for:
1. literal-true assertions — `check(true, …)`, `check(1, …)`
2. tautologies — `check(x == true || x == false, …)` (always true for a bool)
3. empty bodies — loops with no work, blocks with no `check()`
4. exact duplicates — the same section twice
5. assertions that cannot fail — a check whose condition was used to select the
   value it then asserts on
6. redundant coverage — a stub for something a real test already covers

## Result

| | Before | After |
|---|---|---|
| Test checks | 1911 passed, 0 failed | **1891 passed, 0 failed** |
| Sections (`section(...)`) | 278 | 267 |
| Vacuous `check(true)` | 15 | 0 |

**−20 checks, −11 sections, suite still green.**

---

## Removed — whole sections (tested nothing)

| Section | File | Why removed |
|---|---|---|
| `[82] VictoryReward returns loot for known monsters` | selftest_world.cpp | body ended in `check(true, "calculateLoot returns without crash")` — no assertion |
| `[83] loot filtered by dungeon depth` | selftest_world.cpp | body ended in `check(true, …)`; real depth/tier gating is tested in `[70]` |
| `[102] confusion causes friendly fire` | selftest_world.cpp | loop body was an empty comment placeholder + `check(true, …)`; the real test is `[21]` (combat), which rolls 1000 attacks and asserts `friendlyFire > 0` |
| `[103] monster fire breath sets on fire` | selftest_world.cpp | `check(true, "monster fire breath doesn't crash")` — never asserts the fire state |
| `[110] uncurse removes cursed flag` | selftest_world.cpp | asserted `cursedDef->cursed` on a value selected *by* `def.cursed` — a tautology; real uncurse coverage is `[13]` in selftest_core.cpp |
| `[154] startPlayTime/playTimeSeconds returns non-zero` (2nd copy) | selftest_world.cpp | exact duplicate of the `[154]` at line 4081 |
| `[155] PlayTimeSeconds persists through save/load` (2nd copy) | selftest_world.cpp | exact duplicate of the `[155]` at line 4092 |
| `[161] flashCombatFx does not crash without combat group` | selftest_world.cpp | contained **no `check()` at all** — the whole section was unverifiable |
| `[278] arena combat state cleanup` | selftest_world.cpp | `check(true, "ArenaDialog constructs without crash")` — no assertion |
| `[280] arena item usage` | selftest_world.cpp | only real check was `hero.inventory.size() == 1`; the item path was `check(used == true \|\| used == false, …)`. Consumable use is tested in `[11]`/`[131]` (core) |
| `[281] arena waves cleared tracking` | selftest_world.cpp | `check(true, "arena waves cleared tracking is set up")` — no assertion |

## Removed — dead blocks inside otherwise-good sections

| Block | File | Why removed |
|---|---|---|
| `[16]` `isPlayerTurn` block | selftest_combat.cpp | `check(isPlayer == true \|\| isPlayer == false, …)` — tautology |
| `[22]` "Loot: filtered by dungeon depth" block | selftest_combat.cpp | computed loot then `check(true, "loot calculation runs")`; the rest of `[22]` (11 real checks) is intact |
| `[12]` "Identify an item" block | selftest_core.cpp | superseded by `[60c]`; ended in `check(true, "identifyItem callable")` |
| `[62g]` second `AutomapDialog` block | selftest_world.cpp | `check(true, "AutomapDialog updatePlayerPosition works")` — the first block keeps the real construction check |
| `[75]` one line | selftest_world.cpp | `check(true, "applySpellDamageBySchool executed without crash")`; the section keeps its real `hpAfter != hpBefore` assertion |

## Converted — silent skips → loud failures

| Location | Before | After |
|---|---|---|
| `equipment_test.cpp` | `check(true, "no guild-restricted items found (skip)")` | `check(false, …)` — the DB has such items; a missing one is a real problem |
| `[117]` quest turn-in | `check(true, "no kill quest posted (skip)")` | `check(false, …)` — same reasoning |

A test that cannot run its assertion must not report PASS.

---

## Left as-is (intentional)

- **Conditional `check(false, …)` guards** — `[4]`, `[7]`, `[139]`–`[141]` etc.
  use `check(false, …)` inside `else` branches when a test file cannot be
  written. These are correct: they fail loudly if setup breaks.
- **`[56] Release packaging`** — asserts the version string in five places. Brittle
  (needs updating every release) but it catches real packaging regressions.
- **Weak-but-real assertions** — e.g. `check(x >= 0, …)`. Not vacuous; they do
  constrain behaviour.
- **Duplicate section *numbers*** — `[42]` (Tavern / Quest board / Alignment) and
  `[43]` (Character sheet / NPC dialog) are distinct tests that share a number.
  Cosmetic only; matched by title, never by number.

## How this was verified

- Brace balance re-checked after every edit (a naive `{`/`}` count plus a
  string/comment-aware scan) — a first attempt with fuzzy matching silently
  over-deleted and broke compilation; redone with exact line ranges.
- `make` clean, `make check` green: **1891 passed, 0 failed**.
- The diff removes exactly the 11 sections listed above (verified with
  `git diff | grep '^-.*section('`).
- The one section with a real assertion kept in full — `[22]` still runs its
  11 XP/gold/drop-table checks.
