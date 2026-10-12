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

### 6.0b — Fix XP curve
**Why**: Leveling is too slow at high levels.
**Do**: Change formula to `round(80 * n^1.3)` or similar.
**Verify**: Test proves level 20 is reachable in reasonable play time.

### 6.0c — Wire Mind spells into combat
**Why**: Mind Blast, Confusion, Fear, Insanity do nothing.
**Do**: Add Mind school to CombatActions::castSpell.
**Verify**: Test proves Mind spells apply effects.

### 6.0d — Add Electrical spells
**Why**: Only 2 spells vs 4 for Fire/Cold/Mind.
**Do**: Add Thunderclap (L4) and Storm (L8).
**Verify**: Test proves 4+ electrical spells exist.

### 6.0e — Fix Instant Kill spells
**Why**: No counterplay.
**Do**: Change to "damage = 3x caster level" instead of instant kill.
**Verify**: Test proves spells deal damage, not instant kill.

### 6.0f — Fix monster XP
**Why**: Values are 1000x too high.
**Do**: Scale XP to match level curve.
**Verify**: Test proves monster XP is reasonable.

### 6.1 — Release
**Why**: Final verification.
**Do**: Clean build, run tests, tag v6.0.0.
**Verify**: All tests pass.
