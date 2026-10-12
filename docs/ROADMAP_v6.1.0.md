# v6.1.0 — Spell Wiring & Loot Integrity

## Status: DONE (released 2026-10-12, tagged v6.1.0)

Audit run after v6.0.0 (2026-10-12). v6.0.0 fixed *balance numbers*; this
release fixes *plumbing* — effects that are described in data but never
reach the game world.

### Audit findings

**1. Two parallel spell paths disagree — dialog still one-shots**
- `CombatActions::applySpellDamageBySchool` (the good path, fixed in 6.0e)
  deals `qMax(50, casterLevel*3)` for "Instant Kill".
- `SpellCastingDialog::castDamageSpell` (the path DungeonDialog actually uses
  for the player's Cast Spell button) **still returns 9999** and rolls a flat
  `30 + intelligence`% chance.
- Same class of bug as 6.0e, in a second code path. The player-facing button
  never goes through the fixed path.
- **Fix**: Make `SpellCastingDialog` delegate damage/effect resolution to
  `CombatActions` instead of duplicating the logic. One source of truth.

**2. Kill / Banish schools are routed to a no-op**
- `SpellCastingDialog::onCastClicked` routes only Heal / (Fire|Cold|Electrical|
  Mind|Magical|Damage) / (Element|Resistant|Protection).
- `Kill` (Exorcism, Vampire Slayer) and `Banish` (Banish Demon/Undead) fall to
  `castUtilitySpell`, whose if-chain has no branch for them → generic
  "You cast X." with `damageDealt = 0`. The spells cost mana and do nothing.
- **Fix**: Route Kill/Banish through the school-aware combat path.

**3. Damage-over-Time school never applies its DoT**
- `Damage` school = Poison, Wound, Drain Life. Their `damage` field is the
  string `"Damage over Time"`.
- `calculateSpellDamage` splits on `-`, fails to parse, returns 0 → "no effect".
- `CombatState` already has `poisonDuration` and `tickStatusEffects()` applies
  1-3 poison damage/round — the mechanism exists, the spells just never set it.
- **Fix**: Give Poison/Wound a real DoT via the existing status system; Drain
  Life should deal instant damage and heal the caster.

**4. No monster drops loot — drop tables are empty**
- `VictoryReward::getDropTable` reads `Item0`..`Item9` from the monster row.
- `data/bestiary.json` (the live monster table, 401 rows) has **no Item0-9
  fields at all** → `calculateLoot` always returns an empty list.
- `DungeonDialog::awardBattleLoot()` (the only code that would drop a random
  item) has **no callers** — dead code.
- **Result**: killing monsters yields XP + gold, never items.
- **Fix**: Either add drop tables to bestiary.json, or derive loot from the
  monster's category/level via ItemProgression tiers. Prefer the latter — it
  needs no 401-row hand-edit and stays consistent with tier gating.

**5. Dead `awardBattleLoot()` random-item path**
- Picks a uniformly random item from the whole database regardless of monster
  or floor — would be a balance bug if it were live. It is not live.
- **Fix**: Remove it (superseded by the drop-table path).

### Slices

### 6.1a — Unify spell resolution (kill the 9999 duplicate) ✅ DONE
**Why**: Player Cast Spell button still one-shots; two sources of truth.
**Do**: Added `SpellMechanics::instantKillDamage(level)` as the single source of truth (`max(50, level*3)`); both `CombatActions` and `SpellCastingDialog` call it. Removed the 9999 literal and the flat 30+int% chance roll.
**Verify**: Non-vacuous: `instantKillDamage` is asserted directly; dialog damage is never 9999.

### 6.1b — Wire Kill and Banish schools into combat ✅ DONE
**Why**: Exorcism / Vampire Slayer / Banish Demon / Banish Undead did nothing.
**Do**: Added Kill and Banish to the school-routed categories in `onCastClicked`, so they reach `castDamageSpell` and the school-aware combat path.
**Verify**: Non-vacuous: Banish Undead deals massive damage instead of a no-op.

### 6.1c — Implement Damage-over-Time spells ✅ DONE
**Why**: Poison / Wound / Drain Life parsed to 0 damage.
**Do**: Added a `School::Damage` branch in `applySpellDamageBySchool`. Poison/Wound set the existing poison DoT (`tickStatusEffects` deals 1-3/round); Drain Life deals instant damage and heals the caster. Also fixed `schoolFor` to map "Damage" (it previously fell through to Magical, so the branch was unreachable).
**Verify**: Non-vacuous: disabling the branch leaves poison duration 0 and no drain heal.

### 6.1d — Monster loot from category + level ✅ DONE
**Why**: bestiary.json has no drop tables → no loot ever drops.
**Do**: Added `VictoryReward::deriveDropTable`, which picks a tier from the monster's level (clamped to dungeon depth) and returns floor-appropriate items when the row has no Item0-9. `calculateLoot` falls back to it.
**Verify**: Non-vacuous: disabling the fallback yields 0 drops in 200 kills.

### 6.1e — Remove dead awardBattleLoot() ✅ DONE
**Why**: Uniformly-random item from the whole DB, and never called.
**Do**: Deleted the method and its declaration.
**Verify**: Build succeeds; no references remain.

### 6.2 — Release
**Why**: Final verification.
**Do**: Clean build, run tests, bump to 6.1.0, tag v6.1.0.
**Verify**: All tests pass.
