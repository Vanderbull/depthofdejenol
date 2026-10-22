# Roadmap v6.15.0 — Kompanjoner, hållplats & ekonomin (LVC)

**Senaste release:** `v6.14.0` (taggad, ren trädstol: `fcbb075`). AutoMap (sep
1), Tutorial (sep 71) och Spell classes (sep 6) är klara. Kvar i MORDOR-manualen
är hjärtat på spelet:

| Manual-sep | Ämne | Status idag |
|------------|------|-------------|
| ~8 "Companions" | Magojer tvingar monster att bli kompanjoner; Hållplats & förvaring | **Kod finns, hanterare är stubs** — `Companion`-struct + `ConfinementAndHoldingDialog` (bind/identify/sell/buy), men `bindCompanion()` medför bara en beskedruta. |
| ~5 "Varoföremål" | Identifiering, gildor, containers, gold sinks | Butik finns; identifiering avgifter och gildens uppdrag är **saknas**. |
| "Biblioteket" | Dungeonkarta / bestiaries utökning | Delvis (JSON-filer finns). |

**Varför v6.15.0:** Kompanjoner är den största "manuellt trogna" funktionen där
koden är formgivningsmässigt komplett men tom i beteende. LVC här = koppla de
befintliga `Companion`-strukturen och `Confinement`-dialogen till datan — ingen
nytt subsystem (AI, räckvidd, gildor) den här veckan.

---

## Förslag till slices (LVC först, enklast först)

| # | Slice | LVC-mål | Timmar | Kostnad |
|---|-------|---------|--------|---------|
| 0 | Kompanjoncore | En Mage kan tvinga ett nivå-1 monster att bli en Kompanjon. Kompanjonen går med i partiet, har namn+kön+level+HP, dyrkar på djungeln, kan säljas på Hållplatsen. | 1.5 | 1350 kr |
| 1 | Hållplats & Förvaring | Befintliga dialogknappar (bind/identify/sell/buy) kopplas till datan: sälj en kompanjon, köp en ny från datan, betala avgifter. | 1.0 | 900 kr |
| 2 | Ekonomi: identifiering + gildor | Butiken tar en identifieringavgift; "gildor"/"uppdrag" betalar guld; kistor fungerar. | 1.5 | 1350 kr |
| **Totalt** | | 4.0 h | **3600 kr** |

*Lyft förbi 10 000 kr/veckans tak (~6 400 kr ≈ 7 h kvar). Ingen precision —
bart LVC.*

---

## Testregister v6.15.0

**Policy:** varje kontroll registreras **innan** körning, även de som misslyckas.
Ett misslyckat test återupprepas bara efter att datan är fixad — aldrig mot samma
stale-tillstånd.

| # | Kontroll | Förväntat (LVC) | Hur verifieras | Status |
|---|----------|------------------|----------------|--------|
| 0.1 | Kompanjon ansluter från Magojer | `Character::companions()` ej tom efter bind | runtime-harness: class=Mage, binda level-1 monster | ☐ |
| 0.2 | Kompanjon syns i djungeln | `getPartyMembers()[i].isCompanion()` sann; namn/kön/level | harness: läs kompanjonslotten | ☐ |
| 0.3 | Kompanjon slår | Tillfällig HP går ner vid angrepp | harness: `useCombatAction` mot en kompanjon | ☐ |
| 0.4 | Kompanjon säljs | Hållplatsens sell returnerar guld, sloten töms | harness: guld innan/efter `sellCompanion` | ☐ |
| 0.5 | Dålig kompanjon nekas | ingen krash / ingen kostnad vid bind utan data | harness: tomt slot, förväntat `QMessageBox`, intet krash | ☐ |
| 1.1 | Sell ger guld | partiguldet ökar | harness: guld innan/efter | ☐ |
| 1.2 | Köp visar data | köp-listan ej tom | harness: `buyCompanion()` visar >0 | ☐ |
| 1.3 | Bind kostar guld | hanteraren sparar avgift, inte krash | harness: guld innan/efter | ☐ |
| 2.1 | Identifiering har avgift | `Item::identifyPrice` > 0, avgift dras | harness: identifiera, guld -avgift | ☐ |
| 2.2 | Gildens uppdrag betalar guld | avslutat uppdrag ökar partiguldet | harness: slutför gilduppdrag | ☐ |
| 2.3 | Kistoperationer | förvara, hämta, sälja ur kista | harness: förvara → hämta → sälj | ☐ |
| 2.4 | Dålig/äldre save överlever | gammal save (utan identifiering) laddar | läs en v2-save, ingen krash | ☐ |

**Körordning:** 0.1→0.5 (kompanjoner), sedan 1.1→1.3 (hållplats), sedan
2.1→2.4 (ekonomi). Portar: slice N godkänns innan N+1 börjar. Ett misslyckat
test registreras med sin anledning och återupprepas bara efter fixen — aldrig
dolt.

---

## Viktiga fynd (vad analysen visade)

1. `struct Companion` i `src/character/character.h` har förortsmärkta namn
   (`M548B`, `M4ED4`, …) — reverse-engineered återställning, inte ren källkod.
   Behandla som databärare, inte som spec.
2. `ConfinementAndHoldingDialog` har redan layout (bind/identify/realign/sell/buy
   grupper, sökbox); endast hanterarna är stubs — detta är den billigaste vägen
   till en levande kompanjonfunktion.
3. Spell-skolor är fullt mappade (18, enligt `src/core/ReleaseInfo.cpp` 6.9.0) —
   hoppa över "spell-skolor" som v6.15.0-slice.
4. I18n: apptext på svenska, användaren förväntas på engelska — allt spelande
   avsnitt på svenska.

## Risker

- Kompanjon-atskott (combat AI) hamnar utanför LVC vecka 1 (kan behöva en 2:a
  vecka med precision). Om en slice överstiger 4 h, nedskala till datanivån bara.
- Hållplatsens UI-text måste översättas till svenska (`tr()` / `.pro` ts) — fall
  tillbaka på `QString` matching om det bromsar.
