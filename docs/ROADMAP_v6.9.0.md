# Roadmap v6.9.0 — Städning, combat feedback, och QoL

**Why:** Döda filer tar plats, combat feedback kan förbättras, och QoL-förbättringar gör spelet mer behagligt att spela.

Status: 🔄 IN PROGRESS

---

## 1. Ta bort döda filer

**Why:** `automap_dialog_old.h`, `game_controller`, och `mordorstatistics` har ingen extern referens och är död kod.

**Do:**
- Ta bort `src/automap/automap_dialog_old.h`
- Ta bort `src/game_controller/game_controller.cpp` och `.h`
- Ta bort `src/mordorstatistics/mordorstatistics.cpp` och `.h`
- Ta bort poster från `blacklands.pro`
- Regenerera Makefile

**Verify:** Build, run tests, grep för noll referenser till borttagna klasser.

## 2. Färgkodade status-effekter i target list

**Why:** Status-effekter visas som text men inte med färger — svårt att snabbt se vilken monster som är poisoned/on fire/blinded.

**Do:**
- Lägg till färgkodade ikoner eller textfärger i target list
- Poison = grön, Fire = röd/orange, Blind = gul

**Verify:** Build, run tests, add test att färger korrekt tillämpas.

## 3. Combat log förbättring

**Why:** Combat log visar inte damage numbers, crits, eller misses tydligt.

**Do:**
- Lägg till damage numbers i combat log
- Markera crits med speciell färg eller text
- Markera misses tydligt

**Verify:** Build, run tests, add test att damage numbers visas.

## 4. Förbättra character system

**Why:** `character.cpp:514` har kommentaren "simplified system — full spell resolution comes in Phase 2".

**Do:**
- Ersätt simplified system med full spell resolution
- Använd SpellMechanics för att beräkna spell damage

**Verify:** Build, run tests, add test att spell resolution fungerar.

## 5. Dungeon minimap förbättring

**Why:** Minimap visar inte monster, dörrar, eller stairs.

**Do:**
- Visa monster som röda punkter på minimap
- Visa dörrar som gula punkter
- Visa stairs som blå punkter

**Verify:** Build, run tests, add test att minimap visar monster.

## 6. QoL: Spara/ladda snabbknappar

**Why:** Spelaren måste navigera till menyn för att spara/ladda.

**Do:**
- F5 = spara
- F9 = ladda
- F12 = snabbspara

**Verify:** Build, run tests, add test att shortcuts fungerar.

## 7. Balans: Monster scaling

**Why:** Monster HP/attack kan behöva justeras för bättre svårighetsgrad.

**Do:**
- Justera monster HP/attack i `MonsterBalance.cpp`
- Testa med `isRunWinnable()` och `isFloorWinnable()`

**Verify:** Build, run tests, add test att balans är inom mål.

---

## Release criteria

- [ ] Alla döda filer borttagna
- [ ] Färgkodade status-effekter implementerade
- [ ] Combat log förbättrad
- [ ] Character system förbättrat
- [ ] Minimap förbättrad
- [ ] Snabbknappar implementerade
- [ ] Balans justerad
- [ ] Alla tester passerar
- [ ] Version bumpad till 6.9.0
- [ ] RELEASE_NOTES.md uppdaterad
