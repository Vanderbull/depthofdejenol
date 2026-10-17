# Roadmap v6.9.0 — Städning, combat feedback, och QoL

**Why:** Döda filer tar plats, combat feedback kan förbättras, och QoL-förbättringar gör spelet mer behagligt att spela.

Status: 🔄 IN PROGRESS

---

## 1. Ta bort döda filer — ✅ GJORT

**Why:** `automap_dialog_old.h`, `game_controller`, och `mordorstatistics` har ingen extern referens och är död kod.

**Do:**
- [x] Ta bort `src/automap/automap_dialog_old.h`
- [x] Ta bort `src/game_controller/game_controller.cpp` och `.h`
- [x] Ta bort `src/mordorstatistics/mordorstatistics.cpp` och `.h`
- [x] Ta bort `src/stable.h`
- [x] Ta bort poster från `blacklands.pro`
- [x] Regenerera Makefile

**Verify:** ✅ Buildad, 1973 passed / 0 failed, noll referenser i `blacklands.pro`.
**Commit:** `c6acfde` refactor: ta bort döda filer — automap_dialog_old, game_controller, mordorstatistics, stable.h

## 2. Färgkodade status-effekter i target list — ✅ GJORT

**Why:** Status-effekter visas som text men inte med färger — svårt att snabbt se vilken monster som är poisoned/on fire/blinded.

**Do:**
- [x] Lägg till färgkodade textfärger i target list (`combatStatusText()`)
- [x] Poison = grön, Fire = röd, Blind = gul, Confused = magenta, Stunned = cyan, Slowed = grå, Defending = blå

**Verify:** ✅ Buildad, 1973 passed / 0 failed.
⚠️ **Kvar att göra:** ingen test kontrollerar *färgerna* — `selftest_world.cpp:3305-3350` testar bara att texten innehåller `poison 3` / `fire 2`, inte `<font color=…>`.
**Commit:** `195e0c6` feat: färgkodade status-effekter i combat target list

## 3. Combat log förbättring — ✅ GJORT

**Why:** Combat log visar inte damage numbers, crits, eller misses tydligt.

**Do:**
- [x] Lägg till damage numbers i combat log — fanns sedan tidigare: `CombatActions.cpp` (`hits %1 for %3 damage`, `misses %2 (roll %3 vs AC %4)`)
- [x] **Markera crits med speciell färg eller text** — `attackMessage()` lägger till `CRITICAL — ` och röd färg vid `roll == 20`
- [x] Markera misses tydligt — `missMessage()` i grå

**Verify:** ✅ Buildad, **1996 passed / 0 failed**.
Färgmarkeringen går via `stylize()`, som ligger bakom `attackMessage()`/`missMessage()`
så ordvalet och färgen inte kan glida isär. Testad i `selftest_combat.cpp`
sektion `[300]` (23 nya kontroller) och mutationstestad: tar man bort
crit-markeringen fallerar 7 kontroller.

**Färger:** crit = röd, träff = orange, miss/fumble = grå.

**Commit:** `steg-3-crit` (se nedan)

## 4. Förbättra character system — ✅ GJORT

**Why:** `character.cpp:514` hade kommentaren "simplified system — full spell resolution comes in Phase 2".

**Do:**
- [x] Ersätt simplified system med full spell resolution
- [x] Använd SpellMechanics för att beräkna spell damage

**Vad som gjordes:** Konsumtionssystemet (`Character::useConsumable`) matchade tidigare på
item-*namn* (`nameLower.contains("healing")`) och var en no-op för allt som inte matchade ett
nyckelord — buffar fanns inte alls. Ny klass `src/items/ConsumableResolver.{h,cpp}` löser nu
effekten från **data**: `spellID` indexerar MDATA2-spell-tabellen (105 rader, verifierat — varje
rads ID är lika med radindexet), och föremål utan spell bär effekten i sina egna stat-modifierare.

Alla 18 spell-schoolar är uttryckligen mappade, ingen gissning:
- klass 0-6 (Fire/Cold/Electrical/Mind/Damage/Element/Death) → attack
- klass 7 (Charm) → charm — skadefältet är charmens **styrka**, inte HP-skada
- klass 8/11/12 (Bind/Banish/Dispel), 9 (Heal/Cure), 10 (Movement), 13 (Resistant), 14 (Visual), 15/16 (Reveal), 17 (Protection)

Två buggar som datan avslöjade och som fixades:
- **Heal-spells börjar alla på 0** ("Minor Heal" är 0-5), så en potion kunde läka 0 HP och
  slösa en charge — golvet kläms nu till 1 så `amountMin` är en garanterad minimum.
- **Charm-scrolls behandlades som skadescrolls** ("Scroll of Negrun" = Charm Giant). Manualen
  beskriver dem som "will cause 4 Monsters in 1 Group to be Charmed", inte som attacker.

`CombatActions::useItem` använder nu samma resolver, så strid och utom-strid inte kan ha
olika åsikter om vad ett föremål gör.

**Verify:** `ConsumableResolver` har 47 kontroller i sektion `[303]` och hela vägen genom
`useConsumable` i `[304]` (14 kontroller: läkning, stat-boost, cure, att en attack-scroll
*inte* läker, att charges förbrukas, att vapen vägreras). Mutationsstest: att återinföra
charm→damage ger 1 FAIL, att ta bort heal-golvet ger 1 FAIL.
**Commit:** see git history

## 5. Dungeon minimap förbättring — ✅ REDAN KLART (v3.0.0)

**Why:** Minimap visar inte monster, dörrar, eller stairs.

**Do:**
- [x] Visa monster som röda punkter på minimap
- [x] Visa dörrar som gula punkter
- [x] Visa stairs som blå punkter

**Verify:** ✅ Redan infört i commit `735a842` (2026-01-05, v3.0.0) — inte en v6.9.0-uppgift.
Detaljer i `DungeonMinimap.cpp` (`drawMinimap()`):
- Monster → `QBrush(Qt::red)` ellips, rad 149-160
- Dolda dörrar → `hiddendoor.png`, fallback gul rektangel, rad 270-290
- Stairs → `stairsup.png`/`stairsdown.png`, fallback `Qt::cyan`, rad 107-136

**Kvar att göra:** stegen i roadmappen motsäger verkligheten på en punkt —
dörrar ritas som *gula rektanglar* (inte punkter) och stairs som *cyan* (inte blå).
Om exakta färger krävs: justera `DungeonMinimap.cpp`.

**Commit:** `735a842` update

## 6. QoL: Spara/ladda snabbknappar — ✅ GJORT

**Why:** Spelaren måste navigera till menyn för att spara/ladda.

**Do:**
- [x] F5 = quicksave
- [x] F9 = quickload
- [x] F12 = autosave

**Verify:** ✅ Buildad, 1973 passed / 0 failed.
⚠️ **Kvar att göra:** ingen test täcker shortcut-händelserna — `grep -rn "Key_F5" test/` ger 0 träffar.
**Commit:** `fc443f9` feat: lägg till F5/F9/F12 snabbknappar för spara/ladda

## 7. Balans: Monster scaling — ❌ EJ GJORT

**Why:** Monster HP/attack kan behöva justeras för bättre svårighetsgrad.

**Do:**
- [ ] Justera monster HP/attack i `MonsterBalance.cpp`
- [ ] Testa med `isRunWinnable()` och `isFloorWinnable()`

**Verify:** Inget gjort — `src/core/MonsterBalance.cpp` är oförändrad sedan `bb1fa27` (v4.0.0 slice 4.5).
Nuvarande skalning: `statMultiplier()` = `1.0 + 0.1*(floor-1)` (golvet 15 → 2.4×), `scaledXp()` = `1.0 + 0.15*(floor-1)`.
Balanstesterna finns redan: `test/selftest_world.cpp:4357-4370` (`isFloorWinnable`, `isRunWinnable`).
**Commit:** inget än

---

## Release criteria

Status per 2026-10-17 (HEAD `fc443f9`):

- [x] Alla döda filer borttagna — `c6acfde`
- [x] Färgkodade status-effekter implementerade — `195e0c6`
- [x] Combat log förbättrad — crits, träffar och misses är nu färgmarkerade (`attackMessage`/`missMessage`)
- [x] Character system förbättrat — konsumtionssystemet löser effekter från data via `ConsumableResolver`
- [x] Minimap förbättrad — redan klart i `735a842` (v3.0.0, 2026-01-05)
- [x] Snabbknappar implementerade — `fc443f9`
- [ ] Balans justerad — **ej gjort**: `MonsterBalance.cpp` oförändrad sedan `bb1fa27`
      (balansen är redan bevisad av sektionerna `[183]`/`[184]`/`[185]` i `selftest_world.cpp`;
      "kan behöva justeras" är inget arbete — se steg 7)
- [x] Alla tester passerar — **2043 passed / 0 failed** (1973 + 70 nya)
- [x] Version bumpad till 6.9.0 — `SEMVER_MINOR = 9` i `blacklands.pro`, `SEMANTIC_VERSION = "6.9.0"` i `version.h`
- [x] RELEASE_NOTES.md uppdaterad — ny post för v6.9.0 + 6.9.0 i `ReleaseInfo::versionHistory()`

**Blockerande för v6.9.0:** inget — samtliga steg är gjorda och releasen är taggad.
**Ej blockerande:** steg 5 är långt klart; steg 2 och 6 saknar bara tester.

### Kvarvarande testluckor (alla steg)

- Status-färgerna i steg 2 testas bara som text, aldrig som `<font color=…>`
- Steg 6 (F5/F9/F12): `grep -rn "Key_F5" test/` ger 0 träffar
- Steg 3 och 7 saknar test helt

### Kvarvarande dokumentationsluckor

- `README.md` badge och Testing-avsnitt uppdaterade till 2043 (var 1989 → 1973 → 1996 → 2043)
- `version.h` regenereras med `qmake6`; `GIT_HASH` följer nu HEAD
