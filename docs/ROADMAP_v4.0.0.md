# Roadmap v4.0.0 — Endgame Depth and Replayability

**Status:** Not started
**Version:** v4.0.0 (major — extends the game past victory with NG+, deeper endgame content, and replayability hooks)
**Test suite at start:** 1448 + 3 (all passing)

## Goal

v3.0.0 made combat tactical and growth chosen. But once you beat the final boss, the game
is over — there is no reason to keep playing. v4.0.0 adds:

- **New Game Plus**: carry your party forward with a difficulty multiplier and fresh rewards.
- **Endgame dungeon**: a post-victory mega-dungeon with unique bosses and loot.
- **Arena / challenge mode**: repeatable combat challenges with leaderboard scoring.
- **Achievements**: persistent milestones that unlock cosmetics and bragging rights.
- **Balance pass**: ensure the full run (floor 1 to victory) is winnable but not trivial.

This is a major bump because it changes the game's arc: from "win once and stop" to
"win, then push further."

## Slices

### 4.1 New Game Plus `M` — ✅ DONE

**Why:** `Endgame` already had a "New Game Plus" button, but it did nothing — it just
restarted the game without carrying anything over. Players who beat the final boss had
no reason to keep playing.

**Done:**
- `gameStateManager::startNewGamePlus()` — increases NG+ level, resets dungeon/quest/playtime, keeps party
- `getNgPlusLevel()` / `setNgPlusLevel()` — read/write NG+ level in game state
- `EncounterBuilder` — accepts `ngPlusLevel`, scales monsters with `ngPlusMonsterMultiplier` (+50% per cycle)
- `VictoryReward` — uses `ngPlusRewardMultiplier` (+25% per cycle)
- Save versioning: `CURRENT_SAVE_VERSION` bumped to 3, migration handles NGPlusLevel
- NGPlusLevel persists through save/load

**Files:** `src/core/gameStateManager.cpp`, `src/core/savegameVersion.cpp`, `src/combat/EncounterBuilder.cpp`, `src/combat/VictoryReward.cpp`, `src/dungeon_dialog/DungeonDialog.cpp`

**Verify:** tests [162]-[166] — NG+ level increments ✅; party level and gold preserved ✅; NGPlusLevel persists through save/load ✅; NGPlusLevel defaults to 0 for old saves ✅.

### 4.2 Endgame dungeon `M` — ✅ DONE

**Why:** after beating the final boss, there was nowhere left to go. A post-victory
mega-dungeon gives players a reason to keep playing.

**Done:**
- `AbyssThemes` — 10-floor post-victory mega-dungeon with unique themes
- Boss floors 5 and 10 with `Abyssal Maw` and `The Abyssal Heart`
- `BossEncounter::buildAbyssBoss()` — multiplicative scaling (floor 10 significantly harder than floor 5)
- Abyss boss XP far above main dungeon bosses
- Version bumped to 4.0.0

**Files:** `src/core/AbyssThemes.h` / `.cpp`, `src/core/BossEncounter.h` / `.cpp`

**Verify:** tests [167]-[170] — Abyss has 10 floors ✅; boss floors are 5 and 10 ✅; floor 10 boss has >2x HP of floor 5 ✅; Abyss floor 5 boss XP > main floor 15 boss XP ✅.

### 4.3 Arena / challenge mode `M`

**Why:** combat is now tactical, but there is no way to test your build against
increasing difficulty outside the main quest. An arena gives players a reason to
keep fighting after victory.

**Do:**
- Add an arena mode: 10 waves of monsters, each harder than the last.
- Score is based on speed, damage taken, and waves cleared.
- Leaderboard stored in the Hall of Records.
- Accessible from town (not just the dungeon).

**Files:** `src/dungeon_dialog/DungeonDialog.cpp`, `src/core/Endgame.cpp`, `src/combat/EncounterBuilder.cpp`

**Verify:** the arena is accessible from town; the waves get harder; the score
is recorded; the leaderboard sorts correctly.

### 4.4 Achievements `M`

**Why:** there are no persistent milestones. Achievements give players long-term
goals and a sense of progression beyond leveling.

**Do:**
- Add an achievement system: ~20 achievements for milestones (e.g., "Beat floor 10",
  "Defeat the final boss", "NG+ 3", "Earn 10000 gold", "Die 10 times").
- Track achievement progress in the save file.
- Show unlocked achievements in the Hall of Records.
- Optional: grant a small reward (gold, XP, or a title) for each achievement.

**Files:** `src/core/AchievementSystem.h` (new), `src/core/AchievementSystem.cpp` (new), `src/core/Endgame.cpp`, `src/core/gameStateManager.cpp`

**Verify:** achievements unlock at the right milestones; they persist across saves;
the Hall of Records shows them; the leaderboard includes them.

### 4.5 Balance pass `M`

**Why:** the full run (floor 1 to victory) has never been playtested end to end.
Some fights may be trivially easy or impossibly hard.

**Do:**
- Playtest the full run and adjust monster stats, loot tables, and XP curve.
- Ensure the game is winnable in 10–15 hours of play.
- Add a "recommended level" hint to each dungeon floor.
- Adjust the NG+ difficulty multiplier if it feels too easy or too hard.

**Files:** `src/combat/EncounterBuilder.cpp`, `src/core/MonsterBalance.cpp`, `src/core/ItemProgression.cpp`

**Verify:** a full playthrough is winnable; the recommended level hints are accurate;
the NG+ multiplier feels right.

### 4.6 Repo and docs cleanup `S`

**Why:** the repo has accumulated a lot of old roadmaps and docs. Only the latest
should be prominent.

**Do:**
- Move old roadmaps (v0.1 through v3.0.0) to a `docs/archive/` subdirectory.
- Update the README to link only to the latest roadmap.
- Update `FIXLIST.md` to mark resolved items.

**Files:** `docs/`, `README.md`, `FIXLIST.md`

**Verify:** the README links to the latest roadmap; the old roadmaps are in
`docs/archive/`; `FIXLIST.md` is up to date.

### 4.7 Release `S`

**Why:** v4.0.0 needs a proper release with notes and a version bump.

**Do:**
- Bump `SEMANTIC_VERSION` to `4.0.0`.
- Update `RELEASE_NOTES.md` with all v4.0.0 changes.
- Tag the release in git.
- Update `ReleaseInfo::versionHistory()` and `releaseDate()`.

**Files:** `src/core/GameConstants.h`, `RELEASE_NOTES.md`, `src/core/ReleaseInfo.cpp`

**Verify:** `make check` is green; the version is 4.0.0 in the about dialog; the
release notes are complete.

## Dependency Graph

```
4.1 NG+ ──── 4.2 Endgame dungeon (needs NG+ counter)
           └─ 4.3 Arena (needs NG+ difficulty)
4.4 Achievements (independent)
4.5 Balance pass (independent)
4.6 Repo cleanup (independent)
4.7 Release (after all others)
```

**Critical path:** `4.1 → 4.2` — NG+ is the foundation for the endgame content.

**Ordering note:** land 4.1 (NG+) before 4.2 and 4.3 (which depend on it). Land 4.6
(repo cleanup) before 4.7 (release).

## Release Criteria

- [x] New Game Plus carries the party and scales difficulty
- [x] Endgame dungeon is accessible and beatable
- [ ] Arena mode is accessible and scored
- [ ] Achievements unlock and persist
- [ ] Full playthrough is winnable in 10–15 hours
- [ ] Old roadmaps are archived
- [ ] `SEMANTIC_VERSION` is `4.0.0`
- [ ] `RELEASE_NOTES.md` is updated
- [ ] All tests pass (target: 1500+)
- [ ] `make check` is green

## Slice Sizes

| Slice | Size | Depends on |
|-------|------|------------|
| 4.1 NG+ | M | — |
| 4.2 Endgame dungeon | M | 4.1 |
| 4.3 Arena | M | 4.1 |
| 4.4 Achievements | M | — |
| 4.5 Balance pass | M | — |
| 4.6 Repo cleanup | S | — |
| 4.7 Release | S | 4.1–4.6 |

---

**Target:** 7 slices, all `M` or `S`. The focus is on endgame depth and replayability —
giving players a reason to keep playing after victory.
