# Roadmap v6.11.0 — Multiplayer UI-integration

**Why:** NetworkManager, JSON-protokoll och referensservern är klara (merged). Nu ska multiplayern bli tillgänglig för spelaren — connecta till server, chatta, se andra spelare.

Status: ✅ DONE (alla slices implementerade; release taggad)

**Audit-not (2026-10-19):** Denna roadmap var felaktigt märkt — slice 1 och 2 stod
som "✅ GJORT" trots att `MultiplayerLobbyDialog` med chatt, spelarlista och
zone-sync redan var implementerad i `src/multiplayer_dialog/`. Verifierat mot
källkoden: `m_chatDisplay`, `m_chatInput`, `onChatSend`, `onChatReceived`,
`m_playerList`, `onPlayerJoined`, `onPlayerLeft`, `onZoneUpdateReceived` finns alla
och är connectade. Endast release-steget återstod.

---

## 1. Multiplayer-lobby dialog — ✅ GJORT

**Why:** Spelaren måste kunna ange host/port/username och connecta till en server.

**Do:**
- [x] Skapa `MultiplayerLobbyDialog` — host, port, username inputs + connect/knapp
- [x] Lägg till "Multiplayer"-knapp på huvudmenyn
- [x] Connecta `NetworkManager::connectToServer()` från dialogen
- [x] Visa connection status (connected/disconnected/reconnecting)

**Verify:** Bygg + tester passerar. ✅

## 2. Chat-fönster — ✅ GJORT

**Why:** Spelare kunna kommunicera via chat.

**Do:**
- [x] Skapa chatt-panel — `QTextEdit` för meddelanden + `QLineEdit` för input
- [x] Connecta `NetworkManager::chatReceived` → visa meddelande
- [x] Skicka chat via `NetworkManager::sendAction("chat", {{"message": ...}})`
- [x] Enter i inputfältet skickar meddelande (`returnPressed` → `onChatSend`)
- [x] Ljudnotis vid nytt meddelande (`resources/sounds/chat_notify.wav`)

**Verify:** Bygg + tester passerar. ✅

## 3. Spelarlista — ✅ GJORT

**Why:** Veta vem som är online.

**Do:**
- [x] Visa connected players från `playerJoined`/`playerLeft` signals
- [x] `QListWidget` med online-lista

**Verify:** Bygg + tester passerar. ✅

## 4. Zone-sync mottagande — ✅ GJORT

**Why:** Motta se Andra spelares actions.

**Do:**
- [x] Connecta `NetworkManager::zoneUpdateReceived` → visa i lobbyn

**Verify:** Bygg + tester passerar. ✅

## 5. Release — ✅ GJORT

**Why:** Släpp v6.11.0.

**Do:**
- [x] Uppdatera RELEASE_NOTES.md
- [x] Uppdatera ReleaseInfo.cpp (`releaseDate` 2026-10-19, `versionHistory[0]`, `changes()`)
- [x] Bumpa SEMVER till 6.11.0 (`blacklands.pro`)
- [x] Tagga v6.11.0
- [x] Gjort packaging-testet icke-fragilt: `[56]` läser `ReleaseInfo::releaseDate()`
      i stället för en hårdkodad datumsträng

**Verify:** Bygg + alla tester passerar. ✅

---

## Release criteria

- [x] Multiplayer-lobby dialog fungerar
- [x] Chat fungerar (skicka/motta)
- [x] Spelarlista visar online-spelare
- [x] Zone-sync mottas och visas
- [x] Alla tester passerar
- [x] Version bumpad till 6.11.0
- [x] RELEASE_NOTES.md uppdaterad
