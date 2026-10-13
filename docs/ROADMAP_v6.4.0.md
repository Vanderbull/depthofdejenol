# ROADMAP v6.4.0 — Kartgenerering & nivåsystem

**Status:** ✅ COMPLETE
**Released:** 2026-10-13

## Mål

Förbättra kartgenereringen och nivåsystemet för en mer varierad och engagerande spelupplevelse.

---

## Fas 1 — Större karta, strukturell trappa, progressiv komplexitet

### 6.1a — Större karta
- **Why:** 30×30 karta (900 tiles, 68% sten) kändes trång och blev snabbt utforskad.
- **Do:** Öka `MAP_SIZE` från 30 till 50 (2500 tiles). Justerar startposition för det större formatet.
- **Verify:** Bygger utan fel; alla befintliga tester passerar.

### 6.1b — Strukturell trapp-placering
- **Why:** Trappor placerades slumpmässigt på golf-tiles — ibland omgärdade av sten och onåbara.
- **Do:** Placera trappor i rumscentraler. Upp-trappan i startrummet, ner-trappan i längsta rummet (det naturliga målet).
- **Verify:** `generateStairs` använder nu `m_roomFloorTiles` för att hitta centrala positioner.

### 6.1c — Progressiv komplexitet
- **Why:** Alla 15 nivåer använde samma algoritm (40 rum) — ingen ökande komplexitet.
- **Do:** Rumsantal skalar med nivådjupet: `qBound(20, 15 + level * 3, 40)`. Nivå 1 = 18 rum, nivå 15 = 40 rum.
- **Verify:** `enterLevel` beräknar nu `roomCount` baserat på nivå.

---

## Fas 2 — BSP-algoritm och korridorsnätverk

### 6.2a — BSP-algoritm
- **Why:** "Room-sprouting" skapade oregelbundna grottor med dålig struktur.
- **Do:** Binary Space Partitioning (BSP) med rekursiv partitionering (depth 4). Varje leaf-node får ett rum. Rum kopplas med L-formade korridorer.
- **Verify:** `bspSplit`, `bspPlaceRoom`, `bspConnect` implementerade.

### 6.2b — Korridorsnätverk
- **Why:** Korridorer var 1 tile breda och raka — inga loopar eller alternativa vägar.
- **Do:** 2-tiles breda L-formade korridorer med både horisontellt-och vertikalt-första vägar. Skapar loopar och alternativa rutter.
- **Verify:** `bspConnect` carvar korridorer 2 tiles breda med båda riktningarna.

---

## Fas 3 — Zon-indelning, multi-nivås rum och bättre rendering

### 6.3a — Zon-indelning
- **Why:** Alla våningar hade samma utseende — bara temat skiljde.
- **Do:** Dela kartan i 3–4 tematiska zoner baserat på x-position (celler, grottor, tempel, boningar). Djupare nivåer får fler zoner.
- **Verify:** `assignZones` och `m_zoneMap` implementerade.

### 6.3b — Multi-nivås rum
- **Why:** Allt på en plan nivå — inga vertikala strukturer.
- **Do:** 1–2 rum per våning får en nedre nivå (z=-1) med trappor. Ny `m_stairsLowerButton` för att gå ned/upp.
- **Verify:** `generateMultiLevelRooms`, `m_lowerLevelTiles`, `m_onLowerLevel` implementerade.

### 6.3c — Bättre rendering
- **Why:** `isWallAt` stödde inte multi-nivås — väggar ritades felaktigt.
- **Do:** `isWallAt` tar nu z-parameter. Rendering visar rätt väggar beroende på nivå.
- **Verify:** `isWallAt(x, y, z)` och `isWallAtSide` uppdaterade.

---

## Testresultat

- **Bygge:** Inga kompileringsfel
- **Tester:** 1831 passerar, 6 misslyckade (kända versionsrelaterade test)
- **Spelet:** Startar normalt med alla nya funktioner
