# v6.2.0 — Dungeon floor fidelity & theme application

## Status: SUPERSEDED by v6.3.0

Audit run after v6.1.0. v6.0.0–v6.1.0 fixed *numeric balance* and *spell
wiring*; this release fixes the one class of problems that keeps looping back:
**the floor-generation and encounter pipeline never reads its own design data.**

The `FloorTheme` struct (and the `DungeonThemes` table) is fully populated with
tile-probability percentages and monster-level bonuses, but every consumer is
hardcoded to ignore them. The result: every floor looks like floor 1 —
identical tile mix, identical hazard set, flat monster difficulty — and the
theme's design intent (a "cathedral" floor with heavy pits and slow fog, a
"shrine" floor with sparse hazards and level-progressive monsters) is
decorative fiction.

### Audit findings

**1. Tile probabilities in `FloorTheme` are never applied**
- `DungeonThemes::forLevel()` fills `waterChance`, `pitChance`, `fogChance`,
  `antimagicChance` (0–20 range) but `DungeonDialog::generateSpecialTiles` is
  hard-coded to the same roll buckets for every floor: ~15% monsters,
  ~15% treasure, ~15% water (ceil 300–415), ~10% anti-magic, ~5% pits,
  ~10% traps. Every floor is statistically identical.
- **Fix**: consume `theme->waterChance`, `theme->pitChance`, `theme->fogChance`,
  `theme->antimagicChance` as the exact per-floor percentages. Validate the
  four values sum to <= 100 and treat the remainder as empty floor space.

**2. `monsterLevelBonus` is never applied**
- `FloorTheme.monsterLevelBonus` is populated (e.g. 15 + level - 1 for deep
  floors, 0 at floor 1) but `EncounterBuilder` reads `levelFound` straight out
  and never adds it. A "level 10, +3" floor places monsters that are still
  level 10, so the intended progression never shows up in combat, stats or
  bestiary.
- **Fix**: add `monsterLevelBonus` to the base level used for encounter HP,
  attractions, and treasure expectation.

**3. Four hazard tile sets exist but are never generated**
- `m_fogPositions`, `m_rotatorPositions`, `m_extinguisherPositions` and
  `m_studPositions` are declared and have rendering, and they are cleared each
  generation, but no code path inserts into them — only the mineable ones
  (water, anti-magic, pit, traps, chutes, teleporters) ever appear.
- Minimap + 3D renderer draw them (and `drawSpinner`, `drawStud`,
  `drawExtinguisher`), and `DungeonHandlers` declares
  `handleSpinnler`/`handleStud`/`handleExtinguisher` stubs exist. So the tiles
  were *designed* to be in the world; generation just skipped them.
- **Fix**: generate each missing tile type (with a rolling hit chance from the
  theme), and wire each to a `DungeonHandlers` handler that applies a real
  effect on player stepping. The extinguisher already has a handler — it just
  never gets placed.

**4. Dead method declarations remain**
- `DungeonDialog::handleFalling()` (a "New method" comment in the header, never
  defined or called),
- `DungeonDialog::tryUseStairs(bool)` (declared, never defined),
- `GuildsDialog::updateReacquaintButtonState()` (declared, never defined;
  the button state here is actually managed inline at lines 346/352).
- **Fix**: remove these declarations so the header matches the implementation
  and the compiler can warn on the real remaining surface.

**5. `m_hiddenDoorPositions` is a map, not a tile set**
- It stores `DoorState { position, secret, locked, difficulty }` and renders
  fine, but there is no player-facing way to ever step on or trigger it: the
  only consumers are the minimap renderer and a "collides with the wall unless
  discovered" check. A secret door behind a wall is invisible with no
  interaction, which makes it feel like a bug rather than a feature.
- **Fix** (optional, lower priority): add a `Search`/`Open` interaction in
  `DoorAndSearch` so the hidden-door map actually changes play, or remove the
  door render if secrets are deliberate. This slice is only done if the
  gameplay intent lands in the place we want.

### Verification

```
1793 passed, 0 failed   (main suite, up from 1783)
3 passed, 0 failed     (equipment suite)
```

## Slices

### 6.2a — Wire `FloorTheme` tile chances into `generateSpecialTiles`
**Why**: every floor is statistically identical; the theme's chance design
(a cathedral with heavy pits/anti-magic, a shrine with sparse hazards) is
never applied.
**Do**: consume `theme->waterChance`, `theme->pitChance`, `theme->fogChance`
and `theme->antimagicChance` as the exact roll buckets. If the four sum to <=
100, the remainder is empty floor space; if they overrun, cap each and drop
the lowest. Keep the existing monster/treasure/trap buckets (their absolute
odds are the theme's `monsterCount`/`trapCount`).
**Verify**: a floor with 20% water/10% anti-magic/10% pits renders those three
hazard ratios across 200 tiles; a flat theme renders the same flat ratio for
every floor.

### 6.2b — Apply `monsterLevelBonus` in the encounter builder
**Why**: `FloorTheme.monsterLevelBonus` is populated (deep floors +bonus) but
`EncounterBuilder` uses `levelFound` straight off, so thematic difficulty
never reaches combat.
**Do**: pass `monsterLevelBonus` through `EncounterBuilder`, add it to the
base level before computing HP, attractions, and treasure expectation.
**Verify**: a level-10 monster with +3 bonus has a level-13 `level` field and
scaled stats (the test asserts `level == 13`, `hp >= baseHp*mult`, etc.);
removing the increment fails the test.

### 6.2c — Generate fog tiles + handler
**Why**: `m_fogPositions` is cleared every generation but never filled, so fog
never appears even though a `drawFog`/minimap path and a `handleFog`
declaration exist.
**Do**: add fog to `generateSpecialTiles` as a new roll bucket (use the theme's
`fogChance` so it scales with depth), wire a `DungeonHandlers::handleFog` that
reduces the player's effective vision radius for a few turns, and add the
foggier to the map renderer.
**Verify**: after generation, `dialog->m_fogPositions` is non-empty on every
floor where `theme.fogChance > 0`; stepping on a fog tile applies the vision
reduction (asserted in the combat/sight test).

### 6.2d — Generate spinner (rotator) tiles + handler
**Why**: `m_rotatorPositions` and `drawSpinner` are present, but the tile is
never generated, so the player has never seen the world rotate around them.
**Do**: generate spinners by rolling `theme->...` (or a dedicated
`rotatorChance` slot), then wire a handler that, when stepped on, rotates the
party's facing direction (left/right) through the existing `DungeonDialog::rotate`
and logs the new heading.
**Verify**: a generated spinner tile exists in the world, and the test asserts
that a step on it changes the party's facing direction (the rotation helper is
unit-callable from the test).

### 6.2e — Generate extinguisher tiles + handler
**Why**: `m_extinguisherPositions` is cleared but never filled, even though the
`handleExtinguisher` handler that quenches fire is already written and tested.
**Do**: generate extinguishers by rolling the extinguisher tile (this is already
planned as a fixed bucket, so reuse it), and confirm stepping on it toggles the
party's on-fire state.
**Verify**: after generation `m_extinguisherPositions` is non-empty on deep
floors; the test asserts a step on an extinguisher clears `isCharacterOnFire`.

### 6.2f — Remove dead method declarations
**Why**: `handleFalling`, `tryUseStairs(bool)`, and
`updateReacquaintButtonState()` are declared in headers but never defined or
called. Keeping them in headers tells users "this exists" and blocks the
compiler from warning on the real remaining DungeonDialog surface.
**Do**: delete the three declarations. `handleFalling`/`tryUseStairs` were
superseded by `DungeonHandlers::handlePit` / the `Qt::Key_T` inline logic;
`updateReacquaintButtonState` was replaced by inline setters at lines 346/352.
**Verify**: `grep` shows no `handleFalling|tryUseStairs|updateReacquaintButtonState`
declarations remain; the build compiles clean; destructor/ctor still link.

### 6.2g — Inspect `m_hiddenDoorPositions` (decision point)
**Why**: the door map is populated and rendered but invisible to the player —
there is no interaction to find or open a secret door, so it is indistinguishable
from a cosmetic leftover.
**Do**: first verify intent. If the player-side secret-door effect is desired,
add a `Search` interaction in `DoorAndSearch` (searching a wall tile checks
`m_hiddenDoorPositions` and, if not locked, marks the door found and removes it
from the wall). If it is decorative-only, remove the render call at
`DungeonDialog.cpp:2168` and the player-facing `m_hiddenDoorPositions` checks
so the feature is not advertised and then unusable.
**Verify**: either (a) an independent search step on a hidden door marks it
found and returns the room behind it, or (b) the door renderer and player checks
are removed cleanly with no dangling references.

### 6.3 — Release
**Why**: end-of-version verification.
**Do**: clean build, run the full test suite, update the link chart, and tag
v6.2.0.
**Verify**: clean build passes; all tests pass.
