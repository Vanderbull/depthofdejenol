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

## 3. Combat log förbättring — 🟡 DELVIS

**Why:** Combat log visar inte damage numbers, crits, eller misses tydligt.

**Do:**
- [x] Lägg till damage numbers i combat log — finns sedan tidigare: `CombatActions.cpp:102-112` (`hits %1 for %3 damage`, `misses %2 (roll %3 vs AC %4)`)
- [ ] **Markera crits med speciell färg eller text** — `CombatActions.cpp:96` dubblar skadan vid `roll == 20` men meddelandet är oförändrat (`hits %1 for %3 damage.`); endast ljudet skiljer (`SoundEffects::Type::CriticalHit`)
- [x] Markera misses tydligt — finns sedan tidigare: `CombatActions.cpp:112`

**Verify:** ⚠️ Ej täckt av någon test.
**Commit:** inget än — crit-delen kvarstår

## 4. Förbättra character system — ❌ EJ GJORT

**Why:** `character.cpp:514` har kommentaren "simplified system — full spell resolution comes in Phase 2".

**Do:**
- [ ] Ersätt simplified system med full spell resolution
- [ ] Använd SpellMechanics för att beräkna spell damage

**Verify:** Inget gjort — `character.cpp:514` har fortfarande kommentaren och konsumtionslogiken matchar på item-namn (`nameLower.contains("healing")`) istället för att gå via `SpellMechanics`, som `CombatActions.cpp:301-374` redan gör.
**Commit:** inget än

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
- [ ] Combat log förbättrad — **delvis**: damage numbers och misses finnas sedan tidigare; crits (roll 20) markeras fortfarande inte i texten
- [ ] Character system förbättrat — **ej gjort**: `character.cpp:514` är oförändrad
- [x] Minimap förbättrad — redan klart i `735a842` (v3.0.0, 2026-01-05)
- [x] Snabbknappar implementerade — `fc443f9`
- [ ] Balans justerad — **ej gjort**: `MonsterBalance.cpp` oförändrad sedan `bb1fa27`
- [x] Alla tester passerar — 1973 passed / 0 failed (verifierat: `./build/bin/blacklands --selftest`)
- [ ] Version bumpad till 6.9.0 — `blacklands.pro` säger fortfarande `SEMVER 6.8.0`
- [ ] RELEASE_NOTES.md uppdaterad — senaste posten är fortfarande v6.8.0

**Blockerande för v6.9.0:** steg 3 (crits), 4 (character system), 7 (balans) + semver-bump.
**Ej blockerande:** steg 5 är långt klart; steg 2 och 6 saknar bara tester.

### Kvarvarande testluckor (alla steg)

- Status-färgerna i steg 2 testas bara som text, aldrig som `<font color=…>`
- Steg 6 (F5/F9/F12): `grep -rn "Key_F5" test/` ger 0 träffar
- Steg 3 och 7 saknar test helt

### Kvarvarande dokumentationsluckor

- `README.md` badge och Testing-avsnitt sade `Tests-1989`, faktiskt 1973
- `version.h` är stale: `GIT_HASH = "92f2524"` medan HEAD är `fc443f9`
  (filen genererades 04:06, före det sista committet — regenerera med `qmake6`)
