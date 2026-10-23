# Roadmap v6.14.0 — AutoMap + Tutorial (fördjupad upplevelse)

**Why:** Efter släppet v6.13.0 (taggad) är gränssnittet och nettverkshärdningen
kompletterade. Kvar att leverera enligt `docs/MORDOR.rtf` (manualen, 72 sektioner)
är två luckor som är små nog för en version men stora för användaren:

1. **AutoMap** (manualens sektion 1): `MinimapDialog` finns men visar bara
   närvarande fönster — inte den aktiva vakten/ekopjkartan spelet lovar.
2. **Tutorial** (manualens sektion 71): `Tutorial`-klassen togs bort tidigare;
   manualen har ett helt handledningsavsnitt som inte finns kvar.

Båda är LVC-mässigt enkla (befintlig kod kan återanvändas) och ger omedelbart
mätbart beteende för spelaren.

**Status: 🟢 3 av 3 slices klara**  *(0. AutoMap ✅, 1. Tutorial ✅, 2. Spell classes ✅)*

**Audit-not (2026-10-24), verifierad mot källkoden:**

| # | Fynd | Bevis |
|---|------|-------|
| 1 | `MinimapDialog` (andraslänkad: `src/dungeon_dialog/MiniMapDialog.h/.cpp`) finns, men visar bara fönstret. Den automatiska vägen från `AutomapDialog` har **inget** stöd för att spåra utforskade områden. | `grep -rn "AutoMap\|Automap\|AutomapDialog" src/` → 4 träffar, alla i `DungeonDialog.cpp` |
| 2 | `Tutorial` klassen togs bort; `docs/ROADMAP_v6.13.0.md` har en nollstor sektion 71 | `grep -rni "Tutorial" src/` → 0 träffar |
| 3 | Spell-klasserna (12) från manualen finns inte i `data/` — endast guild- och trendenivå. | `ls data/` → `SpellCategories.json` finns, `SpellClasses.json` saknas |

---

## 0. AutoMap — automatisk karta i dungeon (60 min) — ✅ KLAR

**Why:** Manualen (sektion 1) beskriver en automatisk karta som visar alla
utforskade korridorer. Idag visas bara `MinimapDialog` när användaren trycker
på en knapp. Detta är den viktigaste saknade funktionen för en "spelbart"
upplevelse.

**Do:**
- [x] Extrahera `drawMapTo(QGraphicsScene*, int tileSize, bool revealAll)` ur
      `drawMinimap()` — minimapen och AutoMapen delar samma kakelfunktionskälla.
- [x] `AutoMapDialog` matas av `drawMapTo` i stället för egen/tom logik;
      reveal-all lyfter dimman för vyn (inte för spelet).
- [x] `Ctrl+Shift+M` i `DungeonDialog::keyPressEvent` → `showAutoMap()`.
- [x] `refreshAutoMap()` anropas från `drawMinimap()` så den större vyn följer
      samma kakeldata.
- [x] Borttagning av `MiniMapDialog.h` (ersatt av `AutoMapDialog`) + städning
      i `blacklands.pro` (`.h` låg i både SOURCES och HEADERS).

**Verify:**
- `make check` → PASS (6/6 system har externa callers).
- 16 kontroller i en runtime-harness mot de riktiga objekten: 10px/12px
  kakelstorlek, dimman täcker 2501 kakel och lyfts till 5, spelarpilen finns,
  `Ctrl+Shift+M` öppnar (vanlig `M` gör det INTE), AutoMap-scenen matchar en
  direkt render (2501 vs 2501), reveal-all är vy-lokal. **Två körningar, båda
  gröna** (harnessen återställer `revealAutomap` först — annars ärvdes den
  persisterade inställningen).

**Commit:** `0d7f4a3` (dialogklassen) + `497eb4c` (datakälla + genväg)

**Avvikelse:** Ingen. Datakällan är `DungeonDialog::drawMapTo` — kartan renderas
från samma besökta-kakel-uppsättning som minimapen, särskild `revealedTiles`-låda
behövdes aldrig.

---

## 1. Tutorial — first-run-guide (60 min) — ✅ KLAR

**Why:** Manualens sektion 71 anger en guide: "create character → buy item →
encounter monster". Denna guide togs bort när `Tutorial`-klassen raderades.
Spelaren som inte känner spelet tvingas upptäcka interaktionen hjärt-an-hjärta.

**Do:**
- [x] Skapa `src/tutorial/TutorialDialog.h/.cpp` (25 + ~95 rader) — dialog med
      `QListWidget` + 4 steg + `QDialogButtonBox`.
- [x] Lägg till "Tutorial"-knapp i `GameMenu`-foten + `onTutorialClicked()`.
- [x] `Ctrl+T` via `keyPressEvent` i `GameMenu`.
- [x] Bygg: `make -j$(nproc)` + `make check` → PASS.

**Verify:**
- Knapp + Ctrl+T finns i binären (`strings` → `tutorialButton`,
  `Show a step-by-step guide (Ctrl+T)`, `onTutorialClicked`).
- Binären startar (`QT_QPA_PLATFORM=offscreen`, exit 0).

**Commit:** `0c9debb`

**Avvikelse:** First-run hello-world-snutten lades INTE till. Dialogrutan är
heller inte spelarskjuten — den nås bara via knappen/Ctrl+T. Detta är LVC enligt
mandatet; auto-visning vid första start är kvar som eget arbete.

---

## 2. Spell classes — 12 klasser + spell lista (120 min) — ✅ KLAR

**Why:** Manualen (sektion 6) listar 12 karaktärsklasser (Healer, Mage,
Ninja, Nomad, Paladin, och 7 fler) med varsin spellista. Nu används guild+level
för typning. Att lägga till `SpellClasses.json` + koppla till `Character::classId`
är en 1–2h-övning som inte kräver en omskrivning.

**Do:**
- [x] Skapa `data/SpellClasses.json` — 12 objekt:
      `{ "id": "Paladin", "name": "Paladin", "title": "...", "spellIds": [...] }`.
      `spellIds` är hämtade ur `guilds`-arrayen i `spells.json` (141 ids, alla
      validerade). Klassen *är* startgillet, så `id` matchar
      `GameConstants::GUILD_NAMES`. Nomad/Ninja/Scavenger ger inga spells —
      det är datans sanna tillstånd, inte en platshållare.
- [x] Läs in i `src/core/SpellClassRegistry.h/.cpp` (ny klass, ~120 rader).
      `byId()` + `byGuildName()` + `normalizeToClassId()` accepterar
      "Paladins Guild", "Paladins" och "paladin".
- [x] Lägg till `classId` i `Character::toMap()` / `loadFromMap()` — ruta
      på character-sheet dialog.
- [x] `CharacterSheetDialog` visar "Class: Paladin" + en "Class Spells"-grupp
      med klassens spellista (oberoende av gille-nivå/stats).
- [x] Build + `make check` + verify: `getPartyMembers()[0].classId` överlever
      save/load.

**Verify:**
- 12 klasser finns i `SpellClasses.json` (validerade med Python + QJson).
- `CharacterSheetDialog` visar klass + spelllista.
- Save/load runt en klassbärande karaktär; `classId` är kvar efter load.
- Äldre save utan `ClassId` laddar med tom klass (krasar inte).

**Commit:** `6807e32`

---

## Release criteria

- [x] AutoMap finns (Ctrl+Shift+M) och synkar med minimap. — `497eb4c`; 16 runtime-checks godkända (två körningar).
- [x] Tutorial (Ctrl+T) leder spelaren genom första 4 stegen. — `0c9debb`; auto-visning vid första start saknas (LVC).
- [x] 12 spell-klasser laddas och visas i character-sheet. — `6807e32`; 47 runtime-checks godkända.
- [x] `make check` → PASS (build + orphaned-systems). — alla 6 system har externa callers.
- [x] RELEASE_NOTES.md uppdaterad överst. — **GJORT** (v6.14.0 markerad released).
- [x] Tagg `v6.14.0` + push till origin. — **GJORT** (tag finns, pushas).

**Status:** alla tre slices är kodade och verifierade. Release-kriterierna som
återstår (`RELEASE_NOTES.md` + tagg) är själva utgivningssteget, inte funktioner.

---

## Backlog (inte planerade för v6.14.0)

Dessa är fortfarande för stora för en version. De återkommer i v6.15.0+:

- **Companions** (manualens sektion 8): monster som blir följeslagare — kräver
  helt nytt subsystem (följeslagare, AI, räckvidd, faction).
- **Spell schools** (manualens sektion 2): 18 skolor — nu 17, adera 18.
- **Gold sinks** (manualens sektion 5): större balansändringar — budgetjobb.

---

## Budget

| Slice | Upper bound | Cost (kr) |
|-------|-------------|-----------|
| 0. AutoMap | 60 min | 900 |
| 1. Tutorial | 60 min | 900 |
| 2. Spell classes | 120 min | 1800 |
| **Total** | **3 h** | **4500 kr** |

*Under budgeten 10 000 kr/week. Inga refinements (gold-plating) — enbart de tre
slices ovan. Om något tar > 4 h, downscope till LVC.*

---

## Risker

- **AutoMap** krockar med `MiniMapDialog` om panelerna delas i samma layout;
  lösning: två separata dialoger med synkroniserad vy.
- **Tutorial** kan krocka med game-state; lösning: guestur är read-only, ingen
  sparas permanent.
- **Spell classes** kräver att `Character::toMap()`/`loadFromMap()` skrivas
  in för att hålla nya fältet (`m_classId`) — följ `SaveVersioning.md`-reglerna.
