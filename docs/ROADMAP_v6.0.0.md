# v6.0.0 — Spell & Balance Fix

## Status: IN PROGRESS

### Audit findings

**1. Mana system is broken — spells are unusable**
- Character starts with `calculateMana() = 20 + level*2` = 22 MP at level 1
- Flame Bolt costs 10 MP → only 2 casts
- Fireball costs 25 MP → impossible to cast
- Inferno costs 75 MP → never castable
- **Fix**: Scale mana cost down or mana pool up. Target: 3–5 casts of base spell at level 1.

**2. XP formula is extreme — leveling feels wrong**
- Formula: `xpForLevel(n) = round(100 * n^1.5)`
- Level 2→3 requires 283 XP, level 19→20 requires 8708 XP
- Total XP to reach level 20: ~31,200
- **Fix**: Use a gentler curve. Target: level 20 reachable in ~8–12 hours of play.

**3. Mind spells not wired into combat**
- CombatActions handles Fire, Cold, Lightning — but NOT Mind
- Mind Blast (L1 combat) does nothing in combat
- Confusion, Fear, Insanity are unreachable in combat
- **Fix**: Add Mind school handling to CombatActions::castSpell

**4. Electrical school has only 2 spells**
- Fire: 4, Cold: 4, Mind: 4, Damage: 3, Element: 3
- Electrical: 2 (Lightning L2, Chain Lightning L6)
- **Fix**: Add 2 more electrical spells for parity

**5. Instant Kill spells have no counterplay**
- Insanity (L8), Exorcism (L6), Vampire Slayer (L8) — all "Instant Kill"
- If they work as intended, they're overpowered
- **Fix**: Add resistance check or cap at "massive damage" not instant kill

**6. Monster XP values are absurd**
- Gnarled Hands: 255,671 XP (level 1 monster!)
- Master's Hand: 23,433,232 XP
- Level 1→2 requires only 100 XP
- **Fix**: Scale monster XP to match level curve

### Slices

### 6.0a — Fix mana system ✅ DONE
**Why**: Spells are unusable (cost > pool).
**Do**: Increase maxMana growth from +2 to +5 per level.
**Verify**: Non-vacuous: +2 fails test, +5 passes.

### 6.0b — Fix XP curve ✅ DONE
**Why**: Leveling is too slow at high levels (31,200 total XP for level 20).
**Do**: Change formula to `round(30 * n^1.2)` (10,480 total XP — 66% reduction). Update levels.json and all test assertions.
**Verify**: Non-vacuous: removing round() fails test (206 ≠ 207).

### 6.0c — Wire Mind spells into combat ✅ DONE (N/A)
**Why**: Audit claimed Mind spells were not wired in combat.
**Do**: N/A — Mind is already handled in both `castSpell` (rad 384-394) and `applySpellDamageBySchool`. Mind Blast (L1, damage 5-10) applies damage + stun/confuse. Confusion/Fear/Insanity are control-only and go through `castSpellBySchool`.
**Verify**: N/A

### 6.0d — Add Electrical spells ✅ DONE
**Why**: Only 2 spells vs 4 for Fire/Cold/Mind.
**Do**: Added Thunderclap (L4, 1 chain) and Storm (L8, 3 chains). Updated SpellMechanics chain logic. Test [27] updated to 49 spells.
**Verify**: Non-vacuous: Thunderclap/Storm return correct chain counts.

### 6.0e — Fix Instant Kill spells ✅ DONE
**Why**: No counterplay — 9999 one-shot damage.
**Do**: Changed to `qMax(50, casterLevel * 3)` massive damage. Insanity/Exorcism/Vampire Slayer/Banish now deal 50+ damage instead of instant kill.
**Verify**: Non-vacuous: removing the handler returns 0 damage.

### 6.0f — Fix monster XP
**Why**: Values are 1000x too high.
**Do**: Scale XP to match level curve.
**Verify**: Test proves monster XP is reasonable.

### 6.1 — Release
**Why**: Final verification.
**Do**: Clean build, run tests, tag v6.0.0.
**Verify**: All tests pass.
