# Roadmap v6.13.0 — Återställ MORDOR:s gränssnitt

**Why:** En audit av källkoden mot `docs/MORDOR.rtf` (manualen, 72 sektioner) och
`docs/FIXLIST.md` visar att spelet saknar de flesta informationspanelerna som
manualen specificerar, att en knapp i shipped-game't bara säger "Coming soon.",
och att två krav i `docs/multiplayer_protocol.md` aldrig implementerades.
Detta är tysta luckor — inget kraschar, men spelaren saknar funktioner som
manualen lovar.

Status: 🟡 PLANERAD

**Audit-not (2026-10-21), verifierad mot källkoden:**

| # | Fynd | Bevis |
|---|------|-------|
| 1 | Endast **F5/F9/F12** finns (quicksave/quickload/autosave), och bara i `DungeonDialog`. Manualens F1–F11 informationspaneler saknas helt. | `grep -rnE "Key_F[0-9]+" src/` → 3 träffar, alla `DungeonDialog.cpp` |
| 2 | **Ingen toolbar** — manualens "The Toolbar" (pages, help, guild-exp-info) finns inte. | `src/` har ingen toolbar-klass |
| 3 | **"New Game Plus"**-knappen visar `QMessageBox("Coming soon.")` — död feature i en shipped version, med en **duplicerad lambda** (rad 47 och 56). | `src/victory_dialog/VictoryDialog.cpp:45-60` |
| 4 | `CombatController::castSpell()` är en **tom stub**. | `src/combat/CombatController.cpp:150-155` |
| 5 | Multiplayer saknar **resync-request** och **chat-sanitering**, båda uttryckligen uttryckta som krav i protokolldokumentet. | `grep -riE "resync\|sanitize" src/network_manager/` → 0 träffar |
| 6 | **8 vakuitiva tester** (`check(true, ...)`), varav 4 i mina egna multiplayertester. | `grep -nE "check\(\s*(true|1)\s*," test/*.cpp` |
| 7 | Balans: lvl-1-monster 7–65 HP, lvl-16 425–1250 HP. XP-kurvan är **golvbaserad** (`xpToNext` 30 → 1092), inte nivåbaserad. | `data/levels.json`, `data/bestiary.json` |

---

## 1. F-tangentens informationspaneler — ❌ EJ GJORT

**Why:** Manualen specificerar F1–F11 med varsin panel. Spelaren kan idag bara
quicksave/quickload/autosave. Att öppna Options kräver att man går tillbaka till
huvudmenyn. Detta är den största enskilda funktionsluckan.

**Do:**
- [ ] Beslut: F5/F9/F12 är quicksave/quickload/autosave idag och krockar med
      manualens F5 (Experience Information) och F9 (Buffer Information).
      **Rekommendation:** behåll F5/F9/F12 för save/load (spelarförväntan,
      test `[303]` täcker dem) och lägg manualens paneler på de fria tangenterna
      F1–F4, F6–F8, F10, F11. Dokumentera avvikelsen i manual-boken.
- [ ] Bygg ett gemensamt `InfoPanelDialog` (eller ett `keyPressEvent` som
      dispatchar) med paneler enligt manualen:
      - F1 = Hjälp/ledtrådar (manualens "Hints & Tips"-innehåll)
      - F2 = Area Look — entiteter i nuvarande zon
      - F3 = Character Stats → `CharacterSheetDialog`
      - F4 = Resistance Information
      - F6 = Character Information → `CharacterSheetDialog`
      - F7 = Guild/Abilities Information → `GuildsDialog`
      - F8 = Buffer Information
      - F10 = Game Options → `optionsdialog`
- [ ] Koppla in dem i **både** `DungeonDialog` och `theCity` (manualen säger
      "any time"), inte bara dungeonsdelen
- [ ] Verifiera med klick-/tangent-test att varje tangent gör något observerbart

**Verify:** Nytt test per tangent: skicka `QKeyEvent` och asserta att rätt panel
öppnas. Bygg + tester passerar.

## 2. Toolbar — ❌ EJ GJORT

**Why:** Manualens "The Toolbar" är en ikonrad med sidväxling (kommandon ↔
resistances), hjälp, guild-exp-info m.m. Utan den är varje kommando en menyknapp.

**Do:**
- [ ] Skapa en `GameToolbar` (QDockWidget eller QToolBar) med två lägen enligt manualen:
      kommandosida och informationssida (F11 växlar)
- [ ] Lägg till i både stad och dungeon
- [ ] Koppla ikonerna till samma slots som menyknapparna (duplicera inte logik)

**Verify:** Bygg + tester; test att F11 växlar läge och att en ikon-klick gör
samma sak som motsvarande menyval.

## 3. Ta bort "New Game Plus"-stubben — ❌ EJ GJORT

**Why:** En knapp i en släppt version som bara visar "Coming soon." är en trasig
sak enligt färdighetstestets tredje kriterium. Den har dessutom en duplicerad
lambda.

**Do:**
- [ ] Antingen: implementera NG+ (se backlog) eller ta bort knappen helt
- [ ] Ta bort den duplicerade lambda (`VictoryDialog.cpp:47` och `:56`)
- [ ] Uppdatera test som refererar till NG+-knappen

**Verify:** Bygg + tester passerar; ingen "Coming soon" kvar i `src/`.

## 4. Combat-spellcasting i strid — ❌ EJ GJORT

**Why:** `CombatController::castSpell()` är en tom stub, medan spelaren har en
"Cast Spell"-knapp i `SpellCastingDialog`. Att kasta besvärjelser i strid fungerar
alltså inte, trots att det finns ett helt besvärjelsystem.

**Do:**
- [ ] Implementera `castSpell()` mot `SpellCastingDialog`/`CombatActions`
- [ ] Se till att samma skada/kostnad används i tur-baserad strid som i
      utanför-strid (se FIXLIST-noten om två kodvägar för samma mekanik)
- [ ] Kosta mana och förfalla buffrar korrekt

**Verify:** Nytt test: kasta en skada-besvärjelse i strid → target tappar HP,
mana minskar, striden fortsätter. Mutebart.

## 5. Multiplayer-härdning (resync + sanitering) — ❌ EJ GJORT

**Why:** `docs/multiplayer_protocol.md` säger uttryckligen: "Missing sequences
should trigger a resync request" och "Sanitize chat and other user-supplied
strings on both client and server sides." Båda saknas.

**Do:**
- [ ] Resync: spåra senast sedda `seq` per källa; begär om-synk när ett
      meddelande saknas (trappa aldrig om igen utan be om full state)
- [ ] Sanitera chat på **både** klient och server (`tools/multiplayer/ref_server.py`)
      — inte bara truncera, utan hantera kontrolltecken
- [ ] Hantera protokollversion: ignorera/nedgradera meddelanden med fel version

**Verify:** Nytt test: skicka `zone_sync` med hoppade `seq` → klienten begär
resync; skicka chat med kontrolltecken → saniteras. Bygg + tester.

## 6. Testhygien — ❌ EJ GJORT

**Why:** 8 vakuitiva tester (`check(true, ...)`) — de bevisar inget och ger falsk
säkerhet. Fyra ligger i de multiplayertester som tillkom i v6.11.0.

**Do:**
- [ ] Ersätt `check(true)` med påståenden på observerbar effekt (t.ex. att
      `sendAction` faktiskt skriver till socketen, att servern stängs)
- [ ] F5/F9/F12-testerna: asserta att en fil faktiskt skrivs/läses, inte bara
      att inget kraschar

**Verify:** Bygg + tester passerar; `grep -cE "check\(\s*(true|1)\s*," test/` = 0.

## 7. Release v6.13.0 — ❌ EJ GJORT

**Why:** Släpp versionen.

**Do:**
- [ ] Bumpa SEMVER 6.12.0 → 6.13.0 i `blacklands.pro`
- [ ] `ReleaseInfo.cpp`: `releaseDate`, `versionHistory()[0]`, `changes()` topp
- [ ] `RELEASE_NOTES.md` ny sektion överst
- [ ] `qmake6`, `make clean`, fullt bygge, full testkörning
- [ ] Commit + tagga `v6.13.0`, pusha

**Verify:** Bygg + alla tester passerar; `version.h` visar 6.13.0.

---

## Release criteria

- [ ] Varje F-tangent i manualen gör något observerbart
- [ ] Toolbar finns med sidväxling
- [ ] Ingen "Coming soon" kvar i `src/`
- [ ] `castSpell()` är implementerad
- [ ] Resync + chat-sanitering implementerade
- [ ] Inga vakuitiva tester kvar
- [ ] Alla tester passerar
- [ ] Version bumpad till 6.13.0
- [ ] RELEASE_NOTES.md uppdaterad
- [ ] Taggad och pushad

---

## Backlog (föreslagna, ej planerade för v6.13.0)

Dessa är verifierade luckor men för stora för en enda version:

- **Companions** (manualens sektion 8): monster som blir följeslagare, använder
  sina förmågor (eld, syra, besvärjelser) i strid. Kräver ett helt subsystem.
- **AutoMap** (manualens sektion 1): medvetet borttagen i en tidigare version;
  `MinimapDialog` finns men är inte samma sak som manualens automatiska karta.
- **Tutorial** (manualens sektion 71): `Tutorial`-klassen togs bort; manualen
  har ett helt handledningsavsnitt som inte finns kvar.
- **Spell-klasser**: manualen listar 12 klasser (Healer, Mage, Ninja, Nomad,
  Paladin, m.fl.) med varsin spellista. Spelet använder guild i stället — en
  strukturell avvikelse som bör beslutas, inte patchas.
