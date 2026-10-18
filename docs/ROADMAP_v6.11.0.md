# Roadmap v6.11.0 — Multiplayer UI-integration

**Why:** NetworkManager, JSON-protokoll och referensservern är klara (merged). Nu ska multiplayern bli tillgänglig för spelaren — connecta till server, chatta, se andra spelare.

Status: 🟡 PÅGÅENDE

---

## 1. Multiplayer-lobby dialog — ❌ EJ GJORT

**Why:** Spelaren måste kunna ange host/port/username och connecta till en server.

**Do:**
- [ ] Skapa `MultiplayerLobbyDialog` — host, port, username inputs + connect/knapp
- [ ] Lägg till "Multiplayer"-knapp på huvudmenyn
- [ ] Connecta `NetworkManager::connectToServer()` från dialogen
- [ ] Visa connection status (connected/disconnected/reconnecting)

**Verify:** Bygg + tester passerar.

## 2. Chat-fönster — ❌ EJ GJORT

**Why:** Spelare kunna kommunicera via chat.

**Do:**
- [ ] Skapa `MultiplayerChatWidget` — QListWidget för meddelanden + QLineEdit för input
- [ ] Connecta `NetworkManager::chatReceived` → visa meddelande
- [ ] Skicka chat via `NetworkManager::sendAction("chat", {{"message": ...}})`
- [ ] Enter i inputfältet skickar meddelande

**Verify:** Bygg + tester passerar.

## 3. Spelarlista — ✅ GJORT

**Why:** Veta vem som är online.

**Do:**
- [ ] Visa connected players från `playerJoined`/`playerLeft` signals
- [ ] QListWidget eller QLabel med online-lista

**Verify:** Bygg + tester passerar.

## 4. Zone-sync mottagande — ✅ GJORT

**Why:** Motta se Andra spelares actions.

**Do:**
- [ ] Connecta `NetworkManager::zoneUpdateReceived` → visa i chat/log

**Verify:** Bygg + tester passerar.

## 5. Release — ❌ EJ GJORT

**Why:** Släpp v6.11.0.

**Do:**
- [x] Uppdatera RELEASE_NOTES.md
- [x] Uppdatera ReleaseInfo.cpp
- [x] Bumpa SEMVER till 6.11.0
- [ ] Tagga v6.11.0

**Verify:** Bygg + alla tester passerar.

---

## Release criteria

- [ ] Multiplayer-lobby dialog fungerar
- [ ] Chat fungerar (skicka/motta)
- [ ] Spelarlista visar online-spelare
- [ ] Zone-sync mottas och visas
- [ ] Alla tester passerar
- [ ] Version bumpad till 6.11.0
- [ ] RELEASE_NOTES.md uppdaterad
