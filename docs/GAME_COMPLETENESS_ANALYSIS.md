# BlackLands — Game Completeness Analysis

An analysis of what the game currently has and what it needs to feel like a complete, shippable party-based dungeon crawler.

---

## What Already Exists

The foundation is solid. The project has:

- **Character creation** — 6 races, 6 stats, alignments, sex, guild eligibility
- **Party system** — 4-member party with shared gold
- **Dungeon crawling** — Tile-based movement, 3D wireframe rendering, compass, minimap
- **Spells** — 100 spells across 17 categories with mana costs, level requirements, damage ranges
- **Guilds** — 12 guilds with guild masters, leveling, re-acquainting, guild logs
- **Items** — 366 items (MDATA3) with attack/defense, stat requirements, stat modifiers, rarity, prices
- **Monsters** — 401 monsters
- **Town facilities** — General Store, Bank, Guilds, Seer, Confinement, Morgue, Library, Hall of Records
- **Dungeon features** — Pits, water, teleporters, chutes, anti-magic, fog, rotators, traps, hidden doors, stairs
- **Event system** — Basic trigger/effect event manager
- **Story/lore** — Intro cinematic with 6-part animated backstory
- **Audio** — 16 soundtracks, WAV manager
- **Persistence** — Character save/load, autosave
- **Basic multiplayer** — Lua-based TCP server/client with chat

---

## What's Missing — Organized by Priority

### 1. Combat System (Critical — the core loop is broken)

The current combat is a bare-minimum prototype. It only damages the character at party index 0, has no real turn structure, and doesn't use equipment stats.

**Needed:**
- **Turn-based combat with initiative** — Roll initiative for each party member and each enemy group. Cycle through turns in order. This is the beating heart of the genre. ( IMPLEMENTED )
- **Party-wide combat** — All 4 party members should fight, take damage, and die independently. Front-row vs. back-row positioning would add tactical depth (warriors up front, mages in back). ( ONGOING, TESTING )
- **Equipment-driven damage** — Weapon `att`, `swings`, `damageMod`, and `levelScale` from MDATA3 are already defined but never used in combat. Hook them up. Defense (`def`) should reduce incoming damage. ( IMPLEMENTED, TESTING)
- **Spell combat integration** — `SpellCastingDialog` returns `SpellResult` with damage/healing, but this isn't wired into the combat loop. Spells should be a combat action alongside Attack and Defend.
- **Flee/Run** — Allow the party to attempt to flee. Base success on party DEX vs. monster speed. Failed flee = free monster attack.
- **Critical hits and misses** — Small chance for double damage or complete whiff based on stats and level.
- **Status effects in combat** — Poison dealing DoT per turn, Blind reducing hit chance, Confusion causing friendly fire. The status flags exist in `GameConstants` but aren't applied.
- **Monster AI** — Monsters should choose between attacking, casting spells, or fleeing based on HP thresholds. Some monsters should target the weakest party member or the healer.
- **Group encounters** — Multiple enemy types in one fight (e.g., 2 Orcs and 1 Orc Shaman). The data supports this but encounters are currently single-monster.
- **Loot drops** — Defeating monsters should drop items based on the monster's level and the dungeon floor. The item rarity system in MDATA3 (`rarity`, `floor`) is already there for this.
- **Experience from combat** — Killing monsters should award XP distributed across the party. This feeds directly into leveling.

### 2. Leveling and Progression (Critical — no sense of growth)

Characters have `level` and `experience` fields but there is no leveling system.

**Needed:**
- **XP thresholds per level** — Define how much XP is needed for each level (e.g., level 2 = 1000 XP, level 3 = 3000 XP, etc.). Classic dungeon crawlers use exponential curves.
- **Level-up stat gains** — On level up: increase HP/MaxHP, increase MaxMana (for casters), allow 1-3 stat point allocations, possibly unlock new spells.
- **Guild-based progression** — Leveling should happen through guilds (the "Make Level" button exists in `GuildsDialog`). Different guilds should grant different HP/mana/stat bonuses per level.
- **Multi-guild progression** — Allow characters to switch guilds and level in multiple guilds. A character's total power is the sum of all guild levels. This is a classic Mordor mechanic and key to long-term depth.
- **Spell learning** — As characters level up in magic guilds, they should learn new spells from `spells.json` based on `base_level` and `guilds` fields.
- **Age and death from old age** — `incrementPartyAge()` and `processAgingConsequences()` are declared but need real consequences: stat decay, and eventual death when age exceeds race max.

### 3. Equipment System (Critical — 366 items with no way to use them)

MDATA3 has a full item system with equipment types, stat requirements, and modifiers, but there's no equip/unequip mechanic.

**Needed:**
- **Equipment slots** — The `type` field in MDATA3 maps to slot types (0=Hands, 1=Dagger, 3=Sword, 4=Staff, 5=Mace, 6=Axe, 7=Hammer, 8=Leather, 9=Chain, 10=Plate, 11=Shield, 13=Helm, 14=Gloves, 15=Gauntlets, 16=Cloak, 17=Bracers, 18=Sash, 19=Girdle, 20=Boots, 21=Ring, 23=Potion, 24=Scroll, 26=Dust). Map these to body slots: Main Hand, Off Hand, Head, Body, Hands, Feet, Cloak, Waist, Wrist, Ring.
- **Stat requirement enforcement** — Items have `StrReq`, `IntReq`, `WisReq`, `ConReq`, `ChaReq`, `DexReq`. Block equipping if the character doesn't meet them.
- **Stat modifiers on equip** — Items have `StrMod`, `IntMod`, `WisMod`, `ConMod`, `ChaMod`, `DexMod`. Apply these when equipped, remove when unequipped.
- **Cursed items** — Items with `cursed=1` should be unremovable until uncursed at the General Store (the uncurse function exists in `GeneralStore`).
- **Consumables** — Potions, scrolls, and dust should be usable (cast their spell via `spellIndex`/`spellID`, then consume a `charge`).
- **Item identification** — New items found in the dungeon should be unidentified. Show them as "Unknown Sword" until identified at the General Store or via spell. The ID cost mechanic already exists in the store UI.
- **Guild restrictions** — The `guilds` bitmask in MDATA3 restricts which guilds can use an item. Enforce this.

### 4. Dungeon Persistence and Depth (High — exploration is hollow without it)

The dungeon is generated fresh each visit with no persistence.

**Needed:**
- **Persistent level maps** — Save the generated layout, monster positions, and treasure positions per level. When the player returns to a level, restore the saved state instead of regenerating.
- **15 dungeon levels** — The lore mentions deep depths and a "Prince of Devils" at the bottom. Define at least 15 levels with increasing difficulty, different tile distributions, and distinct themes (mines → caverns → crypts → hell).
- **Boss monsters** — Place unique, scripted boss encounters on key floors (e.g., floor 5, 10, 15). The "Prince of Devils" should be the final boss.
- **Locked doors and keys** — `DoorEnums.h` exists but there's no key-and-lock mechanic. Certain doors should require keys found elsewhere on the level or on earlier levels.
- **Secret door discovery** — Hidden doors are placed but `on_searchButton_clicked()` needs a proper mechanic: roll WIS/INT against a difficulty, reveal hidden door on success.
- **Monster respawning** — After the party leaves a level and returns, some monsters should respawn to keep grinding viable but not trivially safe.
- **Environmental hazards scaling with depth** — Deeper levels should have more pits, anti-magic zones, and deadlier traps. Trap damage should scale with floor.

### 5. Death and Consequences (High — death must matter)

When HP reaches 0, the dialog just closes. There's no real death system.

**Needed:**
- **Character death state** — When a party member dies, mark them dead (`isAlive = false`). They stay in the party as a body that must be carried back.
- **Party wipe** — If all party members die, the party is left in the dungeon. The player must form a rescue party from town to recover bodies and items.
- **Morgue integration** — The Morgue dialog exists with resurrection, body hiring, and body grabbing. Wire this into the death flow: dead characters appear in the Morgue, resurrection costs gold scaled to level.
- **Permadeath option** — For hardcore players, offer a mode where death is permanent (body can only be looted, not resurrected).
- **Body recovery in dungeon** — Dead party members' bodies should remain at their death location. Other parties can find and carry them back.

### 6. Town and City Life (Medium — the city is a menu, not a place)

The city of Marlith is just a set of dialog buttons. It needs to feel like a place.

**Needed:**
- **Tavern/Inn** — A place to rest the party, restore HP/mana, and cure status effects for gold. Resting should also advance time and age characters.
- **Quest board or quest givers** — Even simple fetch quests ("retrieve the Adamantite Shield from level 7") or kill quests ("slay the Goblin King on level 3") would give direction to dungeon runs.
- **NPC dialog** — Guild masters, the Seer, shopkeepers should have personality and context-sensitive dialog. Hint at dungeon secrets, warn about upcoming dangers.
- **Reputation or alignment consequences** — Evil characters should be turned away from the Paladin's Guild. Good characters should be refused by the Villains Guild. The alignment data is in `gamedata.json` but not enforced beyond character creation.
- **Economy tuning** — Item prices in MDATA3 range from 75 gold (Copper Helm) to 303 million (Eliminator). Ensure gold income from dungeon runs and monster kills creates a meaningful progression curve where players can afford gear upgrades at appropriate levels.

### 7. Win Condition and Endgame (Medium — no reason to keep playing)

There is no victory state.

**Needed:**
- **Main quest line** — The lore establishes a "Prince of Devils" in the deepest depths. Killing this boss should trigger a victory sequence.
- **Victory screen/cinematic** — A counterpart to the `StoryDialog` intro, showing the aftermath of the prince's defeat.
- **Hall of Records** — `HallOfRecordsDialog` is declared but empty. Implement it to track: fastest completion, highest level character, most gold earned, most monsters killed, deepest floor reached.
- **New Game Plus or post-game** — After winning, allow continued play with harder monsters or prestige rewards.

### 8. Quality of Life (Medium — smooths out the experience)

- **Character sheet improvements** — Show equipped items, effective stats (base + item modifiers), current guild levels, known spells.
- **Bestiary** — The Library exists but should auto-populate with monster entries as the player encounters them, including weaknesses, resistances, and drop tables.
- **Quest/Journal log** — Track active quests, dungeon notes, and important events.
- **Tutorial/Help** — The help lesson dialog exists. Flesh it out with a guided first dungeon run explaining controls and mechanics.
- **Keyboard shortcuts** — WASD/arrow movement works, but add shortcut keys for common actions (F=Fight, S=Spell, R=Rest, etc.).
- **Sound effects** — The WAV manager exists. Add sound effects for combat hits, spell casting, door opening, trap triggering, level up, death.
- **Visual feedback** — Flash the screen red on damage, gold glow on level up, particle effects on spells.

### 9. Balance and Tuning (Low priority now, critical before release)

- **Monster difficulty curve** — Ensure monsters on floor 1 are beatable by level 1 characters and floor 15 requires high-level, well-equipped parties.
- **Spell balance** — Fire Bolt and Cold Bolt are identical (5-10 damage, 10 mana, level 1). Differentiate the elemental schools with unique mechanics (fire = AoE, cold = slow, lightning = chain, mind = crowd control).
- **Item progression** — Bronze → Iron → Steel → Adamantite → Mithril is a clear tier. Ensure the floor/rarity values create a smooth loot curve where upgrades feel earned.
- **Gold sink balance** — Resurrection, identification, uncursing, buying items, and guild leveling should all cost enough to prevent gold from becoming trivial.

### 10. Technical Debt (Low priority for gameplay, high for stability)

The FIXLIST.md already covers 21 bugs. The most gameplay-impacting ones are:
- **Dead characters always saved as alive** (FIXLIST #2) — Breaks the entire death system
- **`hasLivingCharacters()` rejects living characters with status effects** (FIXLIST #3) — Can softlock the game
- **Font mismatch** (FIXLIST #5) — Missing characters in UI text
- **RCE via Lua socket** (FIXLIST #1) — Security hole in multiplayer

---

## Suggested Implementation Order

1. **Equipment system** — Equip/unequip, stat requirements, modifiers (unlocks item meaning)
2. **Combat overhaul** — Party-based turns, equipment-driven damage, spell integration (unlocks core loop)
3. **Leveling system** — XP thresholds, stat gains, spell learning (unlocks progression)
4. **Death system** — Character death, party wipe, Morgue integration (unlocks stakes)
5. **Dungeon persistence** — Save/load level state, 15 floors, bosses (unlocks exploration)
6. **Win condition** — Main quest, final boss, victory screen (unlocks completion)
7. **Town improvements** — Tavern, quests, NPC dialog (unlocks context)
8. **Balance pass** — Difficulty curve, economy, spell differentiation (unlocks polish)
9. **QoL and polish** — Bestiary, journal, sound effects, tutorials (unlocks accessibility)
10. **Bug fixes from FIXLIST** — Fix the critical bugs that affect the above systems

---

*Generated: 2026-03-12*
