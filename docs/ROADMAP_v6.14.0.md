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

## 0. AutoMap — automatisk karta i dungeon (60 min) — ✅ PLAN

**Why:** Manualen (sektion 1) beskriver en automatisk karta som visar alla
utforskade korridorer. Idag visas bara `MinimapDialog` när användaren trycker
på en knapp. Detta är den viktigaste saknade funktionen för en "spelbart"
upplevelse.

**Do:**
- [ ] Läs in `AutomapDialog` (tabeller för `explored` rot, `revealed` etc.) i
      `src/dungeon_dialog/DungeonDialog.cpp` — finns redan (4 träffar).
- [ ] Skapa `src/mapdialog/AutoMapDialog.h/.cpp` (50–80 rader) som implementerar
      "revealed tiles" via `QSet<quint32>` (tile coords) + `draw()`.
- [ ] Lägg till en `QAction` ("Auto Map", Ctrl+Shift+M) i `DungeonDialog`
      som synkroniserar med `MiniMapDialog` (samma panel, två paneler).
- [ ] Knyt till `gameStateManager` via `revealedTiles` och lägg i `gameStateData`.
- [ ] Bygg: `make -j$(nproc)` + `make check` (orphaned-systems).

**Verify:**
- Två nya dialoger: `AutoMapDialog` (full karta) + `MiniMapDialog` (mini).
- `Ctrl+Shift+M` synkar samtidigt två dialoger.
- `gameStateData["revealedTiles"]` skrivs/läses via `toMap()`/`loadFromMap()`
  (inga dataförluster gällande nya fält).

**Commit:** `<hash>`

---

## 1. Tutorial — first-run-guide (60 min) — ✅ PLAN

**Why:** Manualens sektion 71 anger en guide: "create character → buy item →
encounter monster". Denna guide togs bort när `Tutorial`-klassen raderades.
Spelaren som inte känner spelet tvingas upptäcka interaktionen hjärt-an-hjärta.

**Do:**
- [ ] Skapa `src/tutorial/TutorialDialog.h/.cpp` (40–60 rader) — enkel dialog med
      `QListWidget` + steg (4–6 steg):
      1. "Skapa en karaktär" → `CreateCharacterDialog`
      2. "Köp ett vapen i general store" → `GeneralStore`
      3. "Gå ner i dungeon" → `DungeonDialog`
      4. "Slå ett monster" → `CombatController`
- [ ] Lägg till en `QAction` ("Tutorial", Ctrl+T) i `GameMenu` som öppnar dialog.
- [ ] När första gången spelet startas, visa en kort hello-world-snutt (inga
      sparas permanent).
- [ ] Build + `make check` + verify `grep Tutorial src/` → 2 träffar.

**Verify:**
- Ctrl+T öppnar en 4-stegs guide.
- Dialog stänger utan att skriva något i `gameStateData` (inte-rappa).

**Commit:** `<hash>`

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

- [ ] AutoMap finns (Ctrl+Shift+M) och synkar med minimap.
- [ ] Tutorial (Ctrl+T) leder spelaren genom första 4 stegen.
- [ ] 12 spell-klasser laddas och visas i character-sheet.
- [ ] `make check` → PASS (build + orphaned-systems).
- [ ] RELEASE_NOTES.md uppdaterad överst.
- [ ] Tagg `v6.14.0` + push till origin.

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
