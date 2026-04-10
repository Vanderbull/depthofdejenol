# Depth of Dejenol — Fix List
Ordered by severity and impact. Each item includes location, what's wrong, and the fix.

---

## 1. Remote Code Execution via Lua Socket
**File:** `GameStateManager.cpp:1609`
**Problem:** `onServerDataReceived()` takes raw bytes from a TCP socket and passes them directly to `luaL_dostring(m_L, data.constData())`. Any process that can connect to `127.0.0.1:12345` can execute arbitrary Lua — and Lua has `os.execute()`, `io.open()`, etc. This is a full RCE vulnerability.
**Fix:** Remove the `luaL_dostring` call entirely. Instead, parse the server message as structured data (e.g., JSON or a fixed command set) and dispatch known actions only. If you need remote Lua scripting for development, gate it behind a `#ifdef DEBUG` and disable in release builds. At minimum, sandbox the Lua state by removing `os` and `io` libs before opening the socket.

---

## 2. Dead Characters Can Never Be Saved as Dead ( FIXED )
**File:** `GameStateManager.cpp:784`
**Problem:** `saveCharacterToFile()` writes:
```cpp
out << "isAlive: " << (character["Dead"].toBool() ? 0 : 1) << "\n";
```
The character map uses `"isAlive"` (set in `Character::toMap()`), not `"Dead"`. Since `"Dead"` doesn't exist in the map, `QVariant().toBool()` always returns `false`, so `isAlive` is always saved as `1`. Every character is saved as alive regardless of actual state.
**Fix:** Change the line to:
```cpp
out << "isAlive: " << (character["isAlive"].toBool() ? 1 : 0) << "\n";
```

---

## 3. `hasLivingCharacters()` Rejects Living Characters with Status Effects ( FIXED )
**File:** `GameStateManager.cpp:1774`
**Problem:** The check `character.status == 0` means any character who is Poisoned, Blinded, or On Fire is considered "not living". Worse, the `Alive` flag itself (bit 3, value 8) makes `status != 0`, so a character explicitly marked alive via the flag system also fails this check.
**Fix:** Replace `character.status == 0` with `character.isAlive` (the bool field). This is the correct "living" check that matches `Character::loadFromMap`:
```cpp
if (character.name != "Empty Slot" &&
    character.hp > 0 &&
    character.isAlive)
{
    return true;
}
```

---

## 4. `initializeParty()` Dead Code and Emit Spam ( ONGOING )
**File:** `GameStateManager.cpp:1274-1314`
**Problem:** Inside the loop:
- Line 1305: `emit gameValueChanged(...)` fires on every iteration (4 signals instead of 1).
- Line 1307: Redundant `c.loadFromMap(character)` — reloads the same data into a local copy that's already been appended by value.
- Lines 1298, 1306, 1308-1309, 1311: Commented-out dead code.
**Fix:** Remove the dead code and move the emit outside the loop:
```cpp
void GameStateManager::initializeParty() {
    for (int i = 0; i < GameConstants::MAX_PARTY_SIZE; ++i) {
        QVariantMap character;
        character["Name"] = "Empty Slot";
        // ... (all the field assignments stay the same) ...
        character["Inventory"] = QStringList();

        Character c;
        c.loadFromMap(character);
        m_currentParty.members.append(c);
    }
    m_gameStateData["Party"] = m_currentParty.toMap();
    m_gameStateData["PartyHP"] = QVariantList({50, 40, 30});
    emit gameValueChanged("party_data", m_currentParty.toMap());
}
```

---

## 5. Font Sprite Layout Mismatch Between Header and Implementation ( FIXED )
**Files:** `FontManager.h:43`, `fontManager.cpp:55-59`
**Problem:** The header defines:
```
m_layout = "ABCDEFGHIHIJKLMMNOPQRSTUUVWUWXYZ124578901";
```
This has duplicates (H×2, I×2, M×2, U×2, W×2) and is missing J, 3, 6. The actual implementation in `fontManager.cpp` uses a *different* hardcoded layout string and never references `m_layout` at all. The two are out of sync.
**Fix:** Define the layout once in the header as the single source of truth, fix the character sequence to match the actual spritesheet, and use `m_layout` in `drawSpriteText()` instead of a local variable. The corrected layout (based on the row comments in the .cpp) should be:
```
"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"
```
Adjust row boundaries in `drawSpriteText()` to match the real sprite grid.

---

## 6. `__FILE__` Used for Runtime Resource Paths ( FIXED )
**Files:** `blacklands.cpp:103`, `SeerDialog.cpp:26`
**Problem:** Both use `QFileInfo(__FILE__).absolutePath()` to locate `.qss` stylesheets at runtime. `__FILE__` expands to the *build-time source path*, which won't exist on any other machine or after installation.
**Fix:** Use `QCoreApplication::applicationDirPath()` or Qt's resource system (`qrc`) to locate runtime assets:
```cpp
QString qssPath = QCoreApplication::applicationDirPath() + "/styles/SeerDialog.qss";
```
Or embed stylesheets in a `.qrc` resource file.

---

## 7. "Ghost hounf" Typo ( FIXED )
**File:** `ConfinementDialog.cpp:30, 265`
**Problem:** `"Ghost hounf"` should be `"Ghost hound"`. This means the Ghost Hound creature never matches its stock entry correctly.
**Fix:** Change both occurrences:
- Line 30: `gsm->incrementStock("Ghost hound");`
- Line 265: `buyCreatureListWidget->addItem("Ghost hound 75000     0  2  0");`

---

## 8. Log Message Doesn't Match Actual Count ( FIXED )
**File:** `DungeonDialog.cpp:82, 100`
**Problem:** The loop runs while `itemsPlaced < 100` (places 100 items), but the debug log says `"Successfully placed 10 random items"`.
**Fix:** Change line 100 to:
```cpp
qDebug() << "Successfully placed" << itemsPlaced << "random items from MDATA3 on level" << level;
```

---

## 9. Meaningless Null Check on Singleton ( FIXED )
**File:** `DungeonDialog.cpp:467-469`
**Problem:** `GameStateManager::instance()` returns a pointer to a `static` local — it can never be null. The null check is dead code that gives a false sense of safety.
**Fix:** Remove lines 467-470:
```cpp
// Remove:
if (!GameStateManager::instance()) {
    qCritical() << "CRITICAL: GameStateManager is NULL!";
    return;
}
```

---

## 10. Old String-Based SIGNAL/SLOT Macros ( ONGOING )
**Files:** `blacklands.cpp:302-303`, `DungeonDialog.cpp:40, 65`
**Problem:** Uses old Qt4-style `SIGNAL()` / `SLOT()` macros which bypass compile-time type checking. Typos in signal/slot names silently fail at runtime.
**Fix:** Replace with modern pointer-to-member syntax:
```cpp
// blacklands.cpp:302-303
connect(dialog, &CreateCharacterDialog::characterCreated,
        this, &GameMenu::onCharacterCreated);

// DungeonDialog.cpp createButton/setupControls — refactor to use lambdas or
// &Class::method pointers instead of SIGNAL()/SLOT() strings.
```

---

## 11. Duplicate Includes ( FIXED )
**File:** `SeerDialog.cpp:4-9`
**Problem:** `QFile` is included on lines 4 and 6; `QTextStream` on lines 5 and 9.
**Fix:** Remove lines 6 and 9.

---

## 12. `readyBodyForResurrection` Declared but Never Defined
**File:** `GameStateManager.h:169`
**Problem:** `bool readyBodyForResurrection(const QString& characterName);` is declared but has no implementation in the .cpp file. Any call will cause a linker error.
**Fix:** Either implement the function or remove the declaration if it's not needed yet.

---

## 13. Lua State `m_L` Never Closed
**File:** `GameStateManager.cpp:162` (constructor), no destructor
**Problem:** `m_L = luaL_newstate()` allocates a Lua state that is never closed. `GameStateManager` has no destructor, so the state leaks. (The singleton pattern makes this less critical since it lives for the process lifetime, but it's still bad practice and will show up in leak checkers.)
**Fix:** Add a destructor:
```cpp
GameStateManager::~GameStateManager() {
    if (m_L) lua_close(m_L);
}
```
And declare it in the header.

---

## 14. ODR Violations in `GameConstants.h`
**File:** `src/core/GameConstants.h:37-39, 80-88, 97-99`
**Problem:** `const QString` and `const QStringList` at namespace scope have internal linkage in C++, but each translation unit gets its own copy. This wastes memory and can cause subtle bugs with `QStringList` construction order. More importantly, `STAT_NAMES`, `ALIGNMENT_NAMES`, `GUILD_NAMES`, etc. are constructed in every `.cpp` that includes this header.
**Fix:** Mark them `inline` (C++17) so there's a single definition:
```cpp
inline const QStringList STAT_NAMES = {"Strength", "Intelligence", ...};
inline const QString CAT_RACES = "Races";
```

---

## 15. Duplicate Teleport Tile Sets
**File:** `DungeonDialog.h:179, 181`
**Problem:** Two nearly-identical members: `m_teleportPositions` and `m_teleporterPositions`. `generateSpecialTiles()` populates `m_teleporterPositions`, but other code may read from `m_teleportPositions` (or vice versa), causing invisible teleporters.
**Fix:** Pick one name (e.g., `m_teleporterPositions`), remove the other, and update all references.

---

## 16. `PartyHP` Initialization is Wrong
**File:** `GameStateManager.cpp:1313`
**Problem:** `m_gameStateData["PartyHP"] = QVariantList({50, 40, 30})` has only 3 values for a 4-member party, and the values (50, 40, 30) don't match the HP=0 set for each "Empty Slot" character.
**Fix:** Either remove this line (since HP is already stored per-character in the Party struct) or generate it from the actual party data:
```cpp
QVariantList hpList;
for (const auto& c : m_currentParty.members) hpList.append(c.hp);
m_gameStateData["PartyHP"] = hpList;
```

---

## 17. Header Guard Mismatch
**File:** `blacklands.h:1, 64`
**Problem:** Uses `#ifndef GAMEMENU_H` / `#define GAMEMENU_H` but the file is `blacklands.h`. This won't cause a bug today, but if you ever create a file that also uses `GAMEMENU_H`, you'll get silent include conflicts.
**Fix:** Rename to `#ifndef BLACKLANDS_H` / `#define BLACKLANDS_H` / `#endif // BLACKLANDS_H`.

---

## 18. Filename Case Mismatch
**Files:** `blacklands.pro:120` vs actual file `fontManager.cpp`
**Problem:** The `.pro` file references `FontManager.cpp` (capital F) but the file on disk is `fontManager.cpp` (lowercase f). This works on case-insensitive systems but will break on standard Linux filesystems.
**Fix:** Rename the file to `FontManager.cpp` to match the convention used by the header (`FontManager.h`), or update the `.pro` file to `fontManager.cpp`. Consistency with the header name is preferred.

---

## 19. `INCLUDEPATH` Typo in `.pro`
**File:** `blacklands.pro:28`
**Problem:** `INCLUDEPATH += _PRO_FILE_PWD_/include` is missing the `$$` prefix. It's adding the literal string `_PRO_FILE_PWD_/include` instead of the project directory.
**Fix:**
```
INCLUDEPATH += $$_PRO_FILE_PWD_/include
```

---

## 20. Build Artifacts Committed to Git ( FIXED )
**Problem:** The repository contains generated/binary files: the `blacklands` executable, `.venv/`, `.cache/`, and `qrc_resources.cpp`. These bloat the repo and cause merge conflicts.
**Fix:** Add to `.gitignore`:
```
blacklands
.venv/
.cache/
qrc_resources.cpp
build/
```
Then remove them from tracking: `git rm --cached blacklands qrc_resources.cpp` and `git rm -r --cached .venv .cache`.

---

## 21. Massive Commented-Out Dead Code
**File:** Primarily `GameStateManager.cpp`
**Problem:** Hundreds of lines of commented-out `m_PC`-era code throughout the file (e.g., old `refreshUI`, old `loadCharacterFromFile`, old `setCharacterInventory`, etc.). This makes the file hard to read and maintain.
**Fix:** Delete all commented-out old implementations. They're preserved in git history if ever needed. This is a cleanup pass best done after the functional bugs above are fixed.

---

*Generated: 2026-03-12*
