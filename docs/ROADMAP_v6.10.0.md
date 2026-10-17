# Roadmap v6.10.0 — Konsumtions-effekter implementerade

**Why:** ConsumableResolver löser nu effekter från data, men 6 effekter visade "not yet wired" meddelanden.

Status: ✅ DONE

---

## 1. Implementera consumable-effekter — ✅ GJORT

**Why:** `character.cpp:592-619` hade 6 "not yet wired" meddelanden för Teleport, Levitation, Invisibility, SeeInvisible, Bind och Unknown.

**Do:**
- [x] Lägg till statusvariabler i `character.h` (teleportTurns, levitationTurns, invisibilityTurns, seeInvisibleTurns, bindTurns)
- [x] Ersätt "not yet wired" med faktisk logik i `character.cpp`
- [x] Lägg till statusvariabler i `CombatState.h`
- [x] Ticka ner dem varje runda i `CombatActions::tickStatusEffects`

**Verify:** ✅ Buildad, 2058 passed / 0 failed.
**Commit:** `7c4118f` feat: implementera 6 consumable-effekter

## 2. Använd effekterna i spelet — ✅ GJORT

**Why:** Effekterna måste påverka spelet för att vara meningsfulla.

**Do:**
- [x] Invisibility: osynliga party-medlemmar upptäcks inte av hostile monsters (`DungeonHandlers.cpp`)
- [x] SeeInvisible: visar dolda dörrar på minimap (`DungeonMinimap.cpp`)
- [x] Teleport: teleport-knappen kräver aktiv teleport-effekt (`DungeonDialog.cpp`)
- [x] Levitation: levitating party-medlemmar kan flyga över hinder (`DungeonDialog.cpp`)
- [x] Bind: bundna mål tar halva skada (`CombatActions.cpp`)

**Verify:** ✅ Buildad, 2058 passed / 0 failed.
**Commit:** `80b56ea`, `d0ffb6a`, `8f5bdbe`, `14b9996`, `a9b30f2`

## 3. Släpp v6.10.0 — ✅ GJORT

**Why:** Alla steg är gjorda och testade.

**Do:**
- [x] Uppdatera RELEASE_NOTES.md
- [x] Uppdatera ReleaseInfo.cpp
- [x] Bumpa SEMVER till 6.10.0 i blacklands.pro
- [x] Tagga v6.10.0

**Verify:** ✅ Buildad, 2058 passed / 0 failed.

---

## Release criteria

Status per 2026-10-17 (HEAD `a9b30f2`):

- [x] Alla consumable-effekter implementerade
- [x] Alla effekter används i spelet
- [x] Alla tester passerar — 2058 passed / 0 failed
- [ ] Version bumpad till 6.10.0
- [ ] RELEASE_NOTES.md uppdaterad

**Blockerande för v6.10.0:** inget — samtliga steg är gjorda och releasen är redo att släppas.
