# Roadmap v6.12.0 — Robust assets + code hygiene

**Why:** v6.11.0 gjorde multiplayern spelbar. Nästa version tar itu med tre saker
som en audit av källkoden mot `docs/FIXLIST.md` avslöjade: (1) `FIXLIST #6` stod
som FIXED men `__FILE__` användes fortfarande på fyra ställen för att hitta
`.qss`-filer i runtime — det fungerar bara så länge man kör från källträdet,
(2) `FIXLIST #21` hade ~292 kommenterade död-kods-rader kvar i
`gameStateManager.cpp`, och (3) FIXLIST:s egna statusar stämde inte med källkoden.

Status: 🟡 PÅGÅENDE

**Audit-not (2026-10-19):** Verifierat mot källkoden med `grep`:
- `__FILE__` fanns i `src/blacklands.cpp:264`, `src/blacklands.cpp:640`,
  `src/seer_dialog/SeerDialog.cpp:47`, `src/create_character/createcharacterdialog.cpp:129`
  → **FIXLIST #6 var felaktigt märkt FIXED.**
- `SIGNAL(` / `SLOT(` = **0 träffar** i `src/` och rotens `.cpp`
  → **FIXLIST #10 var i praktiken redan klar.**
- `src/core/gameStateManager.cpp` hade **292 kommentarsrader i 19 block**
  → **FIXLIST #21 kvarstod.**

---

## 1. Runtime-asset-uppslagning i stället för `__FILE__` — ✅ GJORT

**Why:** `QFileInfo(__FILE__).absolutePath()` expanderar till *byggtidens*
källväg. Den finns bara på den maskin som byggde binären. En installerad eller
flyttad binär tappar därför all styling tyst — appen kör men ser fel ut.
`docs/FIXLIST.md` påstod att detta var fixat; det var det inte.

**Do:**
- [x] Ny `src/core/ResourcePaths.h/.cpp` med `ResourcePaths::locate(fileName)`
- [x] Sökordning: `applicationDirPath()`, `appDir/..`, `appDir/../..`,
      `appDir/../../src`, `currentPath()`, `currentPath()/src`, samt kompilerad
      källkatalog som sista utväg (dev-byggen) — nuvarande beteende bevaras som
      ett av fallen, så inget som fungerade förr gick sönder
- [x] Ersätt de fyra `__FILE__`-ställena med `ResourcePaths::locate(...)`
      (`blacklands.cpp` ×2, `SeerDialog.cpp`, `createcharacterdialog.cpp`)
- [x] Registrera i `blacklands.pro` (SOURCES + HEADERS), `qmake6`, bygg
- [x] `grep -rn "__FILE__" src/` → **0 träffar** utanför `ResourcePaths`

**Verify:** Bygg + tester passerar. Nytt test `[305]` (8 checks), inkl. ett
cwd-oberoende probe som bevisar att `cwd/src`-kandidaten faktiskt används.
Muterades (kandidaten borttagen) → exakt **1 FAIL**, återställdes → grönt.
Smoke-test: stylesheet laddas nu från både `build/bin` (tidigare trasigt) och
källträdet. ✅

## 2. Ta bort kommenterad död kod (FIXLIST #21) — ✅ GJORT

**Why:** `gameStateManager.cpp` bar ~292 kommenterade rader gammal `m_PC`-erakod.
Den gör filen svårläst och döljer den levande koden. Git-historiken bevarar den.

**Do:**
- [x] Identifiera de kommenterade kodblocket (MaxXAge-tilldelningarna + de gamla
      `m_gameStateData["..."]`-tilldelningarna i `initializeGameState`)
- [x] Ta bort dem — vitrymmesokänslig diff: **55 borttagna rader, 0 tillagda**,
      alltså ingen logik ändrades
- [x] Rätta 27 tabbar till 4 mellanslag (projektkonventionen)
- [x] Samma genomgång i `src/dungeon_dialog/DungeonDialog.cpp`: alla 9 block är
      förklarande kommentarer, inget död kod — men två vilseledande kommentarer
      (som beskrev arbete som redan gjorts på raden under) rättades
- [x] `src/`-scan: **0 kommenterade C++-satser** kvar

**Verify:** Bygg + tester passerar. ✅

## 3. Gör FIXLIST sanningsenlig — ❌ EJ GJORT

**Why:** FIXLIST läses som sanning om vad som är kvar. Den säger FIXED om #6
(inte fixat förrän denna version) och ONGOING om #10 (klart sedan länge). En
felaktig lista leder nästa session fel.

**Do:**
- [ ] Sätt status per punkt mot verifierat källäge, med commit-hash eller radhänvisning
- [ ] Inför statusprefix i rubrikerna (`✅ FIXED` / `🟡 ONGOING` / `❌ OPEN`)
- [ ] Uppdatera "Last reviewed"-raden

**Verify:** Varje status går att belägga med en `grep` mot källkoden.

## 4. Release v6.12.0 — ❌ EJ GJORT

**Why:** Släpp versionen.

**Do:**
- [ ] Bumpa SEMVER 6.11.0 → 6.12.0 i `blacklands.pro`
- [ ] `ReleaseInfo.cpp`: `releaseDate`, `versionHistory()[0]`, `changes()` topp
- [ ] `RELEASE_NOTES.md` ny sektion överst
- [ ] `qmake6`, `make clean`, fullt bygge, full testkörning
- [ ] Commit + tagga `v6.12.0`, pusha

**Verify:** Bygg + alla tester passerar; `version.h` visar 6.12.0.

---

## Release criteria

- [x] `__FILE__` används inte längre för runtime-assets (0 träffar i `src/`)
- [x] Kommenterad död kod borta ur `gameStateManager.cpp`
- [ ] FIXLIST-statusar stämmer med källkoden
- [x] Alla tester passerar (2038 + 3, 0 failed; orphaned-systems PASS)
- [ ] Version bumpad till 6.12.0
- [ ] RELEASE_NOTES.md uppdaterad
- [ ] Taggad och pushad

---

## commits

| Commit | Vad |
|--------|-----|
| `45673ef` | Slice 1 — `ResourcePaths::locate()`, `__FILE__` borta, test `[305]` |
| `c7e0a4a` | Slice 2 — kommenterad död kod bort ur `gameStateManager.cpp` |
