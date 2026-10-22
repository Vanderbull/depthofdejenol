
#include "src/character_dialog/CharacterDialog.h"
#include "src/core/GameSettings.h"
#include "DungeonHandlers.h"
#include "DungeonDialog.h"
#include "src/core/BossEncounter.h"
#include "src/core/gameStateManager.h"
#include "src/items/ItemDatabase.h"
#include "src/core/DungeonThemes.h"
#include "src/core/DungeonLevelState.h"
#include "src/core/MonsterBalance.h"
#include "src/core/BossEncounter.h"
#include "src/core/QuestChainDriver.h"
#include "src/core/DoorAndSearch.h"
#include "src/traps_calculations.h"
#include "src/core/SoundEffects.h"
#include "../event/EventManager.h"
#include "src/spell_casting/SpellCastingDialog.h"
#include "src/quest_board/QuestBoardDialog.h"
#include "src/combat/CombatController.h"
#include "src/info_dialog/InfoPanelDialog.h"
#include "src/toolbar/GameToolbar.h"

#include "src/core/Endgame.h"
#include "src/core/DeathRecovery.h"
#include "src/journal_dialog/JournalDialog.h"
#include "src/victory_dialog/VictoryDialog.h"
#include <cmath>
#include <functional>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QKeyEvent>
#include <QTimer>
#include <QtDebug>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QGraphicsPolygonItem>
#include <QScreen>
#include <QGuiApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QInputDialog>


// Constants that depend on MAP_SIZE (which is now in the header)
const int TILE_SIZE = 10;
const int MAP_WIDTH_PIXELS = MAP_SIZE * TILE_SIZE;
const int MAP_HEIGHT_PIXELS = MAP_SIZE * TILE_SIZE;
const int MAP_MIN = 0;
const int MAP_MAX = MAP_SIZE - 1;

QPushButton* DungeonDialog::createButton(const QString& text, std::function<void()> slot) {
    QPushButton* button = new QPushButton(text, this);
    button->setFocusPolicy(Qt::NoFocus); // Keeps focus on the main window for movement
    connect(button, &QPushButton::clicked, this, std::move(slot));
    return button;
}

void DungeonDialog::setupControls() {
    // Define the buttons and their corresponding slots
    QMap<QString, std::function<void()>> buttonConfigs = {
        {"Fight",   [this]() { on_fightButton_clicked(); }},
        {"Eat",     [this]() { on_eatButton_clicked(); }},
        {"Spell",   [this]() { on_spellButton_clicked(); }},
        {"Rest",    [this]() { on_restButton_clicked(); }},
        {"Talk",    [this]() { on_talkButton_clicked(); }},
        {"Search",  [this]() { on_searchButton_clicked(); }},
        {"Disarm",  [this]() { on_disarmButton_clicked(); }},
        {"Pickup",  [this]() { on_pickupButton_clicked(); }},
        {"Drop",    [this]() { on_dropButton_clicked(); }},
        {"Open",    [this]() { on_openButton_clicked(); }},
        {"Chest",   [this]() { on_chestButton_clicked(); }},
        {"Teleport",[this]() { on_teleportButton_clicked(); }},

        {"Stairs Up",    [this]() { on_stairsUpButton_clicked(); }},
        {"Stairs Down",    [this]() { on_stairsDownButton_clicked(); }},
    };


    // Loop through and create them automatically
    for (auto it = buttonConfigs.begin(); it != buttonConfigs.end(); ++it) {
        QPushButton* btn = createButton(it.key(), it.value());
        m_controls.insert(it.key(), btn);
    }

    // Assign member pointers for the buttons that have them
    if (m_controls.contains("Fight"))        m_fightButton = m_controls["Fight"];
    if (m_controls.contains("Spell"))        m_spellButton = m_controls["Spell"];
    if (m_controls.contains("Rest"))         m_restButton = m_controls["Rest"];
    if (m_controls.contains("Talk"))         m_talkButton = m_controls["Talk"];
    if (m_controls.contains("Search"))       m_searchButton = m_controls["Search"];
    if (m_controls.contains("Pickup"))       m_pickupButton = m_controls["Pickup"];
    if (m_controls.contains("Drop"))         m_dropButton = m_controls["Drop"];
    if (m_controls.contains("Open"))         m_openButton = m_controls["Open"];
    if (m_controls.contains("Chest"))        m_chestButton = m_controls["Chest"];

    if (m_controls.contains("Teleport"))     m_teleportButton = m_controls["Teleport"];
    if (m_controls.contains("Stairs Up"))    m_stairsUpButton = m_controls["Stairs Up"];
    if (m_controls.contains("Stairs Down"))  m_stairsDownButton = m_controls["Stairs Down"];
    if (m_controls.contains("Eat"))          m_eatButton = m_controls["Eat"];
    if (m_controls.contains("Take"))         m_takeButton = m_controls["Take"];
    if (m_controls.contains("Disarm"))       m_disarmButton = m_controls["Disarm"];
}


void DungeonDialog::populateRandomTreasures(int level)
{
    gameStateManager* gsm = gameStateManager::instance();
    // MDATA3 is loaded into m_itemData in gameStateManager
    const QList<QVariantMap>& allItems = gsm->itemData();
    if (allItems.isEmpty()) {
        qDebug() << "MDATA3 not loaded or empty.";
        return;
    }
    QRandomGenerator* rng = QRandomGenerator::global();
    int itemsPlaced = 0;
    int chestCount = 0;
    while (itemsPlaced < 100) {
        // 1. Pick a random coordinate
        int x = rng->bounded(MAP_SIZE);
        int y = rng->bounded(MAP_SIZE);
        QPair<int, int> pos = {x, y};
        // 2. Ensure it is a floor tile (not a wall/obstacle) and not already a treasure
        if (!m_obstaclePositions.contains(pos) && !m_treasurePositions.contains(pos)) {
            // 3. Pick a random item from the MDATA3 list
            int itemIdx = rng->bounded(allItems.size());
            QString itemName = allItems.at(itemIdx).value("name").toString();
            // 4. Add to local level map (for rendering/interaction)
            m_treasurePositions.insert(pos, itemName);
            // 5. Add to the Global Array in gameStateManager
            gsm->addPlacedItem(level, x, y, itemName);
            // 6. 15% of chests are locked and need a specific key.
            if (itemName.contains("Chest") && chestCount < 15) {
                m_lockedChests.insert(pos);
                // Find a key item in the database (e.g. "Copper Key", "Iron Key").
                QString keyName;
                for (const QVariantMap& item : allItems) {
                    QString name = item.value("name").toString();
                    if (name.contains("Key")) {
                        keyName = name;
                        break;
                    }
                }
                if (!keyName.isEmpty()) {
                    m_chestKeys.insert(pos, keyName);
                    chestCount++;
                }
            }
            itemsPlaced++;
        }
    }
    qDebug() << "Successfully placed 10 random items from MDATA3 on level" << level;
}

void DungeonDialog::revealAroundPlayer(int x, int y, int z=0)
{
    Q_UNUSED(z);
    // Light radius depends on whether the torch is burning.
    int radius = torchLit() ? 2 : 1;
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dy = -radius; dy <= radius; ++dy) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < MAP_SIZE && ny >= 0 && ny < MAP_SIZE) {
                m_visitedTiles.insert({nx, ny});
            }
        }
    }
}

// --- Phase 4: Torch / Light ---

bool DungeonDialog::torchLit() const
{
    return m_torchFuel > 0;
}

int DungeonDialog::torchFuel() const
{
    return m_torchFuel;
}

void DungeonDialog::updateTorchState()
{
    if (m_torchFuel > 0) {
        m_torchFuel--;
        if (m_torchFuel == 0) {
            logMessage("<font color='orange'>Your torch sputters and goes out. You are in darkness.</font>");
        } else if (m_torchFuel == 5) {
            logMessage("<font color='orange'>Your torch is burning low.</font>");
        }
    }
    // Persist to the snapshot so fuel survives leaving and returning.
    DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();
    int level = gameStateManager::instance()->getGameValue("DungeonLevel").toInt();
    if (registry.hasLevel(level)) {
        registry.levelForEdit(level).torchTurnsRemaining = m_torchFuel;
    }
}

void DungeonDialog::restForTorch()
{
    m_torchFuel = DEFAULT_TORCH_FUEL;
    logMessage("You stop to rest and relight your torch. The flames steady.");
    DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();
    int level = gameStateManager::instance()->getGameValue("DungeonLevel").toInt();
    if (registry.hasLevel(level)) {
        registry.levelForEdit(level).torchTurnsRemaining = m_torchFuel;
    }
}

void DungeonDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    // Defer until the child views have been laid out at the new size, then
    // scale the first-person view to fill the viewport.
    QTimer::singleShot(0, this, [this]() { fitViewport(); });
}

void DungeonDialog::fitViewport()
{
    if (m_graphicsView && m_dungeonScene) {
        m_graphicsView->fitInView(m_dungeonScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void DungeonDialog::logMessage(const QString& message)
{
    if (m_messageLog) {
        // Create a standard list item
        QListWidgetItem* item = new QListWidgetItem(m_messageLog);

        // Create a QLabel to hold the HTML content
        QLabel* label = new QLabel(message);
        label->setStyleSheet("background: transparent;"); // Keep log background

        // Set the label as the widget for this item
        m_messageLog->addItem(item);
        m_messageLog->setItemWidget(item, label);

        // Auto-scroll to the bottom
        m_messageLog->scrollToBottom();
    }
}

void DungeonDialog::updateGoldLabel()
{
    // Party gold is the single source of truth for on-hand gold.
    int partyGold = gameStateManager::instance()->getPartyGold();
    if (m_goldLabel) {
        m_goldLabel->setText(QString("You have a total of **%1 Gold**.").arg(partyGold));
    }
}
// --- Party Management Helper Function ---
void DungeonDialog::updatePartyMemberHealth(int row, int damage)
{
    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();

    // Defense is handled in combat via CombatActions::defend().
    int finalDamage = damage;

    if (row >= 0 && row < members.size()) {
        Character& c = members[row];
        c.hp = qMax(0, c.hp - finalDamage);

        if (c.hp <= 0 && c.isAlive) {
            DeathRecovery::killCharacter(c, gsm->getGameValue("DungeonLevel").toInt(),
                                         gsm->getGameValue("DungeonX").toInt(),
                                         gsm->getGameValue("DungeonY").toInt());
            JournalDialog::addEntry(QStringLiteral("Combat"),
                QStringLiteral("%1 was defeated on floor %2 (%3,%4).")
                    .arg(c.name).arg(gsm->getGameValue("DungeonLevel").toInt())
                    .arg(gsm->getGameValue("DungeonX").toInt())
                    .arg(gsm->getGameValue("DungeonY").toInt()));
            logMessage("You have been defeated!");
            if (DeathRecovery::isPartyWiped(members)) {
                handlePartyWipe();
                return;
            }
            if (row == 0) {
                gsm->setGameValue("isAlive", 0);
                this->close();
                emit exitedDungeonToCity();
            }
        }
        updatePartyPanel();
    }
}
// --- Dungeon Management (Movement, Drawing, Encounters) ---
void DungeonDialog::movePlayer(int dx, int dy, int dz=0)
{
    Q_UNUSED(dz);
    gameStateManager* gsm = gameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    int currentZ = gsm->getGameValue("DungeonLevel").toInt();
    int newX = currentX + dx;
    int newY = currentY + dy;
    if (newX < MAP_MIN || newX > MAP_MAX || newY < MAP_MIN || newY > MAP_MAX) {
        logMessage("You hit the dungeon wall.");
        return;
    }
    QPair<int, int> newPos = {newX, newY};

    // Levitating party members can fly over obstacles
    bool partyLevitating = false;
    {
        auto& members = gsm->getPartyMembers();
        for (int i = 0; i < members.size(); ++i) {
            if (members[i].levitationTurns > 0) {
                partyLevitating = true;
                break;
            }
        }
    }

    if (!partyLevitating && m_obstaclePositions.contains(newPos)) {
        logMessage("A solid rock wall blocks your path.");
        return;
    }
    // Door collision: a locked door blocks movement; an undiscovered secret
    // door is indistinguishable from a wall until found.
    auto doorIt = m_hiddenDoorPositions.find(newPos);
    if (doorIt != m_hiddenDoorPositions.end()) {
        const DoorState& door = doorIt.value();
        if (door.secret && !m_visitedTiles.contains(newPos)) {
            logMessage("A solid wall blocks your path.");
            return;
        }
        if (door.locked) {
            // Check if the party holds the key.
            bool hasKey = false;
            auto& members = gsm->getPartyMembers();
            for (const auto& member : members) {
                for (const auto& item : member.inventory) {
                    if (item.name == door.keyName) {
                        hasKey = true;
                        break;
                    }
                }
                if (hasKey) break;
            }
            if (!hasKey) {
                logMessage(QString("The door is locked. It needs the %1.").arg(door.keyName));
                return;
            }
            // Unlock and pass through.
            doorIt.value().locked = false;
            logMessage(QString("The %1 turns in the lock; the door opens.").arg(door.keyName));
        }
    }
    // Record current position as a breadcrumb before moving
    m_breadcrumbPath.append({currentX, currentY});
    if (m_breadcrumbPath.size() > MAX_BREADCRUMBS) {
        m_breadcrumbPath.removeFirst(); // Keep the trail from getting too long
    }
    gsm->setGameValue("DungeonX", newX);
    gsm->setGameValue("DungeonY", newY);

    // Hunger: each step costs 1 hunger. At 0, starvation deals 1 damage per step.
    // A starvation death is a real death: route it through DeathRecovery so the
    // body location is recorded and a journal entry is written, exactly like a
    // combat death. (Previously this called setDead(), which left the body at
    // (0,0,0) and wrote no journal entry — the "starving did not kill" bug.)
    auto& members = gsm->getPartyMembers();
    const int level = gsm->getGameValue("DungeonLevel").toInt();
    const int posX = gsm->getGameValue("DungeonX").toInt();
    const int posY = gsm->getGameValue("DungeonY").toInt();
    for (int i = 0; i < members.size(); ++i) {
        if (!members[i].isAlive) continue;
        members[i].consumeHunger(1);
        if (members[i].isStarving()) {
            members[i].hp = qMax(0, members[i].hp - 1);
            if (members[i].hp == 0) {
                DeathRecovery::killCharacter(members[i], level, posX, posY);
                logMessage(QString("<font color='red'>%1 has starved to death!</font>").arg(members[i].name));
                JournalDialog::addEntry(QStringLiteral("Combat"),
                    QStringLiteral("%1 starved to death on floor %2 (%3,%4).")
                        .arg(members[i].name).arg(level).arg(posX).arg(posY));
            } else {
                logMessage(QString("<font color='orange'>%1 is starving!</font>").arg(members[i].name));
            }
        }
    }
    // If the whole party starved, the run is over — handle it like a combat wipe.
    if (DeathRecovery::isPartyWiped(members)) {
        handlePartyWipe();
        return;
    }
    // Announce the first step into the low-hunger band (once per crossing).
    announceHungerWarnings();
    revealAroundPlayer(newX, newY);
    updateLocation(QString("Dungeon Level %1, (%2, %3)").arg(currentZ).arg(newX).arg(newY));
    m_visitedTiles.insert({newX, newY});
    updateMinimap(newX, newY, 0);
    DungeonHandlers::handleTreasure(this, newX, newY); // Now only logs chest presence
    DungeonHandlers::handleWater(this, newX, newY);
    DungeonHandlers::handleAntimagic(this, newX, newY);
    DungeonHandlers::handleTrap(this, newX, newY);
    DungeonHandlers::handleChute(this, newX, newY);
    DungeonHandlers::handleExtinguisher(this, newX, newY);
    DungeonHandlers::handleFog(this, newX, newY);
    DungeonHandlers::handleRotator(this, newX, newY);
    DungeonHandlers::handleEncounters(this, newX, newY);
    DungeonHandlers::handlePit(this, newX, newY); // Added pit check
    DungeonHandlers::handleTeleporter(this, newX, newY);
    if (m_fogTimer > 0) --m_fogTimer;
    updateTorchState();
    logMessage(QString("You move to (%1, %2).").arg(newX).arg(newY));

    // Describe the area when stepping into a new room or corridor.
    if (m_roomFloorTiles.contains(newPos) && !m_visitedTiles.contains(newPos)) {
        FloorTheme theme = DungeonThemes::forLevel(currentZ);
        logMessage(QString("<font color='gray'>%1</font>").arg(theme.description));
    }
    // Monsters wander after the player moves.
    moveMonsters();

    drawMinimap();
    renderWireframeView();
    updatePartyPanel();
}

int DungeonDialog::distanceBetween(QPair<int, int> a, QPair<int, int> b)
{
    int dx = a.first - b.first;
    int dy = a.second - b.second;
    return static_cast<int>(std::sqrt(dx * dx + dy * dy));
}

bool DungeonDialog::hasLineOfSight(QPair<int, int> from, QPair<int, int> to)
{
    // Bresenham line algorithm — check each tile along the line for walls.
    int x0 = from.first, y0 = from.second;
    int x1 = to.first, y1 = to.second;
    int dx = qAbs(x1 - x0), dy = qAbs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 == x1 && y0 == y1) return true;
        if (m_obstaclePositions.contains(qMakePair(x0, y0))) return false;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

QVector<QPair<int, int>> DungeonDialog::getWalkableNeighbors(QPair<int, int> pos)
{
    QVector<QPair<int, int>> neighbors;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            QPair<int, int> n = {pos.first + dx, pos.second + dy};
            if (n.first < MAP_MIN || n.first > MAP_MAX ||
                n.second < MAP_MIN || n.second > MAP_MAX) continue;
            if (m_obstaclePositions.contains(n)) continue;
            neighbors.append(n);
        }
    }
    return neighbors;
}

void DungeonDialog::moveMonsters()
{
    gameStateManager* gsm = gameStateManager::instance();
    int px = gsm->getGameValue("DungeonX").toInt();
    int py = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> playerPos = qMakePair(px, py);

    QMap<QPair<int, int>, QString> newPositions;
    for (auto it = m_monsterPositions.begin(); it != m_monsterPositions.end(); ++it) {
        QPair<int, int> oldPos = it.key();
        QString name = it.value();

        // Chase AI: if the player is within aggro range (6 tiles) and there
        // is line of sight, move greedily toward the player.
        int dist = distanceBetween(oldPos, playerPos);
        bool chasing = (dist <= 6 && hasLineOfSight(oldPos, playerPos));

        if (chasing) {
            // Greedy step: pick the walkable neighbor closest to the player.
            QVector<QPair<int, int>> neighbors = getWalkableNeighbors(oldPos);
            QPair<int, int> best = oldPos;
            int bestDist = dist;
            for (const QPair<int, int>& n : neighbors) {
                if (n == playerPos) continue;
                if (m_monsterPositions.contains(n)) continue;
                if (newPositions.contains(n)) continue;
                int d = distanceBetween(n, playerPos);
                if (d < bestDist) {
                    bestDist = d;
                    best = n;
                }
            }
            if (best != oldPos) {
                newPositions.insert(best, name);
            }
        } else {
            // Wander AI: 25% chance to shuffle one tile in a random direction.
            if (QRandomGenerator::global()->bounded(4) != 0) continue;

            int dx = QRandomGenerator::global()->bounded(-1, 2);
            int dy = QRandomGenerator::global()->bounded(-1, 2);
            if (dx == 0 && dy == 0) continue;

            QPair<int, int> newPos = {oldPos.first + dx, oldPos.second + dy};

            // Bounds + wall check.
            if (newPos.first < MAP_MIN || newPos.first > MAP_MAX ||
                newPos.second < MAP_MIN || newPos.second > MAP_MAX) continue;
            if (m_obstaclePositions.contains(newPos)) continue;

            // Don't step onto the player or another monster (including ones already moved this tick).
            if (newPos == playerPos) continue;
            if (m_monsterPositions.contains(newPos)) continue;
            if (newPositions.contains(newPos)) continue;

            // Move is valid.
            newPositions.insert(newPos, name);
        }
    }

    // Apply moves: remove old, insert new.
    for (auto it = newPositions.constBegin(); it != newPositions.constEnd(); ++it) {
        // Find the old position by name (names are unique per floor).
        for (auto old = m_monsterPositions.begin(); old != m_monsterPositions.end(); ++old) {
            if (old.value() == it.value()) {
                m_monsterPositions.erase(old);
                break;
            }
        }
        m_monsterPositions.insert(it.key(), it.value());
    }
}

void DungeonDialog::updateCompass(const QString& direction)
{
    m_compassLabel->setText(QString("Facing %1").arg(direction));
}

void DungeonDialog::updateLocation(const QString& location)
{
    m_locationLabel->setText(location);
}

// BSP (Binary Space Partitioning) node for more natural cavern layouts
struct BspNode {
    int x, y, w, h;
    BspNode* left = nullptr;
    BspNode* right = nullptr;
    int roomX = 0, roomY = 0, roomW = 0, roomH = 0;
    bool hasRoom = false;
    ~BspNode() { delete left; delete right; }
};

void DungeonDialog::bspSplit(BspNode* node, QRandomGenerator& rng, int depth)
{
    const int MIN_SIZE = 8;
    if (depth <= 0 || (node->w < MIN_SIZE * 2 && node->h < MIN_SIZE * 2)) return;
    bool splitH = node->w < node->h ? true : (node->h < node->w ? false : rng.bounded(2) == 0);
    if (splitH) {
        int split = rng.bounded(MIN_SIZE, node->h - MIN_SIZE);
        node->left = new BspNode{node->x, node->y, node->w, split};
        node->right = new BspNode{node->x, node->y + split, node->w, node->h - split};
    } else {
        int split = rng.bounded(MIN_SIZE, node->w - MIN_SIZE);
        node->left = new BspNode{node->x, node->y, split, node->h};
        node->right = new BspNode{node->x + split, node->y, node->w - split, node->h};
    }
    bspSplit(node->left, rng, depth - 1);
    bspSplit(node->right, rng, depth - 1);
}

void DungeonDialog::bspPlaceRoom(BspNode* node, QRandomGenerator& rng)
{
    if (!node) return;
    if (node->left || node->right) {
        bspPlaceRoom(node->left, rng);
        bspPlaceRoom(node->right, rng);
        return;
    }
    // Place a room inside this leaf node with 1-tile padding
    int maxW = node->w - 2, maxH = node->h - 2;
    if (maxW < 3 || maxH < 3) return;
    node->roomW = rng.bounded(qMin(6, maxW), qMin(maxW, 10));
    node->roomH = rng.bounded(qMin(6, maxH), qMin(maxH, 10));
    node->roomX = node->x + rng.bounded(1, node->w - node->roomW - 1);
    node->roomY = node->y + rng.bounded(1, node->h - node->roomH - 1);
    node->hasRoom = true;
    for (int rx = node->roomX; rx < node->roomX + node->roomW; ++rx)
        for (int ry = node->roomY; ry < node->roomY + node->roomH; ++ry)
            m_obstaclePositions.remove({rx, ry});
    m_roomFloorTiles.insert(QPair<int, int>(node->roomX + node->roomW / 2, node->roomY + node->roomH / 2));
    // Store room for corridor connections
    m_roomCenters.append({node->roomX + node->roomW / 2, node->roomY + node->roomH / 2});
}

void DungeonDialog::bspConnect(BspNode* node, QRandomGenerator& rng)
{
    if (!node || !node->left || !node->right) return;
    bspConnect(node->left, rng);
    bspConnect(node->right, rng);
    // Connect left and right subtrees with an L-shaped corridor (2 tiles wide)
    QList<QPair<int, int>> leftCenters, rightCenters;
    std::function<void(BspNode*, QList<QPair<int,int>>&)> collect = [&](BspNode* n, QList<QPair<int,int>>& out) {
        if (!n) return;
        if (n->hasRoom) out.append({n->roomX + n->roomW/2, n->roomY + n->roomH/2});
        collect(n->left, out);
        collect(n->right, out);
    };
    collect(node->left, leftCenters);
    collect(node->right, rightCenters);
    if (leftCenters.isEmpty() || rightCenters.isEmpty()) return;
    // Pick a random pair to connect
    auto a = leftCenters[rng.bounded(leftCenters.size())];
    auto b = rightCenters[rng.bounded(rightCenters.size())];
    // L-shaped corridor: horizontal then vertical, 2 tiles wide
    int midX = (a.first + b.first) / 2;
    int x1 = qMin(a.first, midX), x2 = qMax(a.first, midX);
    for (int x = x1; x <= x2; ++x)
        for (int dy = 0; dy < 2; ++dy)
            m_obstaclePositions.remove({x, a.second + dy});
    int y1 = qMin(a.second, b.second), y2 = qMax(a.second, b.second);
    for (int y = y1; y <= y2; ++y)
        for (int dx = 0; dx < 2; ++dx)
            m_obstaclePositions.remove({midX + dx, y});
    // Also connect via vertical-first for more natural paths
    int midY = (a.second + b.second) / 2;
    int vy1 = qMin(a.second, midY), vy2 = qMax(a.second, midY);
    for (int y = vy1; y <= vy2; ++y)
        for (int dx = 0; dx < 2; ++dx)
            m_obstaclePositions.remove({a.first + dx, y});
    int vx1 = qMin(a.first, b.first), vx2 = qMax(a.first, b.first);
    for (int x = vx1; x <= vx2; ++x)
        for (int dy = 0; dy < 2; ++dy)
            m_obstaclePositions.remove({x, midY + dy});
}

// Zone types for thematic floor sections
enum class ZoneType { Cells, Caverns, Temple, Lair };

void DungeonDialog::assignZones(QRandomGenerator& rng)
{
    // Divide the map into 3-4 thematic zones based on position.
    // West = Cells (prison), Center = Caverns (natural), East = Temple (ritual).
    // Deeper floors add a Lair zone in the far corner.
    int zoneCount = (m_currentLevel >= 5) ? 4 : 3;
    m_zoneMap.clear();
    m_zoneMap.resize(MAP_SIZE, QVector<int>(MAP_SIZE, -1));
    // Simple zone assignment by x-coordinate bands
    int bandWidth = MAP_SIZE / zoneCount;
    for (int x = 0; x < MAP_SIZE; ++x) {
        int zone = qMin(x / bandWidth, zoneCount - 1);
        for (int y = 0; y < MAP_SIZE; ++y) {
            if (!m_obstaclePositions.contains({x, y}))
                m_zoneMap[x][y] = zone;
        }
    }
    // Store zone centers for monster/treasure placement
    m_zoneCenters.clear();
    for (int z = 0; z < zoneCount; ++z) {
        QList<QPair<int, int>> zoneTiles;
        for (int x = 0; x < MAP_SIZE; ++x)
            for (int y = 0; y < MAP_SIZE; ++y)
                if (m_zoneMap[x][y] == z)
                    zoneTiles.append({x, y});
        if (!zoneTiles.isEmpty()) {
            // Pick a central tile in this zone
            int midIdx = zoneTiles.size() / 2;
            m_zoneCenters.append(zoneTiles[midIdx]);
        }
    }
}

void DungeonDialog::generateMultiLevelRooms(QRandomGenerator& rng)
{
    // Add multi-level rooms: rooms with z-levels connected by stairs.
    // These are placed in the largest BSP rooms and add verticality.
    if (m_roomCenters.size() < 4) return; // Need enough rooms
    // Pick 1-2 rooms to be multi-level (deeper floors get more)
    int multiLevelCount = (m_currentLevel >= 8) ? 2 : 1;
    for (int i = 0; i < multiLevelCount && i < m_roomCenters.size(); ++i) {
        int idx = rng.bounded(m_roomCenters.size());
        QPair<int, int> center = m_roomCenters[idx];
        // Create a lower level (z=-1) room beneath this room
        int lowerW = rng.bounded(3, 6);
        int lowerH = rng.bounded(3, 6);
        int lowerX = center.first - lowerW / 2;
        int lowerY = center.second - lowerH / 2;
        // Carve the lower level (stored in m_lowerLevelTiles)
        for (int rx = lowerX; rx < lowerX + lowerW; ++rx)
            for (int ry = lowerY; ry < lowerY + lowerH; ++ry)
                if (rx > 0 && ry > 0 && rx < MAP_SIZE - 1 && ry < MAP_SIZE - 1)
                    m_lowerLevelTiles.insert(QPair<int, int>(rx, ry));
        // Place stairs down in the center of the upper room
        m_stairsDownPosition = center;
        // Place stairs up in the center of the lower room
        m_lowerStairsUpPosition = {lowerX + lowerW / 2, lowerY + lowerH / 2};
    }
}

void DungeonDialog::generateRandomObstacles(int roomCount, QRandomGenerator& rng)
{
    m_obstaclePositions.clear();
    m_roomFloorTiles.clear();
    m_roomCenters.clear();
    // 1. Fill entire map with rock
    for (int x = 0; x < MAP_SIZE; ++x)
        for (int y = 0; y < MAP_SIZE; ++y)
            m_obstaclePositions.insert({x, y});
    struct Room { int x, y, w, h; };
    QList<Room> rooms;
    // 2. BSP-based generation for more natural cavern layouts
    BspNode* root = new BspNode{1, 1, MAP_SIZE - 2, MAP_SIZE - 2};
    bspSplit(root, rng, 4);
    bspPlaceRoom(root, rng);
    bspConnect(root, rng);
    // 3. Add a few extra rooms via the sprouting method for variety
    QList<QPair<int, int>> sproutPoints = m_roomCenters;
    int extraRooms = roomCount / 3;
    int roomsCreated = m_roomCenters.size();
    while (!sproutPoints.isEmpty() && roomsCreated < roomCount) {
        int idx = rng.bounded(sproutPoints.size());
        QPair<int, int> current = sproutPoints.takeAt(idx);
        int corridorLen = rng.bounded(2, 5);
        int newW = rng.bounded(3, 6);
        int newH = rng.bounded(3, 6);
        int cX = current.first, cY = current.second;
        int endX = cX, endY = cY;
        int roomX = 0, roomY = 0;
        int dir = rng.bounded(4);
        if (dir == 0) { endY = cY - corridorLen; roomX = endX - newW / 2; roomY = endY - newH; }
        else if (dir == 1) { endX = cX + corridorLen; roomY = endY - newH / 2; roomX = endX; }
        else if (dir == 2) { endY = cY + corridorLen; roomX = endX - newW / 2; roomY = endY; }
        else { endX = cX - corridorLen; roomY = endY - newH / 2; roomX = endX - newW; }
        if (roomX > 0 && roomY > 0 && roomX + newW < MAP_MAX && roomY + newH < MAP_MAX) {
            bool overlaps = false;
            for (const auto& r : rooms) {
                if (roomX < r.x + r.w + 1 && roomX + newW > r.x - 1 &&
                    roomY < r.y + r.h + 1 && roomY + newH > r.y - 1) {
                    overlaps = true; break;
                }
            }
            if (!overlaps) {
                Room newRoom = {roomX, roomY, newW, newH};
                rooms.append(newRoom);
                int stepX = cX, stepY = cY;
                while (stepX != endX || stepY != endY) {
                    if (stepX < endX) stepX++; else if (stepX > endX) stepX--;
                    if (stepY < endY) stepY++; else if (stepY > endY) stepY--;
                    m_obstaclePositions.remove({stepX, stepY});
                }
                    // Carve Room
                    Room nextRoom = {roomX, roomY, newW, newH};
                    for (int rx = roomX; rx < roomX + newW; ++rx) {
                        for (int ry = roomY; ry < roomY + newH; ++ry) {
                            m_obstaclePositions.remove({rx, ry});
                            m_roomFloorTiles.insert(qMakePair(rx, ry));
                        }
                    }
                    rooms.append(nextRoom);
                    m_roomCenters.append({roomX + newW / 2, roomY + newH / 2});
                    roomsCreated++;
                }
            }
    }
    delete root;
    // Start player in the first room center
    if (!m_roomCenters.isEmpty()) {
        gameStateManager::instance()->setGameValue("DungeonX", m_roomCenters[0].first);
        gameStateManager::instance()->setGameValue("DungeonY", m_roomCenters[0].second);
    }
}

void DungeonDialog::generateStairs(QRandomGenerator& rng)
{
    gameStateManager* gsm = gameStateManager::instance();
    int currentX = gsm->getGameValue("DungeonX").toInt();
    int currentY = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> currentPos = {currentX, currentY};

    // Place stairs in room centers for guaranteed reachability.
    // Stairs Up goes in the start room (rooms[0]); Stairs Down goes in the
    // farthest room from the start — the natural "goal" of the floor.
    if (m_roomFloorTiles.isEmpty()) {
        // Fallback: random floor tile (should not happen on a generated map)
        do {
            m_stairsUpPosition = {rng.bounded(MAP_SIZE), rng.bounded(MAP_SIZE)};
        } while (m_stairsUpPosition == currentPos || m_obstaclePositions.contains(m_stairsUpPosition));
        do {
            m_stairsDownPosition = {rng.bounded(MAP_SIZE), rng.bounded(MAP_SIZE)};
        } while (m_stairsDownPosition == currentPos || m_stairsDownPosition == m_stairsUpPosition || m_obstaclePositions.contains(m_stairsDownPosition));
        m_obstaclePositions.remove(m_stairsUpPosition);
        m_obstaclePositions.remove(m_stairsDownPosition);
        return;
    }

    // Find the start room center (closest to currentPos)
    QPair<int, int> startRoomCenter = currentPos;
    int minDist = INT_MAX;
    for (const auto& tile : m_roomFloorTiles.values()) {
        int dx = tile.first - currentPos.first;
        int dy = tile.second - currentPos.second;
        int dist = dx * dx + dy * dy;
        if (dist < minDist) {
            minDist = dist;
            startRoomCenter = tile;
        }
    }
    m_stairsUpPosition = startRoomCenter;

    // Find the farthest room center from the start
    QPair<int, int> farthestRoomCenter = startRoomCenter;
    int maxDist = -1;
    for (const auto& tile : m_roomFloorTiles.values()) {
        int dx = tile.first - startRoomCenter.first;
        int dy = tile.second - startRoomCenter.second;
        int dist = dx * dx + dy * dy;
        if (dist > maxDist) {
            maxDist = dist;
            farthestRoomCenter = tile;
        }
    }
    m_stairsDownPosition = farthestRoomCenter;

    m_obstaclePositions.remove(m_stairsUpPosition);
    m_obstaclePositions.remove(m_stairsDownPosition);
}

void DungeonDialog::generateSpecialTiles(int tileCount, QRandomGenerator& rng,
                                          const FloorTheme* theme)
{
    m_bodyPositions.clear();
    // 1. Reset all containers
    m_antimagicPositions.clear();
    m_extinguisherPositions.clear();
    m_fogPositions.clear();
    m_pitPositions.clear();
    m_rotatorPositions.clear();
    m_studPositions.clear();
    m_chutePositions.clear();
    m_monsterPositions.clear();
    m_trapPositions.clear();
    m_waterPositions.clear();
    m_teleporterPositions.clear();
    m_hiddenDoorPositions.clear();
    gameStateManager* gsm = gameStateManager::instance();
int currentLevel = gsm->getGameValue("DungeonLevel").toInt();
    QPair<int, int> playerPos = {gsm->getGameValue("DungeonX").toInt(), gsm->getGameValue("DungeonY").toInt()};
    // Helper A: Get a tile ONLY from a room (No corridors!)
    auto getValidRoomTile = [&]() -> QPair<int, int> {
        if (m_roomFloorTiles.isEmpty()) return {-1, -1};
        QList<QPair<int, int>> roomList = m_roomFloorTiles.values();
        for (int i = 0; i < 100; ++i) {
            QPair<int, int> p = roomList.at(rng.bounded(roomList.size()));
            if (p != m_stairsUpPosition && p != m_stairsDownPosition && p != playerPos) return p;
        }
        return {-1, -1};
    };
    // Helper: Get ANY floor tile (Rooms OR Corridors)
    auto getAnyFloorTile = [&]() -> QPair<int, int> {
        for (int i = 0; i < 500; ++i) {
            int x = rng.bounded(MAP_SIZE);
            int y = rng.bounded(MAP_SIZE);
            QPair<int, int> p = {x, y};
            if (!m_obstaclePositions.contains(p) && p != playerPos &&
                p != m_stairsUpPosition && p != m_stairsDownPosition &&
                !m_treasurePositions.contains(p)) { // Also check if treasure is already there
                return p;
            }
        }
        return {-1, -1};
    };
    // 2. Guaranteed Room-Only Spawns (Chutes)
    for (int i = 0; i < 4; ++i) {
        QPair<int, int> cp = getValidRoomTile();
        if (cp.first != -1) m_chutePositions.insert(cp);
    }
    // 3. General Population Loop
    // Get thematic monsters for this floor's theme
    QStringList thematicMonsters = getThematicMonsters(currentLevel);
    // Tile mix comes from the floor's theme. With no theme we pass the flat
    // values the original layout used, so callers that don't know a theme keep
    // the old behaviour.
    FloorTheme flatTheme;
    flatTheme.waterChance = 25;
    flatTheme.pitChance = 5;
    flatTheme.antimagicChance = 10;
    flatTheme.fogChance = 0;
    const DungeonThemes::TileMix mix =
        DungeonThemes::tileMix(theme ? *theme : flatTheme);
    const int rollMax = qMax(1, mix.total);
    for (int i = 0; i < tileCount; ++i) {
        int roll = rng.bounded(rollMax); // percentage control
        QPair<int, int> pos;
        if (roll < mix.water) { // Water
            pos = getAnyFloorTile();
            if (pos.first != -1) m_waterPositions.insert(pos);
        }
        else if (roll < mix.antiMagic) { // Anti-magic / Extinguisher
            pos = getAnyFloorTile();
            if (pos.first != -1) m_antimagicPositions.insert(pos);
        }
        else if (roll < mix.fog) { // Fog
            pos = getAnyFloorTile();
            if (pos.first != -1) m_fogPositions.insert(pos);
        }
        else if (roll < mix.pit) { // Pits
            pos = getAnyFloorTile();
            if (pos.first != -1) m_pitPositions.insert(pos);
        }
        else if (roll < mix.monster) { // Monsters
            pos = getAnyFloorTile();
            if (pos.first != -1) {
                // Use thematic monster or fallback to "Orc"
                QString monsterName = thematicMonsters.isEmpty() ? "Orc"
                    : thematicMonsters.at(rng.bounded(thematicMonsters.size()));
                m_monsterPositions.insert(pos, monsterName);
            }
        }
        else if (roll < mix.treasure) { // Treasures
            pos = getAnyFloorTile();
            if (pos.first != -1) m_treasurePositions.insert(pos, "Gold Pouch");
        }
        else if (roll < mix.trap) { // Traps — placed with a type drawn from the floor's danger
            pos = getAnyFloorTile();
            if (pos.first != -1) {
                const char* trapTypes[] = {"Spike", "Poison Needler", "Dart", "Pit Cover", "Snare"};
                int idx = rng.bounded(5);
                m_trapPositions.insert(pos, trapTypes[idx]);
            }
        }
        // Extra Room-Only Chutes/Teleporters via random roll
        else {
            pos = getValidRoomTile();
            if (pos.first != -1) m_chutePositions.insert(pos);
        }
    }

    // Rotator and extinguisher tiles — placed in rooms only, like chutes.
    // Count scales with depth: deeper floors get more rotators/extinguishers.
    int rotatorCount = 2 + currentLevel / 5;
    for (int i = 0; i < rotatorCount; ++i) {
        QPair<int, int> rp = getValidRoomTile();
        if (rp.first != -1) m_rotatorPositions.insert(rp);
        QPair<int, int> ep = getValidRoomTile();
        if (ep.first != -1) m_extinguisherPositions.insert(ep);
    }

    // Stud tiles — structural supports in rooms. Count scales with depth.
    int studCount = 2 + currentLevel / 5;
    for (int i = 0; i < studCount; ++i) {
        QPair<int, int> sp = getValidRoomTile();
        if (sp.first != -1) m_studPositions.insert(sp);
    }

    // Teleporters — count scales with depth: deeper floors get more.
    int teleporterCount = 4 + currentLevel / 3;
    for (int i = 0; i < teleporterCount; ++i) {
        QPair<int, int> tp = getValidRoomTile();
        if (tp.first != -1) m_teleporterPositions.insert(tp);
    }

    // Boss floors: place the boss guarding the stairs down
    if (BossEncounter::hasBoss(currentLevel) &&
        !DungeonLevelRegistry::instance().levelForEdit(currentLevel).bossDefeated) {
        m_monsterPositions.insert(m_stairsDownPosition, BossEncounter::bossName(currentLevel));
        qDebug() << "Boss placed on floor" << currentLevel << "at" << m_stairsDownPosition;
    }

    if (currentLevel == 1) {
        QList<QPair<int, int>> wallList = m_obstaclePositions.values();
        bool placed = false;

        // Shuffle wall list to get a random wall tile for the door
        std::shuffle(wallList.begin(), wallList.end(), std::default_random_engine(rng.generate()));

        for (const auto& wallPos : wallList) {
            // Check 4 cardinal directions for a floor tile
            QList<QPair<int, int>> neighbors = {
                {wallPos.first, wallPos.second - 1}, // North
                {wallPos.first, wallPos.second + 1}, // South
                {wallPos.first - 1, wallPos.second}, // West
                {wallPos.first + 1, wallPos.second}  // East
            };

            for (const auto& neighbor : neighbors) {
                // Check boundaries and ensure neighbor is NOT a wall
                if (neighbor.first >= 0 && neighbor.first < MAP_SIZE &&
                    neighbor.second >= 0 && neighbor.second < MAP_SIZE &&
                    !m_obstaclePositions.contains(neighbor)) {

                    DoorState door;
                    door.position = wallPos;
                    door.secret = true;
                    door.locked = false;
                    door.difficulty = DoorAndSearch::secretDoorDifficulty(currentLevel);
                    m_hiddenDoorPositions.insert(wallPos, door);
                    qDebug() << "Accessible Hidden Door placed in wall at:" << wallPos << " next to floor at:" << neighbor;
                    placed = true;
                    break;
                }
            }
            if (placed) break;
        }
    }
}
// --- Constructor Implementation ---
DungeonDialog::DungeonDialog(QWidget *parent)
    : QDialog(parent),
      m_dungeonScene(new QGraphicsScene(this))
{
    gameStateManager* gsm = gameStateManager::instance();
    if (!gsm) {
        qCritical() << "CRITICAL: gameStateManager is NULL!";
        return;
    }
    m_experienceLabel = new QLabel(this);
    setupWindowAndState(gsm);
    setupMainLayout();
    setupSidebarContent();
    setupMapGeneration();
    setupActionButtons();
    setupSignalConnections();
    setupCombatSystem();

    // Create and configure the toolbar (slice 2: F-tangentens informationspaneler)
    m_toolbar = new GameToolbar(this);
    m_toolbar->setObjectName("GameToolbar");
    m_toolbar->setMode(GameToolbar::CommandPage);
    connect(m_toolbar, &GameToolbar::modeChanged, this, [this](GameToolbar::CommandMode mode) {
        qDebug() << "Toolbar mode changed to" << (mode == GameToolbar::CommandPage ? "Commands" : "Info");
    });
    // Add toolbar to the top of the main layout
    ((QVBoxLayout*)this->layout())->insertWidget(0, m_toolbar);

    // --- Finalize the viewport ---
    m_dungeonScene->setSceneRect(0, 0, 300, 300);
    enterLevel(m_initialLevel);
    drawMinimap();
    renderWireframeView();
    QTimer::singleShot(0, this, [this]() { fitViewport(); });
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();
}

void DungeonDialog::setupWindowAndState(gameStateManager* gsm)
{
    setWindowTitle("Dungeon: Depth of Dejenol");
    setMinimumSize(900, 640);
    resize(1280, 800);

    int initialLevel = gsm->getGameValue("DungeonLevel").toInt();
    int initialX = gsm->getGameValue("DungeonX").toInt();
    int initialY = gsm->getGameValue("DungeonY").toInt();
    if (initialLevel == 0) {
        initialLevel = 1;
        initialX = MAP_SIZE / 2;
        initialY = MAP_SIZE / 2;
        gsm->addPartyGold(1500);
        gsm->startPlayTime();
        gsm->setDungeonLevel(initialLevel);
        gsm->setGameValue("DungeonX", initialX);
        gsm->setGameValue("DungeonY", initialY);
    }
    m_initialLevel = initialLevel;
}

void DungeonDialog::setupMainLayout()
{
    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    setupControls();

    setStyleSheet(
        "QPushButton { min-width: 0px; max-width: 999px;"
        " min-height: 24px; padding: 4px 2px; font-size: 11px; }");

    QVBoxLayout *leftPanelLayout = new QVBoxLayout();
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);
    leftPanelLayout->setSpacing(6);

    m_graphicsView = new QGraphicsView(m_dungeonScene);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setFocusPolicy(Qt::NoFocus);
    m_graphicsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_graphicsView->setMinimumSize(320, 240);
    m_graphicsView->setObjectName("dungeonViewport");
    leftPanelLayout->addWidget(m_graphicsView, 4);

    QGroupBox *logBox = new QGroupBox("Adventure Log");
    QVBoxLayout *logLayout = new QVBoxLayout(logBox);
    m_messageLog = new QListWidget();
    m_messageLog->setFocusPolicy(Qt::NoFocus);
    logLayout->addWidget(m_messageLog);
    leftPanelLayout->addWidget(logBox, 1);

    rootLayout->addLayout(leftPanelLayout, 3);

    QWidget *rightPanel = new QWidget(this);
    rightPanel->setObjectName("dungeonSidebar");
    rightPanel->setFixedWidth(360);
    rightPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    QScrollArea *sidebarScroll = new QScrollArea(this);
    sidebarScroll->setObjectName("dungeonSidebarScroll");
    sidebarScroll->setWidget(rightPanel);
    sidebarScroll->setWidgetResizable(true);
    sidebarScroll->setFrameShape(QFrame::NoFrame);
    sidebarScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sidebarScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    sidebarScroll->setFixedWidth(360);
    rootLayout->addWidget(sidebarScroll, 0);

    QVBoxLayout *rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setSpacing(8);
}

void DungeonDialog::setupSidebarContent()
{
    QVBoxLayout *rightPanelLayout = findChild<QScrollArea*>("dungeonSidebarScroll")->widget()->findChild<QVBoxLayout*>();
    if (!rightPanelLayout) return;

    QGroupBox *infoBox = new QGroupBox("Current Location");
    QGridLayout *infoLayout = new QGridLayout(infoBox);
    m_locationLabel = new QLabel("Dungeon Level 1, (15, 15)");
    m_locationLabel->setStyleSheet("font-weight: bold;");
    infoLayout->addWidget(m_locationLabel, 0, 0, 1, 2);
    m_compassLabel = new QLabel("Facing North");
    infoLayout->addWidget(m_compassLabel, 1, 0);
    QString alignment = gameStateManager::instance()->getGameValue("CurrentCharacterAlignment").toString();
    if (alignment.isEmpty()) alignment = "Neutral";
    QLabel *alignmentLabel = new QLabel(QString("Alignment: **%1**").arg(alignment));
    infoLayout->addWidget(alignmentLabel, 1, 1);
    rightPanelLayout->addWidget(infoBox);
    updateExperienceLabel();
    rightPanelLayout->addWidget(m_experienceLabel);

    m_goldLabel = new QLabel();
    updateGoldLabel();
    rightPanelLayout->addWidget(m_goldLabel);

    QGroupBox* partyBox = new QGroupBox("Party Status");
    QVBoxLayout* partyBoxLayout = new QVBoxLayout(partyBox);
    m_partyStatusList = new QListWidget(partyBox);
    m_partyStatusList->setMaximumHeight(60);
    m_partyStatusList->setFocusPolicy(Qt::NoFocus);
    partyBoxLayout->addWidget(m_partyStatusList);
    rightPanelLayout->addWidget(partyBox);

    QGroupBox* miniMapBox = new QGroupBox("Automap");
    QVBoxLayout* miniMapBoxLayout = new QVBoxLayout(miniMapBox);
    m_miniMapViewIntegrated = new QGraphicsView(miniMapBox);
    m_miniMapViewIntegrated->setMinimumSize(100, 100);
    m_miniMapViewIntegrated->setFocusPolicy(Qt::NoFocus);
    m_miniMapViewIntegrated->setBackgroundRole(QPalette::Dark);
    miniMapBoxLayout->addWidget(m_miniMapViewIntegrated);
    rightPanelLayout->addWidget(miniMapBox);
    updatePartyPanel();
}

void DungeonDialog::setupMapGeneration()
{
    QRandomGenerator initialRng(1 + 12345);
    generateRandomObstacles(40, initialRng);
    generateStairs(initialRng);
    generateSpecialTiles(20, initialRng);
    drawMinimap();
}

void DungeonDialog::setupActionButtons()
{
    QVBoxLayout *rightPanelLayout = findChild<QScrollArea*>("dungeonSidebarScroll")->widget()->findChild<QVBoxLayout*>();
    if (!rightPanelLayout) return;

    QGridLayout *actionLayout = new QGridLayout();
    actionLayout->setSpacing(4);
    actionLayout->setContentsMargins(0, 0, 0, 0);

    QStringList actions = {"Fight", "Spell", "Rest", "Eat", "Talk", "Search", "Pickup", "Drop", "Open", "Map", "Chest", "Teleport", "Exit", "Stairs Up", "Stairs Down"};
    int row = 0, col = 0;
    for (const QString& name : actions) {
        if (m_controls.contains(name)) {
            QPushButton* btn = m_controls[name];
            btn->setMinimumSize(60, 26);
            actionLayout->addWidget(btn, row, col);
            if (++col > 2) { col = 0; row++; }
        }
    }
    actionLayout->setColumnStretch(0, 1);
    actionLayout->setColumnStretch(1, 1);
    actionLayout->setColumnStretch(2, 1);
    rightPanelLayout->addLayout(actionLayout);

    QGroupBox *moveBox = new QGroupBox("Movement");
    QGridLayout *moveLayout = new QGridLayout(moveBox);
    m_upButton = new QPushButton("^");
    m_downButton = new QPushButton("v");
    m_leftButton = new QPushButton("<");
    m_rightButton = new QPushButton(">");
    m_rotateLeftButton = new QPushButton("Rotate L");
    m_rotateRightButton = new QPushButton("Rotate R");
    m_backtrackButton = new QPushButton("Back");
    m_backtrackButton->setToolTip("Retrace your steps");
    moveLayout->addWidget(m_rotateLeftButton, 0, 0);
    moveLayout->addWidget(m_upButton, 0, 1);
    moveLayout->addWidget(m_rotateRightButton, 0, 2);
    moveLayout->addWidget(m_leftButton, 1, 0);
    moveLayout->addWidget(m_downButton, 1, 1);
    moveLayout->addWidget(m_rightButton, 1, 2);
    moveLayout->addWidget(m_backtrackButton, 2, 0, 1, 3);
    rightPanelLayout->addWidget(moveBox);

    m_stairsUpButton = new QPushButton("Stairs Up");
    m_stairsDownButton = new QPushButton("Stairs Down");
    m_stairsLowerButton = new QPushButton("Stairs Lower");
    m_stairsUpButton->setToolTip("Ascend to previous floor");
    m_stairsDownButton->setToolTip("Descend to next floor");
    m_stairsLowerButton->setToolTip("Descend to lower level (multi-level room)");
    QHBoxLayout *stairsLayout = new QHBoxLayout();
    stairsLayout->addWidget(m_stairsUpButton);
    stairsLayout->addWidget(m_stairsDownButton);
    stairsLayout->addWidget(m_stairsLowerButton);
    rightPanelLayout->addLayout(stairsLayout);
}

void DungeonDialog::setupSignalConnections()
{
    connect(m_upButton, &QPushButton::clicked, this, &DungeonDialog::moveForward);
    connect(m_downButton, &QPushButton::clicked, this, &DungeonDialog::moveBackward);
    connect(m_leftButton, &QPushButton::clicked, this, &DungeonDialog::moveStepLeft);
    connect(m_rightButton, &QPushButton::clicked, this, &DungeonDialog::moveStepRight);
    connect(m_rotateLeftButton, &QPushButton::clicked, this, &DungeonDialog::on_rotateLeftButton_clicked);
    connect(m_rotateRightButton, &QPushButton::clicked, this, &DungeonDialog::on_rotateRightButton_clicked);
    connect(m_backtrackButton, &QPushButton::clicked, this, &DungeonDialog::on_backtrackButton_clicked);

    connect(m_fightButton, &QPushButton::clicked, this, &DungeonDialog::on_fightButton_clicked);
    connect(m_spellButton, &QPushButton::clicked, this, &DungeonDialog::on_spellButton_clicked);
    connect(m_restButton, &QPushButton::clicked, this, &DungeonDialog::on_restButton_clicked);
    connect(m_talkButton, &QPushButton::clicked, this, &DungeonDialog::on_talkButton_clicked);
    connect(m_searchButton, &QPushButton::clicked, this, &DungeonDialog::on_searchButton_clicked);
    connect(m_pickupButton, &QPushButton::clicked, this, &DungeonDialog::on_pickupButton_clicked);
    connect(m_dropButton, &QPushButton::clicked, this, &DungeonDialog::on_dropButton_clicked);
    connect(m_openButton, &QPushButton::clicked, this, &DungeonDialog::on_openButton_clicked);
    connect(m_chestButton, &QPushButton::clicked, this, &DungeonDialog::on_chestButton_clicked);
    connect(m_teleportButton, &QPushButton::clicked, this, &DungeonDialog::on_teleportButton_clicked);

    connect(m_stairsDownButton, &QPushButton::clicked, this, &DungeonDialog::on_stairsDownButton_clicked);
    connect(m_stairsUpButton, &QPushButton::clicked, this, &DungeonDialog::on_stairsUpButton_clicked);
    connect(m_stairsLowerButton, &QPushButton::clicked, this, &DungeonDialog::on_stairsLowerButton_clicked);
}

void DungeonDialog::setupCombatSystem()
{
    QVBoxLayout *rightPanelLayout = findChild<QScrollArea*>("dungeonSidebarScroll")->widget()->findChild<QVBoxLayout*>();
    if (!rightPanelLayout) return;

    m_combatGroup = new QGroupBox("Combat", this);
    QVBoxLayout* combatLayout = new QVBoxLayout(m_combatGroup);

    m_combatMonsterHpLabel = new QLabel("Monster: -", m_combatGroup);
    m_combatMonsterHpLabel->setStyleSheet("font-weight: bold; color: red;");
    combatLayout->addWidget(m_combatMonsterHpLabel);

    m_combatTargetList = new QListWidget(m_combatGroup);
    m_combatFxTimer = new QTimer(this);
    m_combatFxTimer->setSingleShot(true);
    connect(m_combatFxTimer, &QTimer::timeout, this, [this]() {
        if (m_combatGroup) {
            m_combatGroup->setStyleSheet("");
        }
    });
    m_combatTargetList->setMaximumHeight(90);
    m_combatTargetList->setFocusPolicy(Qt::NoFocus);
    m_combatTargetList->setToolTip("Select the monster to attack, cast at, or use an item on.");
    combatLayout->addWidget(m_combatTargetList);

    m_combatPartyHpLabel = new QLabel("Party: -", m_combatGroup);
    m_combatPartyHpLabel->setStyleSheet("font-weight: bold; color: green;");
    combatLayout->addWidget(m_combatPartyHpLabel);

    QHBoxLayout* combatBtnLayout = new QHBoxLayout();
    m_combatAttackBtn = new QPushButton("Attack", m_combatGroup);
    m_combatDefendBtn = new QPushButton("Defend", m_combatGroup);
    m_combatFleeBtn = new QPushButton("Flee", m_combatGroup);
    m_combatUseItemBtn = new QPushButton("Item", m_combatGroup);
    m_combatSpellBtn = new QPushButton("Spell", m_combatGroup);
    m_combatRowBtn = new QPushButton("Row", m_combatGroup);
    m_combatRowBtn->setToolTip("Move the current character between the front and back row.\n"
                               "Melee only reaches the front row.");
    m_combatAttackBtn->setFocusPolicy(Qt::NoFocus);
    m_combatDefendBtn->setFocusPolicy(Qt::NoFocus);
    m_combatFleeBtn->setFocusPolicy(Qt::NoFocus);
    m_combatUseItemBtn->setFocusPolicy(Qt::NoFocus);
    m_combatSpellBtn->setFocusPolicy(Qt::NoFocus);
    m_combatRowBtn->setFocusPolicy(Qt::NoFocus);
    combatBtnLayout->addWidget(m_combatAttackBtn);
    combatBtnLayout->addWidget(m_combatDefendBtn);
    combatBtnLayout->addWidget(m_combatFleeBtn);
    combatBtnLayout->addWidget(m_combatUseItemBtn);
    combatBtnLayout->addWidget(m_combatSpellBtn);
    combatBtnLayout->addWidget(m_combatRowBtn);
    combatLayout->addLayout(combatBtnLayout);
    m_combatGroup->setVisible(false);
    rightPanelLayout->addWidget(m_combatGroup);

    // Create combat controller and wire it to the UI widgets
    m_combatController = new CombatController(this);
    m_combatController->setCombatWidgets(m_combatGroup, m_combatMonsterHpLabel,
                                         m_combatTargetList, m_combatPartyHpLabel,
                                         m_combatAttackBtn, m_combatDefendBtn,
                                         m_combatFleeBtn, m_combatUseItemBtn,
                                         m_combatSpellBtn, m_combatRowBtn,
                                         m_combatFxTimer);
    m_combatController->setLogCallback([this](const QString& msg) { logMessage(msg); });
    m_combatController->setRenderCallback([this]() { drawMinimap(); renderWireframeView(); });

    connect(m_combatAttackBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatAttackButton_clicked);
    connect(m_combatDefendBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatDefendButton_clicked);
    connect(m_combatFleeBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatFleeButton_clicked);
    connect(m_combatUseItemBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatUseItemButton_clicked);
    connect(m_combatSpellBtn, &QPushButton::clicked, this, &DungeonDialog::on_combatSpellButton_clicked);
    connect(m_combatRowBtn, &QPushButton::clicked, this, [this]() {
        if (!m_inCombat || !m_combatController) return;
        const int idx = m_combatController->turnEngine()->currentParticipantIndex();
        if (idx < 0) return;
        const CombatParticipant& p = m_combatController->combatState()->participant(idx);
        if (!p.isPlayer) return;
        m_combatController->togglePlayerRow(idx);
        logMessage(QString("%1 moves to the %2 row.")
                       .arg(p.name)
                       .arg(m_combatController->combatState()->participant(idx).row == CombatRow::Front
                                ? "front" : "back"));
        m_combatController->updateCombatUI();
    });
}

void DungeonDialog::updateExperienceLabel()
{
    // Retrieve Experience from gameStateManager
    // Assuming the key is "PlayerExperience"
    quint64 currentXP = gameStateManager::instance()->getGameValue("PlayerExperience").toULongLong();
    if (m_experienceLabel) {
        QString xpString = QStringLiteral("%L1").arg(currentXP);
        m_experienceLabel->setText(QString("Experience: **%1 XP**").arg(xpString));
    }
}

void DungeonDialog::enterLevel(int level, bool movingUp)
{
    m_breadcrumbPath.clear();
    m_visitedTiles.clear();
    // Clear treasures specifically at the start of level generation
    m_treasurePositions.clear();
    m_lockedChests.clear();
    m_chestKeys.clear();
    gameStateManager* gsm = gameStateManager::instance();

    // Theme drives the floor's population and difficulty.
    FloorTheme theme = DungeonThemes::forLevel(level);
    DungeonLevelRegistry& registry = DungeonLevelRegistry::instance();

    // Restoring a floor the party has already visited: keep it as they left it.
    if (registry.hasLevel(level)) {
        const LevelSnapshot* snap = registry.level(level);
        m_monsterPositions = snap->monsterPositions;
        m_treasurePositions = snap->treasurePositions;
        m_visitedTiles = snap->visitedTiles;
        m_stairsUpPosition = snap->stairsUp;
        m_stairsDownPosition = snap->stairsDown;
        m_trapPositions = snap->trapPositions;
        m_triggeredTraps = snap->triggeredTraps;
        m_openedChests = snap->openedChests;
        m_roomFloorTiles.clear();

        // Some of the cleared monsters have moved back in.
        QRandomGenerator respawnRng(static_cast<quint32>(level * 7919 + 13));
        registry.respawnMonsters(level, 0.3, theme.monsterCount, respawnRng,
                                 getThematicMonsters(level));
        m_monsterPositions = registry.level(level)->monsterPositions;

        QPair<int, int> landing = movingUp ? m_stairsDownPosition : m_stairsUpPosition;
        gsm->setDungeonLevel(level);
        gsm->setGameValue("DungeonX", landing.first);
        gsm->setGameValue("DungeonY", landing.second);
        revealAroundPlayer(landing.first, landing.second);
        m_visitedTiles.insert(landing);
        updateLocation(QString("Dungeon Level %1 — %2, (%3, %4)")
                       .arg(level).arg(theme.name).arg(landing.first).arg(landing.second));
        drawMinimap();
        logMessage(QString("You return to **%1** (Level %2).").arg(theme.name).arg(level));
        // Restore torch fuel from the snapshot.
        m_torchFuel = snap->torchTurnsRemaining;
        return;
    }

    // 1. Generate the map using Room-and-Corridor logic
    // Seed by level to ensure the layout is deterministic
    QRandomGenerator levelRng(level + 12345);
    populateRandomTreasures(theme.treasureCount);
    // We pass a 'Room Count' instead of 'Obstacle Count'.
    // Scale room count with depth: deeper floors are larger and more complex.
    int roomCount = qBound(20, 15 + level * 3, 40);
    generateRandomObstacles(roomCount, levelRng);
    // 2. Place Stairs and Special Tiles
    // These must be called AFTER generateRandomObstacles so they know where the floor is.
    generateStairs(levelRng);
    assignZones(levelRng);
    generateMultiLevelRooms(levelRng);
    // Scale the number of special tiles with the theme.
    generateSpecialTiles(theme.monsterCount + theme.trapCount, levelRng, &theme);

    // 3. Boss floors get their guardian placed on a room tile.
    if (theme.isBossFloor && !theme.bossName.isEmpty()) {
        QList<QPair<int, int>> roomTiles = m_roomFloorTiles.values();
        if (!roomTiles.isEmpty()) {
            QPair<int, int> bossPos = roomTiles.at(levelRng.bounded(roomTiles.size()));
            if (bossPos != m_stairsUpPosition && bossPos != m_stairsDownPosition) {
                m_monsterPositions.insert(bossPos, theme.bossName);
                logMessage(QString("<font color='red'>A dreadful presence stirs: %1!</font>")
                           .arg(theme.bossName));
            }
        }
    }

    // 4. Determine Landing Position
    // Arrive at the Down stairs if moving Up, or Up stairs if moving Down
    QPair<int, int> landingPos = movingUp ? m_stairsDownPosition : m_stairsUpPosition;

    // 5. Persist this floor so it survives leaving and returning.
    LevelSnapshot snap;
    snap.level = level;
    snap.generated = true;
    snap.monsterPositions = m_monsterPositions;
    snap.treasurePositions = m_treasurePositions;
    snap.visitedTiles = m_visitedTiles;
    snap.stairsUp = m_stairsUpPosition;
    snap.stairsDown = m_stairsDownPosition;
    snap.trapPositions = m_trapPositions;
    snap.triggeredTraps = m_triggeredTraps;
    snap.openedChests = m_openedChests;
    registry.store(snap);

    // 6. Update Game State and UI
    gsm->setDungeonLevel(level);
    gsm->setGameValue("DungeonX", landingPos.first);
    gsm->setGameValue("DungeonY", landingPos.second);
    revealAroundPlayer(landingPos.first, landingPos.second);
    m_visitedTiles.insert(landingPos);
    updateLocation(QString("Dungeon Level %1 — %2, (%3, %4)")
                   .arg(level).arg(theme.name).arg(landingPos.first).arg(landingPos.second));
    drawMinimap();
    logMessage(QString("You have entered **%1** (Dungeon Level %2).").arg(theme.name).arg(level));
    // Recommended level hint (v4.0.0 slice 4.5): tell the player how deep they
    // are relative to their party, so a wall is a warning and not a surprise.
    logMessage(QString("<font color='#8fa'>%1</font>")
               .arg(MonsterBalance::floorHint(level)));
    if (!theme.description.isEmpty()) {
        logMessage(QString("<font color='gray'>%1</font>").arg(theme.description));
    }
    // Light a fresh torch on a new floor.
    m_torchFuel = DEFAULT_TORCH_FUEL;
    DungeonLevelRegistry::instance().levelForEdit(level).torchTurnsRemaining = m_torchFuel;
}





void DungeonDialog::on_pickupButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> pos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };
    if (m_treasurePositions.contains(pos)) {
        processTreasureOpening();
    } else {
        logMessage("There is nothing here to pick up.");
    }
}

void DungeonDialog::on_dropButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();

    int x = gsm->getGameValue("DungeonX").toInt();
    int y = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> pos = {x, y};
    m_bodyPositions.insert(pos);

    // Check if the player is actually carrying a body
    // Assuming "IsCarryingBody" is a flag in your gameStateManager
    if (gsm->getGameValue("IsCarryingBody").toBool()) {

        // Get current position
        int curX = gsm->getGameValue("DungeonX").toInt();
        int curY = gsm->getGameValue("DungeonY").toInt();
        QPair<int, int> pos = {curX, curY};

        // Place the body on the tile the party is standing on, and stop
        // carrying it. m_bodyPositions is what renderWireframeView() reads, so
        // the body stays visible on this tile.
        m_bodyPositions.insert(pos);
        // Update GameState: No longer carrying, and place body on map
        gsm->setGameValue("IsCarryingBody", false);

        logMessage("<font color='gray'>You carefully lay the carried body onto the cold stone floor.</font>");

        // Refresh view to show the dropped object if applicable
        renderWireframeView();
    } else {
        logMessage("You aren't carrying anything to drop.");
    }
    drawMinimap();
}

void DungeonDialog::on_teleportButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();

    // Check if any party member has an active teleport effect
    bool hasTeleport = false;
    auto& members = gsm->getPartyMembers();
    for (int i = 0; i < members.size(); ++i) {
        if (members[i].teleportTurns > 0) {
            hasTeleport = true;
            break;
        }
    }

    if (!hasTeleport) {
        logMessage("No teleport effect is active.");
        return;
    }

    int newX, newY;
    QPair<int, int> newPos;
    // 1. Find a random valid location (not a wall)
    do {
        newX = QRandomGenerator::global()->bounded(MAP_SIZE);
        newY = QRandomGenerator::global()->bounded(MAP_SIZE);
        newPos = {newX, newY};
    } while (m_obstaclePositions.contains(newPos));
    // 2. Update the Game State
    gsm->setGameValue("DungeonX", newX);
    gsm->setGameValue("DungeonY", newY);
    // 3. Log the event to the user
    logMessage(QString("A mystical force teleports you to (%1, %2)!")
               .arg(newX).arg(newY));
    // 4. Update the UI
    updateLocation(QString("Dungeon Level %1, (%2, %3)")
                   .arg(gsm->getGameValue("DungeonLevel").toInt()).arg(newX).arg(newY));
    // Refresh the map and trigger encounter checks at the new location
    drawMinimap();
    DungeonHandlers::handleTreasure(this, newX, newY);
    DungeonHandlers::handleTrap(this, newX, newY);
    DungeonHandlers::handleEncounters(this, newX, newY);
    // Emit the signal defined in the header
    emit teleporterUsed();
}

void DungeonDialog::on_fightButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> pos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };

    if (!startCombatAt(pos)) {
        logMessage("There is nothing to fight here.");
    }
}

bool DungeonDialog::startCombatAt(const QPair<int, int>& pos)
{
    if (!m_monsterPositions.contains(pos)) return false;

    // Ensure combat controller exists (may not if setupCombatSystem wasn't called)
    if (!m_combatController) {
        m_combatController = new CombatController(this);
        m_combatController->setLogCallback([this](const QString& msg) { logMessage(msg); });
        m_combatController->setRenderCallback([this]() { drawMinimap(); renderWireframeView(); });
        m_combatController->setCombatWidgets(m_combatGroup, m_combatMonsterHpLabel,
                                             m_combatTargetList, m_combatPartyHpLabel,
                                             m_combatAttackBtn, m_combatDefendBtn,
                                             m_combatFleeBtn, m_combatUseItemBtn,
                                             m_combatSpellBtn, m_combatRowBtn,
                                             m_combatFxTimer);
    }
    m_combatController->setInCombat(true);

    gameStateManager* gsm = gameStateManager::instance();
    QString monsterName = m_monsterPositions.value(pos);
    int level = gsm->getGameValue("DungeonLevel").toInt();
    bool isBoss = BossEncounter::hasBoss(level) && monsterName == BossEncounter::bossName(level);

    m_combatMonsterName = monsterName;
    m_combatMonsterLevel = level;
    m_combatIsBoss = isBoss;
    m_inCombat = true;

    // Build combat encounter
    m_combatController->combatState()->clear();

    // Add party members
    auto& members = gsm->getPartyMembers();
    for (int i = 0; i < members.size(); ++i) {
        const Character& c = members[i];
        if (!c.isAlive || c.hp <= 0) continue;

        CombatParticipant p;
        p.name = c.name;
        p.isPlayer = true;
        p.isAlive = true;
        p.hp = c.hp;
        p.maxHp = c.maxHp;
        p.level = c.level;
        p.att = 5 + c.level + (c.effectiveStrength() - 10) / 2;
        p.def = 10 + (c.effectiveDexterity() - 10) / 2;
        p.speed = c.effectiveDexterity();
        p.dex = c.effectiveDexterity();
        p.mana = c.mana;
        p.maxMana = c.maxMana;
        // Resistances granted by Resist/Protection consumables; the damage
        // path reads them so the potion actually does something.
        for (int r = 0; r < 12; ++r) p.resistances[r] = c.resistances[r];

        // Default position: the first two members form the front line, the rest
        // stand behind them. The player can move anyone with the Row button.
        p.row = (i < 2) ? CombatRow::Front : CombatRow::Back;
        // A member holding a bow or casting is not modelled as ranged here;
        // player melee reaches the front line of the enemy, which is everyone
        // (monsters have no row to hide behind), so players stay non-ranged.

        // Add weapon bonus
        for (const HeldItem& item : c.equipped) {
            if (const ItemDef* def = ItemDatabase::instance().byName(item.name)) {
                if (def->slot() == ItemSlot::MainHand || def->slot() == ItemSlot::OffHand) {
                    p.att += def->att;
                }
                if (def->slot() == ItemSlot::Body || def->slot() == ItemSlot::Head ||
                    def->slot() == ItemSlot::OffHand) {
                    p.def += def->def;
                }
            }
        }

        m_combatController->combatState()->addParticipant(p);
    }

    // Add monsters
    if (isBoss) {
        QVariantMap bossData = BossEncounter::buildBoss(level);
        CombatParticipant boss;
        boss.name = monsterName;
        boss.isPlayer = false;
        boss.isAlive = true;
        boss.hp = bossData["hp"].toInt();
        boss.maxHp = boss.hp;
        boss.att = bossData["att"].toInt();
        boss.def = bossData["def"].toInt();
        boss.speed = bossData["speed"].toInt();
        boss.dex = bossData["dex"].toInt();
        boss.level = bossData["level"].toInt();
        boss.damageMod = bossData["damageMod"].toInt();
        boss.swings = bossData["swings"].toInt();
        m_combatController->combatState()->addParticipant(boss);
    } else {
        // Monster stats come from the game's own MDATA5 table (loaded by
        // gameStateManager): hits = HP, att/def, numGroups x ingroup = the
        // encounter size.
        QList<CombatParticipant> monsters = EncounterBuilder::buildEncounter(
            monsterName, gsm->getMonsterData(),
            gsm->getNgPlusLevel(),
            gsm->getGameValue("DungeonLevel").toInt(),
            DungeonThemes::forLevel(gsm->getGameValue("DungeonLevel").toInt()).monsterLevelBonus);
        for (const auto& m : monsters) {
            m_combatController->combatState()->addParticipant(m);
        }
    }

    // Start combat
    m_combatController->turnEngine()->startRound();
    m_combatGroup->setVisible(true);
    updateCombatUI();

    logMessage(QString("<font color='red'>⚔️ Combat begins with %1!</font>").arg(monsterName));

    // If monster goes first, take its turn
    if (m_combatController->turnEngine()->hasCurrentParticipant() && !m_combatController->combatActions()->isPlayerTurn()) {
        QString aiResult = m_combatController->monsterAI()->takeTurn();
        logMessage(aiResult);
        advanceCombat();
    }

    return true;
}

// --- Combat Action Slots ---
void DungeonDialog::on_combatAttackButton_clicked()
{
    if (!m_inCombat || !m_combatController) return;
    int targetIdx = m_combatController->selectedMonsterIndex();
    if (targetIdx < 0) targetIdx = m_combatController->selectFirstLivingMonster();
    if (targetIdx < 0) return;
    m_combatController->attack(targetIdx);
}

void DungeonDialog::on_combatDefendButton_clicked()
{
    if (!m_inCombat || !m_combatController) return;
    m_combatController->defend();
}

void DungeonDialog::on_combatFleeButton_clicked()
{
    if (!m_combatController) return;
    m_combatController->flee();
}

bool DungeonDialog::fleeCombat()
{
    if (!m_combatController) {
        m_combatController = new CombatController(this);
        m_combatController->setLogCallback([this](const QString& msg) { logMessage(msg); });
        m_combatController->setRenderCallback([this]() { drawMinimap(); renderWireframeView(); });
    }
    bool fled = m_combatController->flee();
    if (fled) {
        QPair<int, int> pos = getCurrentPosition();
        m_monsterPositions.remove(pos);
        renderWireframeView();
        drawMinimap();
        m_inCombat = false;
    }
    return fled;
}

void DungeonDialog::on_combatUseItemButton_clicked()
{
    if (!m_inCombat || !m_combatController) return;

    gameStateManager* gsm = gameStateManager::instance();
    Character& c = gsm->getPartyMember(gsm->getCurrentCharacterIndex());

    QStringList names;
    QList<int> slotIndices;
    for (int i = 0; i < c.inventory.size(); ++i) {
        if (!Character::isConsumable(c.inventory[i])) continue;
        slotIndices << i;
        const HeldItem& item = c.inventory[i];
        QString label = item.name;
        if (item.M4ED4 > 1) label += QString(" (x%1)").arg(item.M4ED4);
        names << label;
    }

    if (slotIndices.isEmpty()) {
        logMessage("No usable items in inventory.");
        return;
    }

    bool ok = false;
    const QString chosen = QInputDialog::getItem(this, "Use Item",
        "Choose an item to use:", names, 0, false, &ok);
    if (!ok || chosen.isEmpty()) return;

    const int listIdx = names.indexOf(chosen);
    if (listIdx < 0 || listIdx >= slotIndices.size()) return;

    m_combatController->useItem(slotIndices[listIdx]);
}

void DungeonDialog::on_combatSpellButton_clicked()
{
    if (!m_inCombat || !m_combatController) return;
    m_combatController->castSpell();
}

void DungeonDialog::updateCombatUI()
{
    if (m_combatController) m_combatController->updateCombatUI();
}

int DungeonDialog::selectedMonsterIndex() const
{
    if (!m_combatController) return -1;
    return m_combatController->selectedMonsterIndex();
}

bool DungeonDialog::togglePlayerRow(int participantIndex)
{
    if (!m_combatController) return false;
    return m_combatController->togglePlayerRow(participantIndex);
}

// v3.0.0 slice 3.8: combat visual feedback
bool DungeonDialog::combatFxEnabled()
{
    return CombatController::combatFxEnabled();
}

void DungeonDialog::flashCombatFx(bool damage)
{
    if (m_combatController) m_combatController->flashCombatFx(damage);
}

QString DungeonDialog::combatGroupStyleSheet() const
{
    return m_combatGroup ? m_combatGroup->styleSheet() : QString();
}

QString DungeonDialog::combatStatusText(const CombatParticipant& p)
{
    return CombatController::combatStatusText(p);
}

int DungeonDialog::selectFirstLivingMonster()
{
    if (!m_combatController) return -1;
    return m_combatController->selectFirstLivingMonster();
}

void DungeonDialog::advanceCombat()
{
    if (!m_inCombat || !m_combatController) return;
    m_combatController->advanceCombat();
}

void DungeonDialog::handleVictory()
{
    if (!m_combatController) return;
    m_combatController->handleVictory();
    m_inCombat = false;
    m_combatGroup->setVisible(false);
}

void DungeonDialog::handlePartyWipe()
{
    if (!m_combatController) return;
    m_combatController->handlePartyWipe();
    m_inCombat = false;
    m_combatGroup->setVisible(false);
    this->close();
    emit exitedDungeonToCity();
}

void DungeonDialog::syncCombatToGameState()
{
    if (!m_combatController) return;
    m_combatController->syncCombatToGameState();
    updatePartyPanel();
}

void DungeonDialog::updatePartyPanel()
{
    if (!m_partyStatusList) return;
    m_partyStatusList->clear();

    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();
    for (int i = 0; i < members.size(); ++i) {
        const Character& c = members[i];
        QListWidgetItem* item = new QListWidgetItem(partyMemberStatusText(c));
        item->setForeground(partyMemberColor(c));
        m_partyStatusList->addItem(item);
    }
}

// --- v5.0.0 slice 5.1: hunger is visible ---

QString DungeonDialog::partyMemberStatusText(const Character& c)
{
    QString status;
    if (!c.isAlive || c.hp <= 0) {
        status = "DEAD";
    } else {
        int pct = (c.maxHp > 0) ? (c.hp * 100 / c.maxHp) : 0;
        status = QString("%1/%2 HP (%3%)").arg(c.hp).arg(c.maxHp).arg(pct);
    }
    // Hunger rides alongside HP so the drain is visible before it kills.
    return QString("%1 (Lv %2) — %3 — Hunger %4/%5")
        .arg(c.name).arg(c.level).arg(status).arg(c.hunger).arg(c.maxHunger);
}

QColor DungeonDialog::partyMemberColor(const Character& c)
{
    if (!c.isAlive || c.hp <= 0 || c.isStarving()) {
        return QColor(Qt::red);
    }
    if (c.isHungerLow() || c.hp * 100 / qMax(1, c.maxHp) < 30) {
        return QColor(Qt::yellow);
    }
    return QColor(Qt::green);
}

QString DungeonDialog::hungerWarningMessage(const Character& c)
{
    if (!c.isHungerLow()) return QString();
    return QString("<font color='orange'>%1 is getting hungry.</font>").arg(c.name);
}

int DungeonDialog::announceHungerWarnings()
{
    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();

    // Drop warnings for members that no longer exist (party shrank).
    for (auto it = m_hungerWarned.begin(); it != m_hungerWarned.end(); ) {
        if (*it >= members.size()) it = m_hungerWarned.erase(it);
        else ++it;
    }

    int announced = 0;
    for (int i = 0; i < members.size(); ++i) {
        const Character& c = members[i];
        if (c.isHungerLow()) {
            // Warn once per crossing: only when this member was not already low.
            if (!m_hungerWarned.contains(i)) {
                logMessage(hungerWarningMessage(c));
                m_hungerWarned.insert(i);
                ++announced;
            }
        } else {
            // Back above the band: arm the warning for the next crossing.
            m_hungerWarned.remove(i);
        }
    }
    return announced;
}

// --- v5.0.0 slice 5.2: eating ---

int DungeonDialog::firstFoodIndex(const Character& c)
{
    const QList<int> idx = Character::foodIndices(c.inventory);
    return idx.isEmpty() ? -1 : idx.first();
}

int DungeonDialog::foodInventoryIndex(int memberIndex) const
{
    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();
    if (memberIndex < 0 || memberIndex >= members.size()) return -1;
    return firstFoodIndex(members[memberIndex]);
}

bool DungeonDialog::eatFromInventory()
{
    gameStateManager* gsm = gameStateManager::instance();
    const int idx = gsm->getCurrentCharacterIndex();
    auto& members = gsm->getPartyMembers();
    if (idx < 0 || idx >= members.size()) return false;

    Character& c = members[idx];
    const int foodSlot = firstFoodIndex(c);
    if (foodSlot < 0) {
        logMessage(QString("<font color='orange'>%1 has no food to eat.</font>").arg(c.name));
        return false;
    }

    QString effect;
    if (!c.eatFood(foodSlot, effect)) {
        logMessage(QString("<font color='red'>%1</font>").arg(effect));
        return false;
    }
    logMessage(QString("<font color='green'>%1</font>").arg(effect));
    updatePartyPanel();
    return true;
}

void DungeonDialog::on_eatButton_clicked()
{
    eatFromInventory();
}

void DungeonDialog::on_backtrackButton_clicked()
{
    if (m_breadcrumbPath.isEmpty()) {
        logMessage("You have no trail to follow back.");
        return;
    }

    // Pop the last breadcrumb and move there
    QPair<int, int> prev = m_breadcrumbPath.takeLast();
    gameStateManager* gsm = gameStateManager::instance();

    // Move directly to the previous position (bypassing direction logic)
    gsm->setGameValue("DungeonX", prev.first);
    gsm->setGameValue("DungeonY", prev.second);
    m_visitedTiles.insert(prev);
    revealAroundPlayer(prev.first, prev.second);
    updateLocation(QString("Dungeon Level %1, (%2, %3)")
                   .arg(gsm->getGameValue("DungeonLevel").toInt())
                   .arg(prev.first).arg(prev.second));
    logMessage(QString("You retrace your steps to (%1, %2).").arg(prev.first).arg(prev.second));

    // Trigger handlers at the new position
    DungeonHandlers::handleTreasure(this, prev.first, prev.second);
    DungeonHandlers::handleTrap(this, prev.first, prev.second);
    DungeonHandlers::handleEncounters(this, prev.first, prev.second);

    drawMinimap();
    renderWireframeView();
    updatePartyPanel();
}

void DungeonDialog::moveDiagonalForwardLeft()
{
    // Move forward then sidestep left
    QString facing = m_compassLabel->text();
    int dx = 0, dy = 0;
    if (facing == "Facing North") { dx = -1; dy = -1; }
    else if (facing == "Facing South") { dx = 1; dy = 1; }
    else if (facing == "Facing East") { dx = 1; dy = -1; }
    else if (facing == "Facing West") { dx = -1; dy = 1; }
    movePlayer(dx, dy);
}

void DungeonDialog::moveDiagonalForwardRight()
{
    QString facing = m_compassLabel->text();
    int dx = 0, dy = 0;
    if (facing == "Facing North") { dx = 1; dy = -1; }
    else if (facing == "Facing South") { dx = -1; dy = 1; }
    else if (facing == "Facing East") { dx = 1; dy = 1; }
    else if (facing == "Facing West") { dx = -1; dy = -1; }
    movePlayer(dx, dy);
}

// DungeonDialog.cpp
QPair<int, int> DungeonDialog::getCurrentPosition()
{
    gameStateManager* gsm = gameStateManager::instance();
    return qMakePair(gsm->getGameValue("DungeonX").toInt(),
                     gsm->getGameValue("DungeonY").toInt());
}

void DungeonDialog::on_takeButton_clicked()
{
    logMessage("You try to take something, but there is nothing there.");
}

void DungeonDialog::on_searchButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> currentPos = getCurrentPosition();
    int floorLevel = gsm->getGameValue("DungeonLevel").toInt();
    if (floorLevel < 1) floorLevel = 1;

    int wisdom = gsm->getGameValue("CurrentCharacterWisdom").toInt();
    int intelligence = gsm->getGameValue("CurrentCharacterIntelligence").toInt();

    // Hidden doors on this floor, as searchable DoorStates.
    QMap<QPair<int, int>, DoorState> doors;
    for (auto it = m_hiddenDoorPositions.constBegin(); it != m_hiddenDoorPositions.constEnd(); ++it) {
        DoorState d = it.value();
        d.position = it.key();
        d.secret = true;
        d.difficulty = DoorAndSearch::secretDoorDifficulty(floorLevel);
        doors.insert(it.key(), d);
    }

    QList<QPair<int, int>> found;
    QRandomGenerator rng(QRandomGenerator::global()->generate());
    int count = DoorAndSearch::searchForSecretDoors(doors, currentPos.first, currentPos.second,
                                                    wisdom, intelligence, found, rng);

    if (count > 0) {
        for (const QPair<int, int>& pos : found) {
            logMessage(QString("Your search reveals a hidden door at %1, %2!")
                       .arg(pos.first).arg(pos.second));
            SoundEffects::instance()->play(SoundEffects::Type::SecretDoor);
            // Add to visited tiles so it stays on the map.
            m_visitedTiles.insert(pos);
        }
        drawMinimap(); // Redraw map to show the discovered door
    }

    // Also search for traps in the same 3x3 area.
    QList<QPair<int, int>> foundTraps;
    int trapCount = DoorAndSearch::searchForTraps(m_trapPositions, currentPos.first, currentPos.second,
                                                   wisdom, intelligence, floorLevel, foundTraps, rng);
    if (trapCount > 0) {
        for (const QPair<int, int>& pos : foundTraps) {
            logMessage(QString("You notice a trap at %1, %2!")
                       .arg(pos.first).arg(pos.second));
            SoundEffects::instance()->play(SoundEffects::Type::SecretDoor);
            m_visitedTiles.insert(pos);
        }
        drawMinimap();
    } else if (count == 0) {
        logMessage("You search the area but find nothing hidden.");
    }
}
void DungeonDialog::on_talkButton_clicked()
{
    logMessage("You try talking, but the silence replies.");
}

void DungeonDialog::on_disarmButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> pos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };

    // Check if there is a trap at the player's position.
    if (!m_trapPositions.contains(pos)) {
        logMessage("There is no trap here to disarm.");
        return;
    }

    // Already triggered traps cannot be disarmed.
    if (m_triggeredTraps.contains(pos)) {
        logMessage("The trap has already been triggered.");
        return;
    }

    QString trapType = m_trapPositions.value(pos);
    int floorLevel = gsm->getGameValue("DungeonLevel").toInt();

    // Get character stats for disarm chance.
    auto& members = gsm->getPartyMembers();
    if (members.isEmpty()) {
        logMessage("No party member to disarm the trap.");
        return;
    }

    int dex = members[0].dexterity;
    int wis = members[0].wisdom;
    int gLvl = members[0].level;
    int mLvl = floorLevel;  // monster level = floor level
    int dLvl = floorLevel;  // dungeon level

    // Thief modifier: 1.0 for non-thieves, higher for thief/scavenger guilds.
    double thiefMod = 1.0;
    if (members[0].guildLevel("Thief") > 0 || members[0].guildLevel("Scavenger") > 0) {
        thiefMod = 1.5;
    }

    double chance = disarmTrapChance(dex, wis, gLvl, thiefMod, mLvl, dLvl);
    int roll = QRandomGenerator::global()->bounded(1, 101);

    if (roll <= static_cast<int>(chance)) {
        // Success! Remove the trap.
        m_triggeredTraps.insert(pos);
        logMessage(QString("<font color='green'>You successfully disarm the %1 trap!</font>").arg(trapType));
        SoundEffects::instance()->play(SoundEffects::Type::SecretDoor);
    } else {
        // Failure! The trap triggers.
        logMessage(QString("<font color='red'>You fail to disarm the %1 trap!</font>").arg(trapType));
        DungeonHandlers::handleTrap(this, pos.first, pos.second);
    }

    drawMinimap();
}

void DungeonDialog::on_chestButton_clicked()
{
    processTreasureOpening();
}







void DungeonDialog::keyPressEvent(QKeyEvent *event)
{
    // Add this to see if the event is even reaching the function
    qDebug() << "Key Pressed:" << event->key();
    switch (event->key()) {
        // --- Movement (WASD) ---
        case Qt::Key_Up:
            qDebug() << "up";
            moveForward();
            event->accept();
            break;
        case Qt::Key_Down:
            qDebug() << "down";
            moveBackward();
            event->accept();
            break;
        case Qt::Key_Left:
            qDebug() << "left";
            moveStepLeft();
            event->accept();
            break;
        case Qt::Key_Right:
            qDebug() << "right";
            moveStepRight();
            event->accept();
            break;
        // --- Rotation (QE) ---
        case Qt::Key_Q:
            on_rotateLeftButton_clicked();
            break;
        case Qt::Key_E:
            on_rotateRightButton_clicked();
            break;
        // --- Other Action Shortcuts ---
        case Qt::Key_T: {
            gameStateManager* gsm = gameStateManager::instance();
            QPair<int, int> currentPos = {
                gsm->getGameValue("DungeonX").toInt(),
                gsm->getGameValue("DungeonY").toInt()
            };

            if (currentPos == m_stairsUpPosition) {
                logMessage("Taking shortcut: Climbing up...");
                transitionLevel(StairDirection::Up);
            }
            else if (currentPos == m_stairsDownPosition) {
                logMessage("Taking shortcut: Descending down...");
                transitionLevel(StairDirection::Down);
            }
            else {
                logMessage("There are no stairs here to use.");
            }
            break;
        }
        case Qt::Key_F:
            on_fightButton_clicked();
            break;
        case Qt::Key_O:
            on_openButton_clicked();
            break;
        case Qt::Key_R:
            on_restButton_clicked();
            break;
        case Qt::Key_H: {
            // 'H' is Eat (Hunger). F stays Fight, as before.
            eatFromInventory();
            break;
        }
        case Qt::Key_S:
            on_spellButton_clicked();
            break;
        case Qt::Key_U:
            on_teleportButton_clicked();
            break;
        case Qt::Key_I:
        { // The opening brace starts a new local scope
            logMessage("Inventory Key Pressed");
            // Using 'new' ensures the dialog stays open after this function ends
            InventoryDialog *inventory = new InventoryDialog(this);
            inventory->setAttribute(Qt::WA_DeleteOnClose);
            inventory->show();
            inventory->raise();
            inventory->activateWindow();
            break; // Break MUST be inside or immediately after the case
        }
        // NEW: Character Sheet Hotkey
        case Qt::Key_C:
        {
            togglePartyInfo();
            break;
        }
        case Qt::Key_1: {
            logMessage("Opening Character Dialog...");
            CharacterDialog *charDialog = new CharacterDialog(this);
            charDialog->setAttribute(Qt::WA_DeleteOnClose);
            charDialog->show();
            charDialog->raise();
            charDialog->activateWindow();
            break;
        }
        case Qt::Key_D: {
            // Drop functionality (defend is now a combat button)
            on_dropButton_clicked();
            event->accept();
            break;
        }
        case Qt::Key_Escape:
            exitToMainMenu();
            event->accept();
            break;
        // --- Save/Load shortcuts ---
        case Qt::Key_F5: {
            // Quick save. The slot is the current save name when one is set
            // (so quicksave lands in the game's own file), otherwise the
            // dedicated "quicksave" slot. The old code read a "CurrentSaveName"
            // game value that nothing ever wrote, so it always fell back to
            // "quicksave" and the slot choice was dead.
            gameStateManager* gsm = gameStateManager::instance();
            QString saveName = gsm->getCurrentSaveName();
            if (saveName.isEmpty()) saveName = "quicksave";
            gsm->setCurrentSaveName(saveName);
            gsm->saveFullGameState(saveName);
            logMessage("Game saved to " + saveName);
            event->accept();
            break;
        }
        case Qt::Key_F9: {
            // Quick load — mirrors F5's slot choice.
            gameStateManager* gsm = gameStateManager::instance();
            QString saveName = gsm->getCurrentSaveName();
            if (saveName.isEmpty()) saveName = "quicksave";
            gsm->loadFullGameState(saveName);
            logMessage("Game loaded from " + saveName);
            event->accept();
            break;
        }
        case Qt::Key_F12: {
            // Quick save to autosave
            gameStateManager* gsm = gameStateManager::instance();
            gsm->setCurrentSaveName("autosave");
            gsm->saveFullGameState("autosave");
            logMessage("Game autosaved");
            event->accept();
            break;
        }
        case Qt::Key_F1: {
            InfoPanelDialog *panel = new InfoPanelDialog(this);
            panel->setAttribute(Qt::WA_DeleteOnClose);
            panel->show(); panel->raise(); panel->activateWindow();
            event->accept();
            break;
        }
        case Qt::Key_F2: {
            InfoPanelDialog *panel = new InfoPanelDialog(this);
            panel->setAttribute(Qt::WA_DeleteOnClose);
            panel->show(); panel->raise(); panel->activateWindow();
            event->accept();
            break;
        }
        case Qt::Key_F4: {
            InfoPanelDialog *panel = new InfoPanelDialog(this);
            panel->setAttribute(Qt::WA_DeleteOnClose);
            panel->show(); panel->raise(); panel->activateWindow();
            event->accept();
            break;
        }
        case Qt::Key_F6: {
            InfoPanelDialog *panel = new InfoPanelDialog(this);
            panel->setAttribute(Qt::WA_DeleteOnClose);
            panel->show(); panel->raise(); panel->activateWindow();
            event->accept();
            break;
        }
        case Qt::Key_F7: {
            InfoPanelDialog *panel = new InfoPanelDialog(this);
            panel->setAttribute(Qt::WA_DeleteOnClose);
            panel->show(); panel->raise(); panel->activateWindow();
            event->accept();
            break;
        }
        case Qt::Key_F8: {
            InfoPanelDialog *panel = new InfoPanelDialog(this);
            panel->setAttribute(Qt::WA_DeleteOnClose);
            panel->show(); panel->raise(); panel->activateWindow();
            event->accept();
            break;
        }
        case Qt::Key_F10: {
            InfoPanelDialog *panel = new InfoPanelDialog(this);
            panel->setAttribute(Qt::WA_DeleteOnClose);
            panel->show(); panel->raise(); panel->activateWindow();
            event->accept();
            break;
        }
        case Qt::Key_F11: {
            // Unused — reserved for future panels.
            event->accept();
            break;
        }
        default:
            QDialog::keyPressEvent(event);
            break;
    }
}


void DungeonDialog::on_restButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    auto& members = gsm->getPartyMembers();
    if (members.isEmpty()) return;
    Character& c = members[0];
    int healAmount = qMax(1, c.constitution / 2);
    int oldHp = c.hp;
    int newHp = qMin(c.maxHp, oldHp + healAmount);
    c.hp = newHp;
    logMessage(QString("You rest and recover %1 HP.").arg(newHp - oldHp));
    updatePartyPanel();
    // Resting also relights the torch.
    restForTorch();
    SoundEffects::instance()->play(SoundEffects::Type::Rest);
}

void DungeonDialog::on_stairsDownButton_clicked()
{
    transitionLevel(StairDirection::Down);
}

void DungeonDialog::on_stairsUpButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> currentPos = {
                gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };

    if (currentPos == m_stairsUpPosition) {
        logMessage("Taking shortcut: Climbing up...");
        transitionLevel(StairDirection::Up);
    }
}

void DungeonDialog::on_stairsLowerButton_clicked()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> currentPos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };
    if (currentPos == m_stairsDownPosition && !m_lowerLevelTiles.isEmpty()) {
        logMessage("Descending to lower level...");
        gsm->setGameValue("DungeonX", m_lowerStairsUpPosition.first);
        gsm->setGameValue("DungeonY", m_lowerStairsUpPosition.second);
        m_onLowerLevel = true;
        renderWireframeView();
        drawMinimap();
    } else if (m_onLowerLevel) {
        logMessage("Climbing back up...");
        gsm->setGameValue("DungeonX", m_stairsDownPosition.first);
        gsm->setGameValue("DungeonY", m_stairsDownPosition.second);
        m_onLowerLevel = false;
        renderWireframeView();
        drawMinimap();
    }
}

void DungeonDialog::on_openButton_clicked()
{
    processTreasureOpening();
}

void DungeonDialog::updateDungeonView(const QImage& dungeonImage)
{
    m_dungeonScene->clear();
    QPixmap pixmap = QPixmap::fromImage(dungeonImage);
    m_dungeonScene->addPixmap(pixmap);
    m_dungeonScene->setSceneRect(pixmap.rect());
    if (m_graphicsView && m_graphicsView->scene() == m_dungeonScene) {
        m_graphicsView->fitInView(m_dungeonScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void DungeonDialog::exitToMainMenu()
{
    logMessage("You leave the dungeon and return to the main menu.");
    emit exitedDungeonToMainMenu();
    this->close();
}

void DungeonDialog::on_rotateLeftButton_clicked()
{
    rotate(-1);
}

void DungeonDialog::on_rotateRightButton_clicked()
{
    rotate(1);
}

void DungeonDialog::moveForward()
{
    // Index 0: Forward, 1: Backward, 2: Left, 3: Right
    handleMovement(0);
}

void DungeonDialog::moveBackward()
{
    // Index 0: Forward, 1: Backward, 2: Left, 3: Right
    handleMovement(1);
}
void DungeonDialog::moveStepLeft()
{
    // Index 0: Forward, 1: Backward, 2: Left, 3: Right
    handleMovement(2);
}

void DungeonDialog::moveStepRight() {
    // Index 0: Forward, 1: Backward, 2: Left, 3: Right
    handleMovement(3);
}

void DungeonDialog::handleMovement(int actionIndex)
{
    // Map current facing to a list of vectors
    static const QMap<QString, QVector<QPoint>> moveMap = {
        {"Facing North", {QPoint(0,-1), QPoint(0,1),  QPoint(-1,0), QPoint(1,0)}}, // Right is (+1, 0)
        {"Facing South", {QPoint(0,1),  QPoint(0,-1), QPoint(1,0),  QPoint(-1,0)}}, // Right is (-1, 0)
        {"Facing East",  {QPoint(1,0),  QPoint(-1,0), QPoint(0,-1), QPoint(0,1)}},  // Right is (0, +1)
        {"Facing West",  {QPoint(-1,0), QPoint(1,0),  QPoint(0,1),  QPoint(0,-1)}}  // Right is (0, -1)
    };
    QString facing = m_compassLabel->text(); //
    if (moveMap.contains(facing)) {
        QPoint delta = moveMap[facing][actionIndex];
        movePlayer(delta.x(), delta.y()); //
    }
}

void DungeonDialog::rotate(int step)
{
    m_lastRotationStep = step;
    // List must match the layout of your compass logic
    static const QStringList dirs = {"North", "East", "South", "West"};
    // Find current index
    int currentIdx = dirs.indexOf(m_compassLabel->text().mid(7));
    // Calculate new index using modulo to wrap around
    // Adding 4 ensures the result is positive when turning left from North (index 0)
    int nextIdx = (currentIdx + step + 4) % 4;
    updateCompass(dirs[nextIdx]);
    logMessage(QString("You turn to the %1.").arg(step > 0 ? "right" : "left"));
    drawMinimap();
    renderWireframeView();
}

void DungeonDialog::transitionLevel(StairDirection direction)
{
    gameStateManager* gsm = gameStateManager::instance();
    // Retrieve current position once
    int currentZ = gsm->getGameValue("DungeonLevel").toInt();
    QPair<int, int> currentPos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };
    // Use the Enum to pick the correct target
    bool isGoingUp = (direction == StairDirection::Up);
    QPair<int, int> targetStair = isGoingUp ? m_stairsUpPosition : m_stairsDownPosition;
    if (currentPos != targetStair) {
        logMessage("There are no stairs here to take.");
        return;
    }
    if (isGoingUp) {
        int newLevel = currentZ - 1;
        if (newLevel >= 1) {
            logMessage(QString("You take the **stairs up** to Level %1.").arg(newLevel));
            enterLevel(newLevel, true);
            SoundEffects::instance()->play(SoundEffects::Type::Stairs);
        } else {
            exitToMainMenu();
        }
    } else {
        int newLevel = currentZ + 1;

        // Boss floors: the guardian must fall before the stairs will work.
        if (BossEncounter::hasBoss(currentZ)) {
            const LevelSnapshot* snap = DungeonLevelRegistry::instance().level(currentZ);
            bool bossDefeated = snap ? snap->bossDefeated : false;
            if (!BossEncounter::canDescend(currentZ, bossDefeated)) {
                logMessage(QString("<font color='red'>%1</font>")
                           .arg(BossEncounter::blockedMessage(currentZ)));
                return;
            }
        }

        logMessage(QString("You take the **stairs down** to Level %1.").arg(newLevel));
        enterLevel(newLevel, false);
        SoundEffects::instance()->play(SoundEffects::Type::Stairs);
    }
}

void DungeonDialog::processTreasureOpening()
{
    gameStateManager* gsm = gameStateManager::instance();
    QPair<int, int> pos = {
        gsm->getGameValue("DungeonX").toInt(),
        gsm->getGameValue("DungeonY").toInt()
    };
    if (m_treasurePositions.contains(pos)) {
        // Already opened — nothing left to take.
        if (m_openedChests.contains(pos)) {
            logMessage("<font color='gray'>The chest is already empty.</font>");
            return;
        }

        QString treasure = m_treasurePositions.value(pos);
        int activeIdx = gsm->getGameValue("ActiveCharacterIndex").toInt(); //

        // Locked chest: requires a specific key item in the party inventory.
        if (m_lockedChests.contains(pos)) {
            QString requiredKey = m_chestKeys.value(pos);
            bool hasKey = false;
            auto& members = gsm->getPartyMembers();
            for (const auto& member : members) {
                for (const auto& item : member.inventory) {
                    if (item.name == requiredKey) {
                        hasKey = true;
                        break;
                    }
                }
                if (hasKey) break;
            }
            if (!hasKey) {
                logMessage(QString("<font color='orange'>The chest is locked. It needs the %1.</font>").arg(requiredKey));
                return;
            }
            logMessage(QString("<font color='gold'>You unlock the chest with the %1!</font>").arg(requiredKey));
            m_lockedChests.remove(pos);
            m_chestKeys.remove(pos);
        }

        if (treasure.contains("Gold")) {
            // Existing Gold logic...
            quint64 foundGold = QRandomGenerator::global()->bounded(500, 5000);
            gsm->addPartyGold(static_cast<int>(foundGold));
            logMessage(QString("You gain %L1 Gold.").arg(foundGold));
        } else {
            // Add item to character inventory
            HeldItem lootItem;
            lootItem.name = treasure;
            lootItem.identified = false;  // dungeon loot starts unidentified
            if (const ItemDef* def = ItemDatabase::instance().byName(treasure)) {
                lootItem.M4E97 = static_cast<int16_t>(def->id);
            }
            gsm->addItemToCharacter(activeIdx, lootItem);
            logMessage(QString("You found a %1 and added it to your inventory!").arg(treasure));
        }
        m_treasurePositions.remove(pos);
        m_openedChests.insert(pos);
        DungeonLevelRegistry::instance().levelForEdit(gsm->getGameValue("DungeonLevel").toInt()).openedChests = m_openedChests;
        drawMinimap();
    }
}
bool DungeonDialog::isWallAt(int x, int y, int z) {
    // Check if the coordinates are outside the map boundaries
    if (x < 0 || x >= MAP_SIZE || y < 0 || y >= MAP_SIZE) {
        return true;
    }
    // Multi-level: if on lower level (z=-1), use lower level tile set
    if (z < 0 && !m_lowerLevelTiles.isEmpty()) {
        return !m_lowerLevelTiles.contains({x, y});
    }
    // Check if the position is in our obstacle set
    return m_obstaclePositions.contains({x, y});
}

bool DungeonDialog::isWallAtSide(int x, int y, const QString& side) {
    // Retrieve the current facing direction from the UI label
    QString facing = m_compassLabel->text();
    int targetX = x;
    int targetY = y;

    // Relative logic: If facing North, "left" is West (-1, 0).
    // If facing South, "left" is East (+1, 0), etc.
    if (facing == "Facing North") {
        targetX = (side == "left") ? x - 1 : x + 1;
    } else if (facing == "Facing South") {
        targetX = (side == "left") ? x + 1 : x - 1;
    } else if (facing == "Facing East") {
        targetY = (side == "left") ? y - 1 : y + 1;
    } else if (facing == "Facing West") {
        targetY = (side == "left") ? y + 1 : y - 1;
    }

    return isWallAt(targetX, targetY, m_onLowerLevel ? -1 : 0);
}

void DungeonDialog::renderWireframeView() {
    m_dungeonScene->clear();
    m_dungeonScene->setBackgroundBrush(Qt::black);

    gameStateManager *gsm = gameStateManager::instance();
    int px = gsm->getGameValue("DungeonX").toInt();
    int py = gsm->getGameValue("DungeonY").toInt();
    QString facing = m_compassLabel->text();

    // Multi-level rendering: if on lower level, show lower level tiles
    QSet<QPair<int, int>> *activeObstacles = &m_obstaclePositions;
    QSet<QPair<int, int>> *activeFloor = &m_roomFloorTiles;
    if (m_onLowerLevel && !m_lowerLevelTiles.isEmpty()) {
        activeObstacles = &m_lowerLevelTiles;
        activeFloor = &m_lowerLevelTiles;
    }

    const int w = 300;
    const int h = 300;

    // Perspective points
    int xs[] = {0, 75, 120, 150};
    int ys[] = {0, 75, 120, 150};

    for (int d = 2; d >= 0; --d) {
        // 1. Calculate tile coordinates for the CENTER path
        int tx = px, ty = py;
        int dx = 0, dy = 0;
        if (facing.contains("North")) dy = -1;
        else if (facing.contains("South")) dy = 1;
        else if (facing.contains("East"))  dx = 1;
        else if (facing.contains("West"))  dx = -1;

        tx += (dx * d);
        ty += (dy * d);

        // 2. Calculate coordinates for adjacent tiles (Left/Right) at this depth
        // This is necessary to see if the side-corridor is blocked by a wall
        int lx = tx + dy, ly = ty - dx; // Left tile relative to facing
        int rx = tx - dy, ry = ty + dx; // Right tile relative to facing

        // Wall Checks
        int zLevel = gsm->getGameValue("DungeonZ").toInt();
        bool wallFront      = isWallAt(tx, ty, zLevel);
        bool wallLeftSide   = isWallAtSide(tx, ty, "left");  // The plane parallel to you
        bool wallRightSide  = isWallAtSide(tx, ty, "right"); // The plane parallel to you
        bool wallInLeftTile = isWallAt(lx, ly, zLevel);      // The wall inside the side corridor
        bool wallInRightTile = isWallAt(rx, ry, zLevel);     // The wall inside the side corridor

        // Screen coordinates for this depth
        int xL = xs[d];     int xR = w - xs[d];
        int yT = ys[d];     int yB = h - ys[d];
        int nxL = xs[d+1];  int nxR = w - xs[d+1];
        int nyT = ys[d+1];  int nyB = h - ys[d+1];

        // --- 1. FLOOR & CEILING (Full Span) ---
        QPolygon floor, ceil;
        floor << QPoint(0, yB) << QPoint(w, yB) << QPoint(w, nyB) << QPoint(0, nyB);
        ceil << QPoint(0, yT) << QPoint(w, yT) << QPoint(w, nyT) << QPoint(0, nyT);

        int floorCol = qMax(0, 40 - (d * 10));
        int ceilR = qMax(0, 80 - (d * 20)); // Brown base
        m_dungeonScene->addPolygon(floor, QPen(Qt::NoPen), QBrush(QColor(floorCol, floorCol, floorCol)));
        m_dungeonScene->addPolygon(ceil, QPen(Qt::NoPen), QBrush(QColor(ceilR, qMax(0, 50-(d*15)), qMax(0, 30-(d*10)))));

        // --- 2. SIDE-FACING WALLS (Corridor Walls) ---
        int sideWallCol = qMax(0, 80 - (d * 15));
        if (wallLeftSide) {
            QPolygon p; p << QPoint(xL, yT) << QPoint(nxL, nyT) << QPoint(nxL, nyB) << QPoint(xL, yB);
            m_dungeonScene->addPolygon(p, QPen(Qt::black), QBrush(QColor(sideWallCol, sideWallCol, sideWallCol)));
            drawBrickPattern(p, d);
        }
        if (wallRightSide) {
            QPolygon p; p << QPoint(xR, yT) << QPoint(nxR, nyT) << QPoint(nxR, nyB) << QPoint(xR, yB);
            m_dungeonScene->addPolygon(p, QPen(Qt::black), QBrush(QColor(sideWallCol, sideWallCol, sideWallCol)));
            drawBrickPattern(p, d);
        }

        // --- 3. FRONT-FACING WALLS IN SIDE CORRIDORS ---
        // If there is NO side wall blocking the view, check if the adjacent tile has a wall
        int sideRoomWallCol = qMax(0, 65 - (d * 15));
        if (!wallLeftSide && wallInLeftTile) {
            // Draw a wall on the left wing of the screen
            m_dungeonScene->addRect(0, yT, xL, yB - yT, QPen(Qt::black), QBrush(QColor(sideRoomWallCol, sideRoomWallCol, sideRoomWallCol)));
        }
        if (!wallRightSide && wallInRightTile) {
            // Draw a wall on the right wing of the screen
            m_dungeonScene->addRect(xR, yT, w - xR, yB - yT, QPen(Qt::black), QBrush(QColor(sideRoomWallCol, sideRoomWallCol, sideRoomWallCol)));
        }

        bool hasAntimagic = m_antimagicPositions.contains({tx, ty});

        // 1. Draw floor/ceiling
        // ...

        // 2. Draw Antimagic effect on the floor
        if (hasAntimagic) {
            drawAntimagic(d, xL, xR, yB, nxL, nxR, nyB);
        }
        // Identify if tile is water (Adjust based on your tile type logic)
        // Assuming you have a set or map for water tiles
        bool hasWater = m_waterPositions.contains({tx, ty});

        // 1. Draw floor and ceiling first
        // ... (existing floor drawing) ...

        // 2. Draw Water on top of the floor
        if (hasWater) {
            drawWater(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // 1. Check if this tile is a Spinner (Rotator)
        bool hasSpinner = m_rotatorPositions.contains({tx, ty});

        // ... (Existing Floor, Ceiling, and Side Wall rendering)

        // 2. Draw the Spinner effect
        if (hasSpinner) {
            drawSpinner(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check for teleporter at this map coordinate
        bool hasTeleporter = m_teleporterPositions.contains({tx, ty});

        // ... existing Floor/Ceiling/Side Wall rendering ...

        // 2. Draw the Teleporter effect on the floor
        if (hasTeleporter) {
            drawTeleporter(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check for pit at this coordinate
        bool hasPit = m_pitPositions.contains({tx, ty});
        if (hasPit) {
            drawPit(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check for fog at this coordinate
        bool hasFog = m_fogPositions.contains({tx, ty});
        if (hasFog) {
            drawFog(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check for extinguisher at this coordinate
        bool hasExtinguisher = m_extinguisherPositions.contains({tx, ty});
        if (hasExtinguisher) {
            drawExtinguisher(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check for treasure at this coordinate
        bool hasTreasure = m_treasurePositions.contains({tx, ty});
        if (hasTreasure) {
            drawTreasure(d, xL, xR, yB);
        }

        // Check for trap at this coordinate
        bool hasTrap = m_trapPositions.contains({tx, ty});
        if (hasTrap) {
            drawTrap(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check for hidden door at this coordinate
        bool hasHiddenDoor = m_hiddenDoorPositions.contains({tx, ty});
        if (hasHiddenDoor) {
            drawHiddenDoor(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check for stud at this coordinate
        bool hasStud = m_studPositions.contains({tx, ty});
        if (hasStud) {
            drawStud(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check for body at this coordinate
        bool hasBody = m_bodyPositions.contains({tx, ty});
        if (hasBody) {
            drawBody(d, xL, xR, yB, nxL, nxR, nyB);
        }

        // Check if there is a monster at this coordinate
        bool hasMonster = m_monsterPositions.contains({tx, ty});

        // ... (draw floor and side walls)

        // Draw the monster if present
        if (hasMonster) {
            drawMonster(d, xL, xR, yB);
        }

        // --- 4. FRONT WALL (Main Path) ---
        if (wallFront) {
            int frontCol = qMax(0, 110 - (d * 20));
            m_dungeonScene->addRect(xL, yT, xR - xL, yB - yT, QPen(Qt::black), QBrush(QColor(frontCol, frontCol, frontCol)));
        }

        // 1. Check if there is a chute at this specific map tile
        bool hasChute = m_chutePositions.contains({tx, ty});
        // 2. Draw the chute on top of the floor
        if (hasChute) {
            drawChute(d, xL, xR, yB, nxL, nxR, nyB);
        }
    }
}

void DungeonDialog::drawBrickPattern(const QPolygon& wallPoly, int depth)
{
    Q_UNUSED(depth);
    QRect bounds = wallPoly.boundingRect();
    QPen mortarPen(QColor(40, 40, 40, 150)); // Semi-transparent dark gray
    mortarPen.setWidth(1);

    int rows = 6;    // Number of brick layers
    int columns = 4; // Number of bricks per row

    // Calculate row height
    double rowHeight = static_cast<double>(bounds.height()) / rows;

    for (int i = 1; i < rows; ++i) {
        int y = bounds.top() + (i * rowHeight);

        // Find left and right edges of the polygon at this specific Y height
        // to ensure bricks don't float outside the perspective wall
        int xMin = bounds.right();
        int xMax = bounds.left();

        // Basic scan-line logic to keep lines inside the trapezoid
        for (int x = bounds.left(); x <= bounds.right(); ++x) {
            if (wallPoly.containsPoint(QPoint(x, y), Qt::OddEvenFill)) {
                xMin = qMin(xMin, x);
                xMax = qMax(xMax, x);
            }
        }

        // Draw horizontal mortar line
        if (xMin < xMax) {
            m_dungeonScene->addLine(xMin, y, xMax, y, mortarPen);

            // Draw vertical "staggered" mortar lines
            double colWidth = static_cast<double>(xMax - xMin) / columns;
            double offset = (i % 2 == 0) ? 0 : colWidth / 2; // Stagger bricks

            for (int j = 0; j <= columns; ++j) {
                int vx = xMin + (j * colWidth) + offset;
                if (vx > xMin && vx < xMax) {
                    m_dungeonScene->addLine(vx, y, vx, y - rowHeight, mortarPen);
                }
            }
        }
    }
}

void DungeonDialog::drawChute(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    // We want the chute to be in the center of the tile floor
    // Calculate a 40% width/depth hole
    double margin = 0.3;
    // Interpolate points for the "hole" on the floor
    int cxL = xL + (xR - xL) * margin;
    int cxR = xR - (xR - xL) * margin;
    int cnxL = nxL + (nxR - nxL) * margin;
    int cnxR = nxR - (nxR - nxL) * margin;
    // Perspective depth for the floor (yB to nyB)
    int cyNear = yB;
    int cyFar = yB + (nyB - yB) * 0.6; // The hole doesn't take the whole tile

    QPolygon chuteHole;
    chuteHole << QPoint(cxL, cyNear) << QPoint(cxR, cyNear)
              << QPoint(cnxR, cyFar) << QPoint(cnxL, cyFar);
    // Draw the "void" (black hole)
    m_dungeonScene->addPolygon(chuteHole, QPen(Qt::black), QBrush(Qt::black));
    // Draw inner "walls" of the chute for a 3D effect
    QPolygon leftInner;
    leftInner << QPoint(cxL, cyNear) << QPoint(cnxL, cyFar)
              << QPoint(cnxL, cyFar + 10) << QPoint(cxL, cyNear + 10);

    int depthShade = qMax(0, 30 - (d * 10));
    m_dungeonScene->addPolygon(leftInner, QPen(Qt::NoPen), QBrush(QColor(depthShade, depthShade, depthShade)));
}

void DungeonDialog::drawMonster(int d, int xL, int xR, int yB) {
    // Calculate size based on depth
    // d=0 (Near): Large, d=2 (Far): Small
    int monsterWidth = (xR - xL) * 0.6;
    int monsterHeight = monsterWidth * 1.2;

    // Center horizontally, sit on the floor (yB)
    int centerX = xL + (xR - xL) / 2;
    int xPos = centerX - (monsterWidth / 2);
    int yPos = yB - monsterHeight;

    // Depth shading: make monsters darker in the distance
    int shade = qMax(0, 200 - (d * 60));
    QColor monsterColor(shade, 0, 0); // Dark red silhouette

    // Draw a simple head and body (billboard style)
    QRect body(xPos, yPos + (monsterHeight * 0.3), monsterWidth, monsterHeight * 0.7);
    QRect head(centerX - (monsterWidth / 4), yPos, monsterWidth / 2, monsterHeight * 0.4);

    m_dungeonScene->addEllipse(head, QPen(Qt::black), QBrush(monsterColor));
    m_dungeonScene->addRect(body, QPen(Qt::black), QBrush(monsterColor));
}

void DungeonDialog::drawTeleporter(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB)
{
    Q_UNUSED(nxR);
    Q_UNUSED(nxL);
    // Calculate the center and size of the tile floor
    int centerX = xL + (xR - xL) / 2;
    int centerY = yB + (nyB - yB) / 2;
    // Scale radius based on depth
    int baseRadius = (xR - xL) / 3;
    // Create a shimmering effect with multiple ellipses
    for (int i = 0; i < 3; ++i) {
        int r = baseRadius - (i * (baseRadius / 4));
        QRectF glowRect(centerX - r, centerY - (r / 2), r * 2, r); // Flattened for perspective
        // Alternating cyan/blue glow
        QColor glowColor = (i % 2 == 0) ? QColor(0, 255, 255, 150) : QColor(0, 100, 255, 100);
        // Depth shading: make it dimmer in the distance
        int alpha = glowColor.alpha() - (d * 30);
        glowColor.setAlpha(qMax(0, alpha));
        m_dungeonScene->addEllipse(glowRect, QPen(Qt::white, 1), QBrush(glowColor));
    }
    // Optional: Add some "sparkles" (small white dots)
    for (int j = 0; j < 5; ++j) {
        int sx = centerX + (QRandomGenerator::global()->bounded(baseRadius * 2) - baseRadius);
        int sy = centerY + (QRandomGenerator::global()->bounded(baseRadius) - (baseRadius / 2));
        m_dungeonScene->addRect(sx, sy, 1, 1, QPen(Qt::white), QBrush(Qt::white));
    }
}

void DungeonDialog::drawSpinner(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    // Calculate the center of the tile floor using both near and far bounds
    int centerX = (xL + xR + nxL + nxR) / 4;
    int centerY = (yB + nyB) / 2;
    // Scale the size of the spinner based on the available floor width at this depth
    int radius = (xR - xL) / 3;
    QPen spinnerPen(QColor(200, 200, 0), 2);
    // Depth shading to make it darker in the distance
    int alpha = qMax(0, 200 - (d * 50));
    spinnerPen.setColor(QColor(200, 200, 0, alpha));
    // Define the bounding rect for the ellipse/arcs, flattened for perspective
    QRectF arcRect(centerX - radius, centerY - (radius / 2), radius * 2, radius);
    // QGraphicsScene doesn't have addArc; we use QPainterPath instead
    for (int i = 0; i < 4; ++i) {
        QPainterPath path;
        int startAngle = i * 90; // Angles in QPainterPath are in degrees
        int spanAngle = 60;

        path.arcMoveTo(arcRect, startAngle);
        path.arcTo(arcRect, startAngle, spanAngle);

        m_dungeonScene->addPath(path, spinnerPen);
    }
    // Add a center point (uses near/far center)
    m_dungeonScene->addEllipse(centerX - 2, centerY - 1, 4, 2, spinnerPen, QBrush(spinnerPen.color()));
}

void DungeonDialog::drawWater(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    // 1. Create the water surface polygon (the floor area)
    QPolygon waterPoly;
    waterPoly << QPoint(xL, yB) << QPoint(xR, yB) << QPoint(nxR, nyB) << QPoint(nxL, nyB);
    // Deep blue color, semi-transparent so you can still see the floor/chutes
    int alpha = qMax(0, 180 - (d * 40));
    QColor waterColor(0, 105, 148, alpha); // Sea Blue
    m_dungeonScene->addPolygon(waterPoly, QPen(Qt::NoPen), QBrush(waterColor));
    // 2. Add "Ripples" (shimmering lines)
    QPen ripplePen(QColor(255, 255, 255, qMax(0, 100 - (d * 30))));
    ripplePen.setWidth(1);
    // Draw 3 horizontal lines at different depths within the tile
    for (int i = 1; i <= 3; ++i) {
        double ratio = i / 4.0;
        // Interpolate Y position between near (yB) and far (nyB)
        int ry = yB + (nyB - yB) * ratio;
        // Interpolate X width based on perspective
        int rxL = xL + (nxL - xL) * ratio;
        int rxR = xR + (nxR - xR) * ratio;
        // Draw a partial shimmering line (not the whole width for a natural look)
        int rippleWidth = (rxR - rxL) * 0.6;
        int rippleStart = rxL + (rxR - rxL) * 0.2;

        m_dungeonScene->addLine(rippleStart, ry, rippleStart + rippleWidth, ry, ripplePen);
    }
}
void DungeonDialog::drawAntimagic(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    // Define the floor polygon for clipping or reference
    QPolygon floorPoly;
    floorPoly << QPoint(xL, yB) << QPoint(xR, yB) << QPoint(nxR, nyB) << QPoint(nxL, nyB);
    // Color: A muted, "dulling" purple or gray
    int alpha = qMax(0, 120 - (d * 30));
    QColor fieldColor(100, 100, 150, alpha);
    // Draw a subtle tint first
    m_dungeonScene->addPolygon(floorPoly, QPen(Qt::NoPen), QBrush(fieldColor));
    // Draw "Static" lines (cross-hatch pattern)
    QPen staticPen(QColor(200, 200, 255, qMax(0, 150 - (d * 40))));
    staticPen.setWidth(1);

    int lines = 5;
    for (int i = 0; i <= lines; ++i) {
        double ratio = (double)i / lines;

        // Vertical-ish lines (converging towards vanishing point)
        int xNear = xL + (xR - xL) * ratio;
        int xFar = nxL + (nxR - nxL) * ratio;
        m_dungeonScene->addLine(xNear, yB, xFar, nyB, staticPen);

        // Horizontal lines (interpolated by depth)
        int yHoriz = yB + (nyB - yB) * ratio;
        int xHLeft = xL + (nxL - xL) * ratio;
        int xHRight = xR + (nxR - xR) * ratio;
        m_dungeonScene->addLine(xHLeft, yHoriz, xHRight, yHoriz, staticPen);
    }
}

void DungeonDialog::drawPit(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    // Dark hole in the floor — smaller than a chute, darker
    double margin = 0.35;
    int cxL = xL + (xR - xL) * margin;
    int cxR = xR - (xR - xL) * margin;
    int cnxL = nxL + (nxR - nxL) * margin;
    int cnxR = nxR - (nxR - nxL) * margin;
    int cyNear = yB;
    int cyFar = yB + (nyB - yB) * 0.5;
    QPolygon pitHole;
    pitHole << QPoint(cxL, cyNear) << QPoint(cxR, cyNear)
            << QPoint(cnxR, cyFar) << QPoint(cnxL, cyFar);
    m_dungeonScene->addPolygon(pitHole, QPen(Qt::black), QBrush(Qt::black));
    int depthShade = qMax(0, 15 - (d * 5));
    QPolygon leftInner;
    leftInner << QPoint(cxL, cyNear) << QPoint(cnxL, cyFar)
               << QPoint(cnxL, cyFar + 8) << QPoint(cxL, cyNear + 8);
    m_dungeonScene->addPolygon(leftInner, QPen(Qt::NoPen), QBrush(QColor(depthShade, depthShade, depthShade)));
}

void DungeonDialog::drawFog(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    int alpha = qMax(60, 120 - (d * 20));
    m_dungeonScene->addRect(xL, yB, xR - xL, nyB - yB,
                            QPen(QColor(180, 180, 180, alpha), 1),
                            QBrush(QColor(180, 180, 180, alpha / 3)));
}

void DungeonDialog::drawExtinguisher(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    int centerX = xL + (xR - xL) * 0.7;
    int centerY = yB + (nyB - yB) * 0.3;
    int radius = (xR - xL) / 8;
    QColor waterColor(0, 150, 255, 200);
    for (int i = 0; i < 3; ++i) {
        int r = radius - (i * (radius / 4));
        m_dungeonScene->addEllipse(centerX - r, centerY - r / 2, r * 2, r,
                                  QPen(waterColor, 1), QBrush(waterColor));
    }
}

void DungeonDialog::drawTreasure(int d, int xL, int xR, int yB) {
    int chestW = (xR - xL) * 0.5;
    int chestH = chestW * 0.6;
    int chestX = xL + (xR - xL) / 2 - chestW / 2;
    int chestY = yB - chestH;
    m_dungeonScene->addRect(chestX, chestY + chestH * 0.3, chestW, chestH * 0.7,
                            QPen(QColor(139, 69, 19)), QBrush(QColor(218, 165, 32)));
    m_dungeonScene->addRect(chestX, chestY, chestW, chestH * 0.3,
                            QPen(QColor(139, 69, 19)), QBrush(QColor(240, 200, 80)));
    int lockSize = chestW * 0.1;
    m_dungeonScene->addRect(chestX + chestW / 2 - lockSize / 2, chestY + chestH * 0.25,
                            lockSize, lockSize, QPen(Qt::black), QBrush(Qt::black));
}

void DungeonDialog::drawTrap(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    int spikeCount = 5;
    int spikeW = (xR - xL) / (spikeCount * 2);
    int spikeH = (nyB - yB) * 0.8;
    for (int i = 0; i < spikeCount; ++i) {
        int sx = xL + (xR - xL) * (i + 0.5) / spikeCount - spikeW / 2;
        int sy = yB - spikeH;
        QPolygon spike;
        spike << QPoint(sx, yB) << QPoint(sx + spikeW / 2, sy) << QPoint(sx + spikeW, yB);
        int shade = qMax(0, 180 - (d * 40));
        m_dungeonScene->addPolygon(spike, QPen(QColor(shade, shade, shade)), QBrush(QColor(shade, shade, shade)));
    }
}

void DungeonDialog::drawHiddenDoor(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    Q_UNUSED(d); Q_UNUSED(nxL); Q_UNUSED(nxR); Q_UNUSED(nyB);
    int doorW = (xR - xL) * 0.4;
    int doorH = yB * 0.8;
    int doorX = xL + (xR - xL) / 2 - doorW / 2;
    int doorY = yB - doorH;
    m_dungeonScene->addRect(doorX, doorY, doorW, doorH,
                            QPen(QColor(101, 67, 33), 2), QBrush(QColor(139, 90, 43)));
    m_dungeonScene->addRect(doorX - 2, doorY - 2, doorW + 4, doorH + 4,
                            QPen(QColor(80, 50, 20), 1), QBrush(Qt::NoBrush));
    m_dungeonScene->addEllipse(doorX + doorW * 0.75, doorY + doorH * 0.5, 4, 4,
                              QPen(Qt::yellow), QBrush(Qt::yellow));
}

void DungeonDialog::drawStud(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    Q_UNUSED(d); Q_UNUSED(nxL); Q_UNUSED(nxR); Q_UNUSED(nyB);
    int studSize = (xR - xL) * 0.08;
    double positions[] = {0.2, 0.5, 0.8};
    for (int i = 0; i < 3; ++i) {
        int sx = xL + (xR - xL) * positions[i] - studSize / 2;
        int sy = yB - studSize * 2;
        m_dungeonScene->addRect(sx, sy, studSize, studSize,
                                QPen(QColor(100, 100, 100)), QBrush(QColor(160, 160, 160)));
    }
}

void DungeonDialog::drawBody(int d, int xL, int xR, int yB, int nxL, int nxR, int nyB) {
    Q_UNUSED(d); Q_UNUSED(nxL); Q_UNUSED(nxR); Q_UNUSED(nyB);
    int bodyW = (xR - xL) * 0.6;
    int bodyH = (nyB - yB) * 0.3;
    int bodyX = xL + (xR - xL) / 2 - bodyW / 2;
    int bodyY = yB - bodyH;
    m_dungeonScene->addEllipse(bodyX, bodyY, bodyW, bodyH,
                              QPen(QColor(80, 40, 40)), QBrush(QColor(120, 60, 60)));
    int headR = bodyW * 0.15;
    m_dungeonScene->addEllipse(bodyX + bodyW / 2 - headR, bodyY - headR,
                              headR * 2, headR * 2,
                              QPen(QColor(80, 40, 40)), QBrush(QColor(140, 100, 80)));
}

void DungeonDialog::togglePartyInfo() {
    if (m_charSheet) {
        m_charSheet->close();
        m_charSheet = nullptr;
        return;
    }

    m_charSheet = new PartyInfoDialog(this);
    // 1. ADD Qt::WindowDoesNotAcceptFocus
    // This allows the main window to keep keyboard focus for movement
    m_charSheet->setWindowFlags(Qt::Tool |
                                Qt::WindowStaysOnTopHint |
                                Qt::WindowDoesNotAcceptFocus |
                                Qt::CustomizeWindowHint |
                                Qt::WindowTitleHint |
                                Qt::WindowCloseButtonHint);

    m_charSheet->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_charSheet, &QObject::destroyed, this, [this]() { m_charSheet = nullptr; });
    // 2. Use show() but DO NOT call activateWindow() or raise()
    // activateWindow() is what forces the focus change; we want to avoid that.
    m_charSheet->show();
    // 3. Position in Bottom-Right
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int dialogWidth = m_charSheet->frameGeometry().width();
        int dialogHeight = m_charSheet->frameGeometry().height();
        int padding = 10;

        m_charSheet->move(screenGeometry.width() - dialogWidth - padding,
                          screenGeometry.height() - dialogHeight - padding);
    }
}

void DungeonDialog::on_spellButton_clicked()
{
    logMessage("You try to cast some sort of spell but fail.");
    // Check if in antimagic zone
    gameStateManager* gsm = gameStateManager::instance();
    int x = gsm->getGameValue("DungeonX").toInt();
    int y = gsm->getGameValue("DungeonY").toInt();
    QPair<int, int> pos = {x, y};

    if (m_antimagicPositions.contains(pos)) {
        logMessage("<font color='purple'>An antimagic field prevents you from casting spells here!</font>");
        return;
    }

    // Open the spell casting dialog
    SpellCastingDialog* spellDialog = new SpellCastingDialog(this, m_inCombat);
    spellDialog->setAttribute(Qt::WA_DeleteOnClose);

    // Connect spell effects to dungeon actions
    connect(spellDialog, &SpellCastingDialog::spellCast, this,
            [this](const QString& spellName, const SpellResult& result) {

    logMessage(QString("<font color='cyan'>%1</font>").arg(result.message));

        // Handle damage to current monster if in combat (turn-based).
        // Route through CombatActions so death bookkeeping and the defensive
        // stance are respected, instead of poking participant HP directly.
        if (result.damageDealt > 0 && m_inCombat && m_combatController) {
            // Cast at the monster the player selected (falls back to the first
            // living one), not always the first in the participant list.
            int targetIdx = selectedMonsterIndex();
            if (targetIdx < 0) targetIdx = selectFirstLivingMonster();
            if (targetIdx >= 0) {
                QString dmgResult;
                // Route through the school-aware path so fire splashes, cold
                // slows, lightning chains and mind stuns actually happen.
                m_combatController->combatActions()->applySpellDamageBySchool(targetIdx, spellName,
                                                          result.damageDealt, dmgResult);
                logMessage(QString("<font color='yellow'>%1</font>").arg(dmgResult));

                if (!m_combatController->combatState()->participant(targetIdx).isAlive) {
                    // Remove from map
                    QPair<int, int> pos = getCurrentPosition();
                    m_monsterPositions.remove(pos);
                    renderWireframeView();
                    drawMinimap();
                }

                // Check combat end
                QStringList deaths = m_combatController->deathHandler()->processDeaths();
                for (const QString& d : deaths) logMessage(d);
                syncCombatToGameState();
                updateCombatUI();
                advanceCombat();
            }
        }

        // Handle teleport spell
        if (result.effectApplied == "Teleport") {
            on_teleportButton_clicked();
        }
    });

    spellDialog->exec();
}

QStringList DungeonDialog::getThematicMonsters(int level) const
{
    // Monsters come from the game's own monster table (MDATA5, loaded by
    // gameStateManager). Each row carries `levelFound`, the dungeon floor it
    // belongs on, so a floor draws from monsters of a matching level rather
    // than a hardcoded name list. Fall back to nearby levels if a floor is thin.
    const QList<QVariantMap>& table = gameStateManager::instance()->getMonsterData();
    QStringList monsters;

    auto collect = [&](int wantLevel) {
        for (const QVariantMap& m : table) {
            if (m.value("levelFound").toInt() == wantLevel) {
                const QString name = m.value("name").toString();
                if (!name.isEmpty() && !monsters.contains(name)) {
                    monsters << name;
                }
            }
        }
    };

    collect(level);
    // Widen the net if this floor has too few distinct monsters.
    for (int delta = 1; delta <= 3 && monsters.size() < 4; ++delta) {
        if (level - delta >= 1) collect(level - delta);
        collect(level + delta);
    }

    // Last resort: any monster at all, so the floor is never empty.
    if (monsters.isEmpty()) {
        for (const QVariantMap& m : table) {
            const QString name = m.value("name").toString();
            if (!name.isEmpty()) monsters << name;
        }
    }

    return monsters;
}

DungeonDialog::~DungeonDialog() = default;
