#include "ArenaDialog.h"
#include "src/core/Arena.h"
#include "src/core/gameStateManager.h"
#include "src/character/character.h"
#include "src/combat/CombatState.h"
#include "src/combat/TurnEngine.h"
#include "src/combat/CombatActions.h"
#include "src/combat/MonsterAI.h"
#include "src/combat/CombatDeathHandler.h"
#include "src/combat/EncounterBuilder.h"
#include <QFile>
#include <QJsonDocument>
#include <QVariantMap>
#include <QVariantList>

// Combat state members (same pattern as DungeonDialog)
namespace {
    int selectedMonsterIdx = -1;
    int waveDamageTaken = 0;
    int waveRounds = 0;
    int waveKills = 0;
}

ArenaDialog::ArenaDialog(QWidget *parent) : QDialog(parent) {
    m_combatState = std::make_unique<CombatState>();
    m_turnEngine = std::make_unique<TurnEngine>();
    m_turnEngine->setCombatState(m_combatState.get());
    m_combatActions = std::make_unique<CombatActions>(m_combatState.get(), m_turnEngine.get());
    m_monsterAI = std::make_unique<MonsterAI>(m_combatState.get(), m_turnEngine.get(), m_combatActions.get());
    m_deathHandler = std::make_unique<CombatDeathHandler>(m_combatState.get(), m_turnEngine.get(), m_combatActions.get());
    setupUi();
    updateUI();
}

void ArenaDialog::setupUi() {
    setWindowTitle(Arena::arenaName());
    setMinimumSize(600, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title
    m_titleLabel = new QLabel(Arena::arenaName(), this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("Georgia", 24, QFont::Bold);
    m_titleLabel->setFont(titleFont);
    mainLayout->addWidget(m_titleLabel);

    // Wave / score labels
    m_waveLabel = new QLabel("Welcome to the Arena", this);
    m_waveLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_waveLabel);

    m_scoreLabel = new QLabel("Score: 0", this);
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_scoreLabel);

    // Party HP
    m_partyHpLabel = new QLabel(this);
    m_partyHpLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_partyHpLabel);

    // Monster HP
    m_monsterHpLabel = new QLabel(this);
    m_monsterHpLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_monsterHpLabel);

    // Message log
    m_messageLog = new QListWidget(this);
    mainLayout->addWidget(m_messageLog);

    // Target list
    m_targetList = new QListWidget(this);
    mainLayout->addWidget(m_targetList);

    // Buttons — grouped logically: Navigation / Combat / Wave control
    QHBoxLayout* topBtnLayout = new QHBoxLayout();
    m_startBtn = new QPushButton("Start Arena", this);
    m_fleeBtn = new QPushButton("Flee", this);
    m_nextWaveBtn = new QPushButton("Next Wave", this);
    topBtnLayout->addWidget(m_startBtn);
    topBtnLayout->addWidget(m_fleeBtn);
    topBtnLayout->addWidget(m_nextWaveBtn);
    mainLayout->addLayout(topBtnLayout);

    QHBoxLayout* combatBtnLayout = new QHBoxLayout();
    m_attackBtn = new QPushButton("Attack", this);
    m_defendBtn = new QPushButton("Defend", this);
    m_spellBtn = new QPushButton("Cast Spell", this);
    m_useItemBtn = new QPushButton("Use Item", this);
    combatBtnLayout->addWidget(m_attackBtn);
    combatBtnLayout->addWidget(m_defendBtn);
    combatBtnLayout->addWidget(m_spellBtn);
    combatBtnLayout->addWidget(m_useItemBtn);
    mainLayout->addLayout(combatBtnLayout);

    // Signals
    connect(m_startBtn, &QPushButton::clicked, this, &ArenaDialog::on_startButton_clicked);
    connect(m_fleeBtn, &QPushButton::clicked, this, &ArenaDialog::on_fleeButton_clicked);
    connect(m_attackBtn, &QPushButton::clicked, this, &ArenaDialog::on_attackButton_clicked);
    connect(m_defendBtn, &QPushButton::clicked, this, &ArenaDialog::on_defendButton_clicked);
    connect(m_spellBtn, &QPushButton::clicked, this, &ArenaDialog::on_spellButton_clicked);
    connect(m_useItemBtn, &QPushButton::clicked, this, &ArenaDialog::on_useItemButton_clicked);
    connect(m_nextWaveBtn, &QPushButton::clicked, this, &ArenaDialog::on_nextWaveButton_clicked);

    // Target list: click to select a monster
    connect(m_targetList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (!item) return;
        int idx = item->data(Qt::UserRole).toInt();
        if (idx >= 0 && m_combatState && idx < m_combatState->participantCount()
            && !m_combatState->participant(idx).isPlayer
            && m_combatState->participant(idx).isAlive) {
            selectedMonsterIdx = idx;
            updateUI();
        }
    });
}

void ArenaDialog::on_startButton_clicked() {
    m_currentWave = 0;
    m_totalScore = 0;
    m_totalKills = 0;
    m_totalDamageTaken = 0;
    m_totalRounds = 0;
    m_waveResults.clear();
    m_arenaComplete = false;
    m_messageLog->clear();

    logMessage("The arena gates open. Ten waves stand between you and glory.");
    startWave(1);
}

void ArenaDialog::startWave(int wave) {
    m_currentWave = wave;
    m_inCombat = true;
    waveDamageTaken = 0;
    waveRounds = 0;
    waveKills = 0;
    selectedMonsterIdx = -1;

    // Clear previous combat
    if (m_combatState) m_combatState->clear();

    // Build monsters for this wave
    gameStateManager* gsm = gameStateManager::instance();
    int ngPlus = gsm->getNgPlusLevel();
    int monsterCount = Arena::monsterCountForWave(wave);
    int monsterLevel = Arena::monsterLevelForWave(wave, ngPlus);

    // Create combat participants
    for (int i = 0; i < monsterCount; ++i) {
        CombatParticipant p;
        p.name = QString("Wave %1 Monster %2").arg(wave).arg(i + 1);
        p.isPlayer = false;
        p.hp = 20 + monsterLevel * 5;
        p.maxHp = p.hp;
        p.att = 5 + monsterLevel * 2;
        p.def = 2 + monsterLevel;
        p.speed = monsterLevel;
        p.dex = monsterLevel;
        p.level = monsterLevel;
        p.damageMod = 100 + monsterLevel * 5;
        p.swings = 1;
        p.levelScale = monsterLevel / 2;
        p.ranged = false;
        m_combatState->addParticipant(p);
    }

    // Add party members
    QList<Character> members = gsm->getPartyMembers();
    for (int i = 0; i < members.size(); ++i) {
        const Character& c = members[i];
        CombatParticipant p;
        p.name = c.name;
        p.isPlayer = true;
        p.hp = c.hp;
        p.maxHp = c.maxHp;
        p.att = c.strength;
        p.def = c.constitution / 2;
        p.speed = c.dexterity;
        p.dex = c.dexterity;
        p.level = c.level;
        p.damageMod = 100;
        p.swings = 1;
        p.levelScale = 0;
        p.mana = c.mana;
        p.maxMana = c.maxMana;
        p.ranged = false;
        p.row = (i < 2) ? CombatRow::Front : CombatRow::Back;
        m_combatState->addParticipant(p);
    }

    // Roll initiative
    m_combatState->rollInitiative();
    m_combatState->startRound();

    logMessage(QString("Wave %1 begins! %2 monsters approach.").arg(wave).arg(monsterCount));
    updateUI();
    advanceCombat();
}

void ArenaDialog::on_attackButton_clicked() {
    if (!m_inCombat || !m_combatState) return;

    // Find first living monster if none selected
    if (selectedMonsterIdx < 0 || !m_combatState->participant(selectedMonsterIdx).isAlive) {
        for (int i = 0; i < m_combatState->participantCount(); ++i) {
            if (!m_combatState->participant(i).isPlayer && m_combatState->participant(i).isAlive) {
                selectedMonsterIdx = i;
                break;
            }
        }
    }

    if (selectedMonsterIdx < 0) return;

    int targetIdx = selectedMonsterIdx;
    performPlayerAction([&](QString& result) {
        m_combatActions->attack(targetIdx, result);
        if (!m_combatState->participant(targetIdx).isAlive) {
            waveKills++;
            logMessage(QString("Monster killed! (%1 total)").arg(waveKills));
            selectedMonsterIdx = -1;
        }
    });
}

void ArenaDialog::on_defendButton_clicked() {
    if (!m_inCombat || !m_combatState) return;

    performPlayerAction([&](QString& result) {
        m_combatActions->defend(result);
    });
}

void ArenaDialog::on_spellButton_clicked() {
    if (!m_inCombat || !m_combatState || !m_combatActions) return;

    // Pick a spell based on the first living party member's mana
    // Use Fireball as default arena spell
    const QString spellName = "Fireball";
    int targetIdx = selectedMonsterIdx;
    if (targetIdx < 0) {
        for (int i = 0; i < m_combatState->participantCount(); ++i) {
            if (!m_combatState->participant(i).isPlayer && m_combatState->participant(i).isAlive) {
                targetIdx = i;
                break;
            }
        }
    }
    if (targetIdx < 0) return;

    performPlayerAction([&](QString& result) {
        int dmg = m_combatActions->castSpellBySchool(targetIdx, spellName, result);
        if (dmg > 0 && !m_combatState->participant(targetIdx).isAlive) {
            waveKills++;
            logMessage(QString("Monster killed! (%1 total)").arg(waveKills));
            selectedMonsterIdx = -1;
        }
    });
}

void ArenaDialog::on_useItemButton_clicked() {
    if (!m_inCombat || !m_combatState || !m_combatActions) return;

    // Find the current player in the combat participants
    int playerIdx = -1;
    int currentIdx = m_combatState->initiativeOrder()[m_combatState->currentTurnIndex()];
    if (m_combatState->participant(currentIdx).isPlayer) {
        playerIdx = currentIdx;
    }
    if (playerIdx < 0) return;

    // Find the corresponding party member index
    int partyIdx = -1;
    int monsterCount = 0;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        if (!m_combatState->participant(i).isPlayer) monsterCount++;
    }
    partyIdx = currentIdx - monsterCount;
    if (partyIdx < 0 || partyIdx >= gameStateManager::instance()->getPartyMembers().size()) return;

    Character &hero = gameStateManager::instance()->getPartyMembers()[partyIdx];
    if (hero.inventory.isEmpty()) {
        logMessage(QString("%1 has no items.").arg(hero.name));
        return;
    }

    // Find a healing item (simple heuristic: name contains "Potion" or "Heal")
    int itemIdx = -1;
    for (int i = 0; i < hero.inventory.size(); ++i) {
        const QString &name = hero.inventory[i].name;
        if (name.contains("Potion") || name.contains("Heal") || name.contains("Bandage")) {
            itemIdx = i;
            break;
        }
    }
    if (itemIdx < 0) {
        logMessage(QString("%1 has no usable items.").arg(hero.name));
        return;
    }

    performPlayerAction([&](QString& result) {
        bool used = m_combatActions->useItem(itemIdx, hero, result);
        if (used) {
            logMessage(QString("%1 uses %2.").arg(hero.name, hero.inventory.value(itemIdx).name));
        } else {
            logMessage(result);
        }
    });
}

void ArenaDialog::on_fleeButton_clicked() {
    if (!m_inCombat) return;
    logMessage("You flee the arena!");
    recordScore();
    m_inCombat = false;
    m_startBtn->setEnabled(true);
    m_nextWaveBtn->setEnabled(false);
    updateUI();
}

void ArenaDialog::on_nextWaveButton_clicked() {
    if (m_currentWave < 10) {
        startWave(m_currentWave + 1);
    }
}

void ArenaDialog::advanceCombat() {
    if (!m_inCombat || !m_combatState) return;

    // Check if all monsters are dead
    bool allMonstersDead = true;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        if (!m_combatState->participant(i).isPlayer && m_combatState->participant(i).isAlive) {
            allMonstersDead = false;
            break;
        }
    }

    if (allMonstersDead) {
        handleVictory();
        return;
    }

    // Check if all players are dead
    bool allPlayersDead = true;
    for (int i = 0; i < m_combatState->participantCount(); ++i) {
        if (m_combatState->participant(i).isPlayer && m_combatState->participant(i).isAlive) {
            allPlayersDead = false;
            break;
        }
    }

    if (allPlayersDead) {
        handleDefeat();
        return;
    }

    // Tick status effects every advance so poison/fire/blind work
    QStringList statusMsgs = m_combatActions->tickStatusEffects();
    for (const QString& msg : statusMsgs) logMessage(msg);

    // Process monster turns
    if (!m_combatActions->isPlayerTurn()) {
        QString aiResult = m_monsterAI->takeTurn();
        logMessage(aiResult);

        // Track damage taken — combat participants have monsters first,
        // so party member index = combat index - monster count
        int monsterCount = 0;
        for (int i = 0; i < m_combatState->participantCount(); ++i) {
            if (!m_combatState->participant(i).isPlayer) monsterCount++;
        }
        for (int i = 0; i < m_combatState->participantCount(); ++i) {
            if (m_combatState->participant(i).isPlayer) {
                int partyIdx = i - monsterCount;
                if (partyIdx >= 0 && partyIdx < gameStateManager::instance()->getPartyMembers().size()) {
                    Character& c = gameStateManager::instance()->getPartyMembers()[partyIdx];
                    int damage = c.hp - m_combatState->participant(i).hp;
                    if (damage > 0) waveDamageTaken += damage;
                }
            }
        }

        // Advance turn
        if (!m_combatState->nextTurn()) {
            // Round over — tick status effects (poison, fire, blind, etc.)
            m_combatState->startRound();
            waveRounds++;
        }
        updateUI();
        return;
    }

    // Player turn
    updateUI();
}

void ArenaDialog::handleVictory() {
    m_inCombat = false;

    // Record wave result
    Arena::WaveResult result;
    result.wave = m_currentWave;
    result.monstersKilled = waveKills;
    result.damageTaken = waveDamageTaken;
    result.rounds = waveRounds;
    result.cleared = true;
    m_waveResults.append(result);

    m_totalKills += waveKills;
    m_totalDamageTaken += waveDamageTaken;
    m_totalRounds += waveRounds;

    // Update ArenaWavesCleared so achievements can trigger
    int clearedWaves = 0;
    for (const Arena::WaveResult &r : m_waveResults) {
        if (r.cleared) clearedWaves++;
    }
    gameStateManager::instance()->setGameValue("ArenaWavesCleared", clearedWaves);

    logMessage(QString("Wave %1 cleared! Kills: %2, Damage taken: %3, Rounds: %4")
               .arg(m_currentWave).arg(waveKills).arg(waveDamageTaken).arg(waveRounds));

    if (m_currentWave >= 10) {
        m_arenaComplete = true;
        logMessage("You have conquered the Arena!");
        recordScore();
    } else {
        m_nextWaveBtn->setEnabled(true);
    }

    m_startBtn->setEnabled(true);
    updateUI();
}

void ArenaDialog::handleDefeat() {
    m_inCombat = false;
    logMessage("The arena claims another challenger.");

    // Record partial result
    Arena::WaveResult result;
    result.wave = m_currentWave;
    result.monstersKilled = waveKills;
    result.damageTaken = waveDamageTaken;
    result.rounds = waveRounds;
    result.cleared = false;
    m_waveResults.append(result);

    m_totalKills += waveKills;
    m_totalDamageTaken += waveDamageTaken;
    m_totalRounds += waveRounds;

    recordScore();
    m_startBtn->setEnabled(true);
    m_nextWaveBtn->setEnabled(false);
    updateUI();
}

void ArenaDialog::performPlayerAction(const std::function<void(QString&)> &action) {
    if (!m_inCombat || !m_combatActions || !m_turnEngine) return;
    if (!m_combatActions->isPlayerTurn()) return;

    QString result;
    action(result);
    logMessage(result);

    // Process deaths
    QStringList deaths = m_deathHandler->processDeaths();
    for (const QString& d : deaths) logMessage(d);

    // Mark as acted and advance turn
    int currentIdx = m_turnEngine->currentParticipantIndex();
    if (currentIdx >= 0) m_combatState->markActed(currentIdx);
    m_combatState->nextTurn();

    updateUI();
    advanceCombat();
}

void ArenaDialog::recordScore() {
    Arena::Score score;
    score.heroName = gameStateManager::instance()->getGameValue("CurrentCharacterName").toString();
    score.wavesCleared = m_waveResults.size();
    for (const Arena::WaveResult& r : m_waveResults) {
        if (r.cleared) score.wavesCleared++;
    }
    score.totalKills = m_totalKills;
    score.totalDamageTaken = m_totalDamageTaken;
    score.totalRounds = m_totalRounds;
    score.score = Arena::totalScore(m_waveResults);

    // Save to Hall of Records
    QVariantMap arenaRecord;
    arenaRecord["type"] = "arena";
    arenaRecord["heroName"] = score.heroName;
    arenaRecord["score"] = score.score;
    arenaRecord["wavesCleared"] = score.wavesCleared;
    arenaRecord["totalKills"] = score.totalKills;
    arenaRecord["totalDamageTaken"] = score.totalDamageTaken;
    arenaRecord["totalRounds"] = score.totalRounds;

    // Append to arena records in game state
    QVariantList arenaRecords = gameStateManager::instance()->getGameValue("ArenaRecords").toList();
    arenaRecords.append(arenaRecord);
    gameStateManager::instance()->setGameValue("ArenaRecords", arenaRecords);

    logMessage(QString("Final score: %1").arg(score.score));
}

void ArenaDialog::updateUI() {
    // Wave label
    if (m_inCombat) {
        m_waveLabel->setText(QString("Wave %1 / 10").arg(m_currentWave));
    } else if (m_arenaComplete) {
        m_waveLabel->setText("Arena Complete!");
    } else {
        m_waveLabel->setText("Welcome to the Arena");
    }

    // Score label
    int currentScore = Arena::totalScore(m_waveResults);
    m_scoreLabel->setText(QString("Score: %1 | Kills: %2 | Damage: %3")
                          .arg(currentScore).arg(m_totalKills).arg(m_totalDamageTaken));

    // Party HP
    if (m_combatState && m_inCombat) {
        QStringList hpParts;
        for (int i = 0; i < m_combatState->participantCount(); ++i) {
            const CombatParticipant& p = m_combatState->participant(i);
            if (p.isPlayer && p.isAlive) {
                hpParts << QString("%1: %2/%3").arg(p.name).arg(p.hp).arg(p.maxHp);
            }
        }
        m_partyHpLabel->setText("Party: " + hpParts.join(" | "));

        // Monster HP
        QStringList monsterParts;
        for (int i = 0; i < m_combatState->participantCount(); ++i) {
            const CombatParticipant& p = m_combatState->participant(i);
            if (!p.isPlayer && p.isAlive) {
                monsterParts << QString("%1: %2/%3").arg(p.name).arg(p.hp).arg(p.maxHp);
            }
        }
        m_monsterHpLabel->setText("Monsters: " + monsterParts.join(" | "));

        // Target list — show selected marker
        m_targetList->clear();
        for (int i = 0; i < m_combatState->participantCount(); ++i) {
            const CombatParticipant& p = m_combatState->participant(i);
            if (!p.isPlayer && p.isAlive) {
                QString label = QString("%1 (%2/%3)").arg(p.name).arg(p.hp).arg(p.maxHp);
                if (i == selectedMonsterIdx) label = "▶ " + label;
                QListWidgetItem* item = new QListWidgetItem(label, m_targetList);
                item->setData(Qt::UserRole, i);
                if (i == selectedMonsterIdx) {
                    item->setSelected(true);
                    m_targetList->setCurrentItem(item);
                }
            }
        }
    } else {
        m_partyHpLabel->clear();
        m_monsterHpLabel->clear();
        m_targetList->clear();
    }

    // Button states
    bool playerTurn = m_inCombat && m_combatActions && m_combatActions->isPlayerTurn();
    m_startBtn->setEnabled(!m_inCombat && !m_arenaComplete);
    m_attackBtn->setEnabled(playerTurn);
    m_defendBtn->setEnabled(playerTurn);
    m_spellBtn->setEnabled(playerTurn);
    m_useItemBtn->setEnabled(playerTurn);
    m_fleeBtn->setEnabled(m_inCombat);
    m_nextWaveBtn->setEnabled(!m_inCombat && m_currentWave > 0 && m_currentWave < 10 && !m_waveResults.isEmpty());
}

void ArenaDialog::logMessage(const QString& msg) {
    m_messageLog->addItem(msg);
    m_messageLog->scrollToBottom();
}

void ArenaDialog::clearCombat() {
    if (m_combatState) m_combatState->clear();
    selectedMonsterIdx = -1;
}

// ensureArenaInit is called at the start of startWave.
