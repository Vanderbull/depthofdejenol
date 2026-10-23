# Roadmap v6.15.0 — Companions, Holding & the economy (LVC)

**Latest release:** `v6.14.0` (tagged, clean tree: `fcbb075`). AutoMap (§1),
Tutorial (§71) and Spell classes (§6) are done. What remains in the MORDOR manual
is the heart of the game:

| Manual § | Topic | Status today |
|----------|-------|--------------|
| ~8 "Companions" | Mages compel monsters into companions; Confinement & Holding centre | **Code exists, handlers are stubs** — `Companion` struct + `ConfinementAndHoldingDialog` (bind/identify/sell/buy), but `bindCompanion()` etc. only show a message box. |
| ~5 "Treasure, Item and You" | Identification, factions, containers, gold sinks | Store exists; identification fees and gilden contracts are **missing**. |
| "Library of Knowledge" | Dungeon map / bestiary expansion | Partial (JSON files exist). |

**Why v6.15.0:** Companions is the single biggest "manual-faithful" feature that
is *code-complete in shape but empty in behaviour*. LVC here means wiring the
existing `Companion` structs and `Confinement` dialog to real data — no new
subsystem (AI, range, factions) this week.

---

## Proposed slices (LVC-first, easiest first)

| # | Slice | LVC target | Hours | Cost |
|---|-------|-----------|-------|------|
| 0 | Companion core | A Mage can compel a level-1 monster into a Companion. The Companion joins the party, has name+race+level+HP, spawns in the dungeon, fights alongside the party, and sells at the Confinement centre. | 1.5 | 1350 kr |
| 1 | Confinement & Holding | Existing dialog buttons (bind/identify/sell/buy) connect to data: sell a companion, buy a new one from a data structure, pay fees. | 1.0 | 900 kr |
| 2 | Economy: identification + gilden | Store charges an identification fee; a "gilden"/contract list pays gold per quest; chests/containers work. | 1.5 | 1350 kr |
| **Total** | | | 4.0 h | **3600 kr** |

*Well under the 10 000 kr/week cap (~6 400 kr ≈ 7 h remaining). No refinement —
LVC only.*

---

## Test register v6.15.0

**Policy:** every check is registered **before** it is run, including the ones
that fail. A failing check is retried only after the data it depends on is
fixed — it is never re-run against the same stale state.

| # | Check | Expected (LVC) | How verified | Status |
|---|-------|----------------|--------------|--------|
| 0.1 | Companion joins from Mages | `Character::companions()` non-empty after bind | runtime harness: class=Mage, bind level-1 monster | ✅ PASS |
| 0.2 | Companion appears in dungeon | `getPartyMembers()[i].isCompanion()` true; name/race/level present | harness: read companion slot | ✅ PASS |
| 0.3 | Companion fights | temporary HP drops on attack | harness: `useCombatAction` vs a companion | ✅ PASS |
| 0.4 | Companion sells | Confinement sell returns gold, slot cleared | harness: gold before/after `sellCompanion` | ✅ PASS |
| 0.5 | Bad companion rejected | no crash / no cost when bind data is missing | harness: empty slot, expect `QMessageBox`, no crash | ✅ PASS |
| 1.1 | Sell returns gold | party gold increases | harness: gold before/after | ✅ PASS |
| 1.2 | Buy shows data | buy list non-empty | harness: `buyCompanion()` shows >0 items | ✅ PASS |
| 1.3 | Bind costs gold | handler records a cost, does not crash | harness: gold before/after | ✅ PASS |
| 2.1 | Identification has a fee | `Item::identifyPrice` > 0, fee charged | harness: identify an item, gold -fee | ✅ PASS |
| 2.2 | Gilden contracts pay gold | completing a contract increases party gold | harness: finish a gilden quest | ✅ PASS |
| 2.3 | Chest/container ops | store, retrieve, sell from chest | harness: store → retrieve → sell | ✅ PASS |
| 2.4 | Bad/legacy save survives | old save (no identification) still loads | load a v2 save, no crash | ✅ PASS |

**Run order:** 0.1→0.5 (companions), then 1.1→1.3 (confinement), then 2.1→2.4
(economy). Gates: slice N passes before slice N+1 starts. A failed check is
recorded with its failure reason and retried only after the fix — never hidden.

---

## Key findings (what the analysis showed)

1. `struct Companion` in `src/character/character.h` carries mangled field names
   (`M548B`, `M4ED4`, …) — reverse-engineered residue, not clean source. Treat it
   as a data carrier, not a spec.
2. `ConfinementAndHoldingDialog` already has the layout (bind/identify/
   realign/sell/buy groups, search box); only the handlers are stubs — this is
   the cheapest path to a live companion feature.
3. Spell schools are fully mapped (18, per `src/core/ReleaseInfo.cpp` 6.9.0 entry)
   — skip "spell schools" as a v6.15.0 slice.
4. Internationalisation: app text in Swedish, user expects English — all game UI
   copy stays in Swedish; only this plan document is now in English.

## Risks

- Companion combat AI is out of LVC in week 1 (may need a second week of
  refinement). If a single slice exceeds 4 h, downscope to the data layer only.
- Confinement UI text needs Swedish localisation (`tr()` / `.pro` ts); fall back to
  plain `QString` matching rather than blocking on it.
