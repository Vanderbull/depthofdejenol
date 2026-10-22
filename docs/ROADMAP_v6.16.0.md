# Roadmap v6.16.0 — Dungeon, Quest, Town & Monsters (LVC)

**Latest release:** `v6.15.0` (tagged, clean tree: `67f5ee6`). Companion core, Confinement & Holding and Economy are done. What remains in the MORDOR manual is the heart of the game:

|| Manual § | Topic | Status today |
||----------|-------|--------------|
|| ~2 "Dungeon" | Monster count, corridors, themes, multi-level monsters, stairs, progressive complexity | Code exists; room geometry and theme mapping wired, theme names in levels.json |
|| ~3 "Quest" | Main quest chain, quests, quest rewards, progression | Quest chain driver exists (QuestChainDriver), main quest chain stubbed |
|| ~4 "Town" | Tavern, bank, quest board, NPCs, alignment | Town Dialog exists; NPCs and quest board partial |
|| ~7 "Monster and Treasure" | Monster hints, equipment, gold sinks (hints, highlight, etc.) | Monster hints partial; equipment not implemented |

**Why v6.16.0:** The dungeon, quest, town and monster sections are the remaining core gameplay. LVC here means wiring the existing structures and handlers to real data — no new subsystem (AI, sound, cross-server) this week.

---

## Proposed slices (LVC-first, easiest first)

|| # | Slice | LVC target | Hours | Cost |
||---|-------|-----------|-------|------|
|| 0 | Dungeon: room geometry & theme | Monster count, corridors, themes, multi-level monsters, stairs, progressive complexity | 1.5 | 1350 kr |
|| 1 | Quest: main chain | Main quest chain, quests, quest rewards, progression | 1.0 | 900 kr |
|| 2 | Town: tavern, bank, quest board, alignment | Tavern, bank, quest board, NPCs, alignment | 1.0 | 900 kr |
|| 3 | Monster & Treasure | Monster hints, equipment, gold sinks | 1.0 | 900 kr |
|| **Total** | | | **4.5 h** | **4500 kr** |

*Under the 10 000 kr/week cap (~5 500 kr ≈ 6 h remaining). No refinement — LVC only.*

---

## Test register v6.16.0

**Policy:** every check is registered **before** it is run, including the ones that fail. A failing check is retried only after the data it depends on is fixed — it is never re-run against the same stale state.

|| # | Check | Expected (LVC) | How verified | Status |
||---|-------|----------------------|--------------|--------|
|| 0.1 | Monster geometry | Monster levels and themes appear in the dungeon | harness: read dungeon level, count monsters, check theme | ☐ |
|| 0.2 | Multi-level monsters | Some monsters have z=-1 and stairs; free a level with a level-up | harness: free a level, check stair tile | ☐ |
|| 0.3 | Monster progression | Larger floors have more monsters | harness: compare monster count across floors | ☐ |
| 1.1 | Main quest chain | Completing the main quest ends the run; NG+ raises level | harness: advance quest chain, check victory | ✅ PASS (21/21) — full chain drive: depth 1 → boss 5 → depth 10 → boss 10 → depth 15 → boss 15; `chainJustCompleted` set, `isChainComplete()` true, `nextStepIndex()` −1; totals 68 600 g / 340 100 XP = Σ step rewards; NG+ 0→1, `DeepestFloor`→0, chain incomplete again |
| 1.2 | Quest rewards | Completing a quest grants gold and XP | harness: complete a quest, check rewards | ✅ PASS — step 0 pays exactly +100 g / +100 XP (`QUEST_S0_GOLD`/`QUEST_S0_XP`); `QuestRewardGiven_first_descent` persisted; second `advance()` does not double-pay |
|| 2.1 | Tavern | Tavern Dialog opens, rest/cure/advance work | harness: open Tavern, rest/cure/advance | ✅ PASS — opens; rest spends 80 g & restores HP; advance ages party; cure no-op when nothing to cure |
|| 2.2 | Bank | Bank Dialog opens, deposit/withdraw works | harness: deposit/withdraw works | ✅ PASS — deposit all empties party gold to 0 & bank +920; withdraw all restores both |
|| 2.3 | Alignments | Enemy types depend on alignment | harness: change alignment, check enemies | ✅ PASS — Evil faces only high-alignment (holy) monsters (57), Neutral only neutral (92), Good only low (92); sets differ by alignment |
|| 3.1 | Monster hints | Monster hints show level, HP, attack, defence | harness: read monster hint | ☐ |
|| 3.2 | Equipment | Equipment modifies stats | harness: equip an item, check stats | ☐ |

**Run order:** 0.1→0.3 (dungeon), then 1.1→1.2 (quest), then 2.1→2.3 (town), then 3.1→3.2 (monster & treasure). Gates: slice N passes before slice N+1 starts. A failed check is recorded with its failure reason and retried only after the fix — never hidden.

---

## Key findings (what the analysis showed)

1. `struct Companion` in `src/character/character.h` carries mangled field names (`M548B`, `M4ED4`, …) — reverse-engineered residue, not clean source. Treat it as a data carrier, not a spec.
2. QuestChainDriver is wired and the main quest chain now has six real steps (depth 1 → boss 5 → depth 10 → boss 10 → depth 15 → boss 15) with per-step gold/XP from `GameConstants`. Reached from `CombatController::handleVictory()`; surfaced in `QuestChainDialog` (city button).
3. Town Dialog is largely a stub; tavern/bank/quest board/alignment are the cheapest LVC path.
4. Monster hints are partial; a cheap approach is to show level/HP/attack/defence from the bestiary.

---

## Risks

- Monster geometry is the highest-risk slice (may need refinement). If it exceeds 4 h, downscope to monster layout and theme assignment only.
- Quest chain is done and verified (1.1, 1.2 PASS). Residual risk: `hasLevel()` requires `generated == true` while boss kills are written via `levelForEdit()`, so a boss killed on a floor without a generated snapshot would be silently dropped — no such path exists today, but the two APIs disagree on the precondition.
- Alignment system requires balancing with enemies; start with a small map.
- Monster hints require bestiary data; reference existing bestiary JSON.
