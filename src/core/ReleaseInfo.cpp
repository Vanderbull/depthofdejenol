#include "ReleaseInfo.h"
#include "version.h"
#include <QPair>

QString ReleaseInfo::version() {
    return QString::fromLatin1(GameConstants::SEMANTIC_VERSION);
}

QString ReleaseInfo::versionString() {
    return "v" + version();
}

QString ReleaseInfo::releaseDate() {
    return "2026-10-22";
}

QStringList ReleaseInfo::releaseNotes() {
    return {
        "Phase 1: Core systems — items, equipment, combat, spells, guilds, save/load",
        "Phase 2: Combat — equipment-driven attacks, monster AI, group encounters, status effects",
        "Phase 3: Progression — XP table, level-up gains, guilds, spell learning, aging",
        "Phase 4: Dungeon depth — 15 floors, 15 themes, bosses, keys, secret doors, respawn",
        "Phase 5: Death and consequences — body carrying, resurrection, party wipe, rescue",
        "Phase 6: Win condition — quest chain, final boss, victory, Hall of Records, New Game Plus",
        "Phase 7: Town and quality of life — tavern, quest board, NPCs, alignment, character sheet, bestiary, journal, sound, shortcuts, tutorial",
        "Phase 8: Balance and release — difficulty curve, spell differentiation, item progression, gold sinks, packaging"
    };
}

QStringList ReleaseInfo::changes() {
    return {
        "F-key information panels restored: F1 Help, F2 Area Look, F4 Resistance, F8 Buffer, plus F3/F6 (character sheet), F7 (guilds) and F10 (options); wired in both DungeonDialog and theCity",
        "GameToolbar added with command/info page switching (F11) in both town and dungeon, connected to the same slots as the menu buttons",
        "New Game Plus implemented: keeps the party, resets the dungeon, quest chain and play time, and raises the NG+ level — the \"Coming soon.\" stub is gone",
        "Combat spellcasting: CombatController::castSpell() now opens SpellCastingDialog and logs the result instead of being a no-op stub",
        "Multiplayer hardening: resync tracking per zone-sync source, chat control-character sanitisation, and protocol-version enforcement",
        "Damage resistances from the bestiary now apply in combat through CombatActions::applyHit, so a monster's listed resistances actually reduce damage",
        "Test hygiene: 8 vacuous check(true) tests replaced with observable assertions and race-checked quicksave/quickload slot (6.13)",
        "Race: F5/F9 quicksave/quickload now honour the current slot; removed the F5/F9 read of the never-written CurrentSaveName game value (6.13)",
        "Stylesheets are located at runtime, so an installed build keeps its theme",
        "ResourcePaths::locate() searches the application dir, its parents and the working directory instead of a build-time __FILE__ path",
        "Dead commented-out code removed from gameStateManager.cpp; no logic changed",
        "Multiplayer lobby: host/port/username, connect and disconnect against a real server",
        "Multiplayer chat with a notification sound, plus a live player list",
        "Zone-sync: other players' actions are received and shown in the lobby",
        "Host a server from the lobby and send move/fight commands as a client",
        "Dungeon floors carry names in levels.json, and a level-up writes a journal entry",
        "Arena removed from the game — combat is now exclusively in the dungeon",
        "Orphaned systems audit: NetworkManager and LoadingScreen confirmed wired",
        "Combat visual feedback confirmed: flashCombatFx, SpellCastingDialog, item selection, monster status in target list",
        "Documentation: 13 game data books with spoiler disclaimers",
        "Options dialog rebuilt around the settings that actually do something: audio, autosave, automap reveal-all, combat speed, fullscreen, fonts, backup/restore",
        "GameSettings is the single source of truth for player options; applyAll() runs at startup so a choice survives a restart",
        "Cancel restores a snapshot taken when the dialog opened instead of leaving changes half applied",
        "Backup/restore targets data/saves and data/characters into data/backup/<date>",
        "The game's font now comes from Options and nothing else: GameSettings owns the default, gameStateManager no longer hard-codes one, and every stylesheet font rule was removed",
        "fontManager only updates widgets that were inheriting the app font, so a picked font no longer clobbers a widget's own font",
        "Unified main-menu button sizes: Hall of Records, Options and Quit are now 55px like the action buttons",
        "Renamed the main-menu Exit button to Quit",
        "Removed the About dialog and its main-menu button; the footer is now Options and Exit",
        "Removed the update system (UpdateManager, UpdateDialog, standalone test drivers)",
        "Removed the dead Version.h left over from an older version-header scheme",
        "Removed the Help dialog (src/helplesson) and its main-menu button (908 lines deleted)",
        "Test hygiene: 11 vacuous/duplicate test sections and 5 dead blocks removed (1911 to 1891 checks)",
        "48 new spells from the MORDOR manual across 18 categories (95 total)",
        "SpellCategory registry: targeting, resistance, and combat flags per category",
        "SpellTargeting: single, group, area, and self-targeting logic",
        "SpellResistance: 20% base, +5% per level difference",
        "SpellScaling: +10% power per level above base",
        "Larger dungeons: 50x50 tiles (was 30x30), more exploration space",
        "BSP-based map generation for natural cavern layouts",
        "Corridor network with loops and alternate routes",
        "Thematic zones: cells, caverns, temple, lair",
        "Multi-level rooms with lower level (z=-1) and stairs",
        "Structural stair placement in room centers",
        "Progressive complexity: room count scales with floor depth",
        "Combat is tactical: pick targets, see each monster, use front/back rows",
        "Level-ups grant spendable stat points; the pool persists",
        "The main quest chain advances from play and grants its rewards",
        "Play time is tracked and the Hall of Records sorts by it",
        "Saves are versioned, migrated, and newer saves are refused",
        "Combat gives visual feedback on hits and deaths",
        "Core loop wired end to end: fight, loot, equip, level, descend",
        "Monster spellcasting, status effects (poison, blind, confusion, fire)",
        "Loot drops start unidentified; identify/uncurse at the General Store",
        "Bestiary unlocks a monster only once you have met it",
        "Journal records victories, level-ups, quests, deaths and identification",
        "Gold sinks charged for resurrection, identification, uncursing, guild leveling, rest and cures",
        "Aging causes stat decay and death by old age; the body reaches the Morgue"
    };
}

QString ReleaseInfo::systemRequirements() {
    return "Qt 6.0+, 2 GB RAM, 500 MB disk space";
}

QString ReleaseInfo::installerName() {
    return "blacklands-" + version() + "-linux-x64";
}

bool ReleaseInfo::isRelease() {
#ifdef QT_NO_DEBUG
    return true;
#else
    return false;
#endif
}

QString ReleaseInfo::banner() {
    return "Depth of Dejenol " + versionString();
}

QList<QPair<QString, QString>> ReleaseInfo::versionHistory() {
    return {
        {"6.14.0", "2026-10-22 — AutoMap, Tutorial och Spellklasser. AutoMap: fullstor karta (Ctrl+Shift+M) som renderas av samma drawMapTo som minimapen, med en vy-lokal \"Reveal Entire Map\" som lyfter dimman utan att ändra vad spelet kommer ihåg; MiniMapDialog borttagen. Tutorial: first-run-guide (Ctrl+T) i fyra steg. Spellklasser: 12 klasser (data/SpellClasses.json + SpellClassRegistry) kopplade till startgillet; klass och klassspells visas nu på karaktärskortet. Bygget kör make check: PASS"},
        {"6.13.0", "2026-10-22 — Återställ MORDOR:s gränssnitt: F1-F4/F6-F8/F10-F11 informationspaneler, GameToolbar med sidväxling (F11), New Game Plus implementerad (Coming soon borta), combat-spellcasting i strid, multiplayer-härdning (resync + chat-sanitering) och testhygien. Skaderesistanser från bestiaryn tillämpas nu i striden. Testhygienen hittade en cacherad quicksave/quickload-bug: F5/F9 läste en \"CurrentSaveName\"-nyckel som aldrig skrevs, så quicksave alltid målade den statiska filen \"quicksave\". Fix: handläggarna använder getCurrentSaveName() och load() sätter sloten. Bygget kör make check: PASS"},
        {"6.11.0", "2026-10-19 — Multiplayer blir spelbart: lobby-dialog med host/port/username, connect/disconnect, chatt med ljudnotis, spelarlista, zone-sync, starta server lokalt samt move/fight-kommandon. NetworkManager med reconnect, ping/pong och JSON-protokoll. Dessutom: floor-namn i levels.json, journal-post vid level-up, och fem refaktoreringsfaser (CombatController utbruten ur DungeonDialog, unique_ptr-minnesäkerhet, tema-variabler i stället för hårdkodade färger, tooltips på huvudmenyn, död kod borttagen). 2030 passed, 0 failed"},
        {"6.10.0", "2026-10-17 — Consumable-effekter: teleport, levitation, invisibility, true sight, bind. Sex effekter var 'not yet wired' och gjorde ingenting; nu är de implementerade med statusvariabler som tickas ner varje runda. Teleporterar party, låter party flyga över hinder, gör party osynlig för monsters, visar dolda dörrar på minimap, och halverar skada från bundna mål. 2058 passed, 0 failed"},
        {"6.9.0", "2026-10-17 — Konsumtionssystem: föremålslöseri från data, inte från namn. Character::useConsumable matchade på item-namn och var en no-op för allt utan rätt nyckelord; ny ConsumableResolver löser effekten via spellID mot MDATA2-spell-tabellen, alla 18 spell-schoolar mappade uttryckligen. Fixar två buggar datan avslöjade: heal-spells börjar på 0 så en potion kunde läka 0 HP, och charm-scrolls behandlades som skadescrolls. CombatActions::useItem delar nu samma resolver. 2043 passed, 0 failed"},
        {"6.8.0", "2026-10-16 — Documentation: 13 game data books (monsters, spells, items, races, guilds, progression, research, dungeon, town, quests, endgame, combat, lifecycle) with spoiler disclaimers, all linked from README"},
        {"6.7.0", "2026-10-15 — Balance pass: monster XP scaled 3x so a level takes roughly 10-20 fights instead of 3; the main menu and theCity buttons now follow the font chosen in Options; 1967 passed, 0 failed"},
        {"6.6.0", "2026-10-14 — Rebuilt the Options dialog around settings that actually work, and made the game's font come from Options and nothing else (GameSettings owns the default, no hard-coded font, no stylesheet font rules); 1954 passed, 0 failed"},
        {"6.5.4", "2026-10-14 — Unified main-menu button sizes (Hall of Records/Options/Quit now 55px like the action buttons); renamed Exit to Quit; 1887→1891 checks, 0 failed"},
        {"6.5.3", "2026-10-14 — Removed the About dialog and the whole update system (UpdateManager/UpdateDialog + test drivers); dead Version.h; 1889→1887 checks, 0 failed"},
        {"6.5.2", "2026-10-14 — Removed the Help dialog and its main-menu button (908 lines); 1891→1889 checks, 0 failed"},
        {"6.5.1", "2026-10-14 — Test hygiene: removed 11 vacuous/duplicate test sections and 5 dead blocks; 1911→1891 checks, 0 failed"},
        {"6.5.0", "2026-10-14 — Spell System Expansion: 48 new spells from MORDOR manual (95 total), SpellCategory registry (18 categories), SpellTargeting, SpellResistance, SpellScaling, 74 non-vacuous tests"},
        {"6.4.0", "2026-10-13 — Kartgenerering & nivåsystem: större karta (50x50), BSP-algoritm, korridorsnätverk, zon-indelning, multi-nivås rum, strukturell trapp-placering, progressiv komplexitet"},
        {"6.3.0", "2026-10-13 — UI Wiring Repair: restored 14 DungeonDialog action connects, removed redundant library_dialog connect, audited disabled buttons (all intentional), verified AgingRules wiring"},
        {"6.2.0", "2026-10-13 — Dungeon Floor Fidelity: theme-driven tile mix, monsterLevelBonus, fog/rotator/extinguisher tiles wired, dead declarations removed"},
        {"6.1.0", "2026-10-12 — Spell Wiring & Loot Integrity: unified spell resolution, Kill/Banish schools wired, DoT spells, monster loot from level+tier, removed dead awardBattleLoot"},
        {"6.0.0", "2026-10-12 — Spell & Balance Fix: mana +5/level, XP curve 30*n^1.2, Thunderclap/Storm spells, instant kill fix, monster XP fix"},
        {"5.9.0", "2026-10-12 — Dead Code Removal: removed 17 standalone prototype files from src/"},
        {"5.8.0", "2026-10-12 — Wire & Cleanup Round 4: removed dead SenderWindow class"},
        {"5.7.0", "2026-10-12 — Wire & Cleanup Round 3: removed dead ShortcutHelp class"},
        {"5.6.0", "2026-10-12 — Wire & Cleanup Round 2: removed dead DungeonMap, hid helplesson dwarf image placeholder, options backup timestamp updates"},
        {"5.5.0", "2026-10-12 — Wire & Cleanup: arena status effects tick, PlayerExperience wiring, partyUpdated signal test, RaceData guild eligibility test, removed dead code (bankpane/, legacy PartyManager)"},
        {"5.4.0", "2026-10-12 — Main Menu & Save System redesign: New Game requires a character, Load/Save complete game state, Assemble Party dialog replaces Character List"},
        {"5.3.0", "2026-10-12 — Aging & Lifecycle: characters age over time with race-specific decay and max age, coming-of-age ceremony at 18, marriage system with spouse tracking, children and family lineage, descendant inheritance, retirement eligibility at 70, all lifecycle data persists in JSON saves"},
        {"5.2.0", "2026-10-12 — Savegame revamp: autosave uses PartyManager as single source of truth, party building from individual character files (1–4 members with their own inventory/equipment/hunger/guild data), CharacterListDialog preview with party size and clear selection, LoadPartyDialog with party preview showing members and levels, JSON-only save format (legacy .lua/.txt removed)"},
        {"5.1.0", "2026-10-12 — Main Menu redesign: Continue button with party preview, Load Party dialog, clear visual hierarchy, fixed grid collision, removed debug text"},
        {"5.0.0", "2026-10-12 — Survival: hunger is visible, food exists and can be bought and eaten, starvation death is a real death with a body and journal entry, one HP source of truth, a town resupply loop, and a whole party can be loaded at once (all 8 slices 5.1–5.8)"},
        {"4.0.0", "2026-10-11 — Endgame depth and replayability: New Game Plus carries the party and scales rewards, the Abyss mega-dungeon and Arena are wired, 25 achievements unlock and persist, and a balance pass fixes monster AC and dexterity so a full run is winnable in 10–15 hours (all 7 slices 4.1–4.6)"},
        {"3.0.0", "2026-10-11 — Tactical combat and character growth: all 9 v3.0.0 slices wired (target selection, per-monster display, front/back row, stat allocation, quest chain driver, play time, save versioning, combat visual feedback, repo hygiene)"},
        {"2.0.0", "2026-10-10 — Core loop complete: all 14 v2.0.0 slices wired (combat, equipment, XP, loot, death, persistence, monster spells, status effects, town, item ID, bestiary, journal, gold sinks, aging)"},
        {"1.0.0", "2026-10-10 — First finishable release: victory sequence, Hall of Records, New Game Plus, and the six orphaned systems wired in"},
        {"0.1.0", "2026-10-10 — Phase 0: security hardening and latent breakage fixes"},
        {"0.0", "2026-10-07 — Game systems built (8 phases)"},
        {"0.9.0", "2026-10-01 — Phase 7 complete"},
        {"0.8.0", "2026-09-28 — Phase 6 complete"},
        {"0.7.0", "2026-09-20 — Phase 5 complete"},
        {"0.6.0", "2026-09-15 — Phase 4 complete"},
        {"0.5.0", "2026-09-10 — Phase 3 complete"},
        {"0.4.0", "2026-09-05 — Phase 2 complete"},
        {"0.3.0", "2026-09-01 — Phase 1 complete"}
    };
}
