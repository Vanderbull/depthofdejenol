#include "PartyManager.h"
#include "src/core/SoundEffects.h"
#include "src/core/LevelTable.h"
#include "src/spell_casting/SpellBook.h"
#include "src/journal_dialog/JournalDialog.h"
#include <QRandomGenerator>

// The constructor must match the header's signature
PartyManager::PartyManager(QObject *parent) 
    : QObject(parent) 
{
    // Initialize with an empty party if needed
    m_party.sharedGold = 0;
}

Character& PartyManager::getMember(int index) {
    if (m_party.members.isEmpty()) {
        static Character nullChar;
        return nullChar;
    }
    if (index >= 0 && index < m_party.members.size()) {
        return m_party.members[index];
    }
    return m_party.members[0];
}

bool PartyManager::isWholePartyDead() const {
    if (m_party.members.isEmpty()) return true;
    for (const auto& member : m_party.members) {
        if (member.isAlive) return false;
    }
    return true;
}

void PartyManager::addExperienceToParty(int totalXp) {
    QList<int> livingIndices;
    for (int i = 0; i < m_party.members.size(); ++i) {
        if (m_party.members[i].isAlive) livingIndices.append(i);
    }
    if (livingIndices.isEmpty()) return;

    int xpPerMember = totalXp / livingIndices.size();
    for (int index : livingIndices) {
        addExperienceToCharacter(index, xpPerMember);
    }
}

void PartyManager::addExperienceToCharacter(int index, int amount) {
    if (index < 0 || index >= m_party.members.size()) return;

    Character& c = m_party.members[index];
    c.experience += amount;

    // Use LevelTable for XP thresholds
    LevelTable& lt = LevelTable::instance();
    int newLevel = lt.levelForXp(c.experience);
    if (newLevel > c.level) {
        c.level = newLevel;
        applyLevelUpGains(c);
        emit leveledUp(index, c.level);
    }

    emit partyUpdated();
}

void PartyManager::applyLevelUpGains(Character& c) {
    SoundEffects::instance()->play(SoundEffects::Type::LevelUp);

    JournalDialog::addEntry(QStringLiteral("Combat"),
        QStringLiteral("%1 reached level %2!").arg(c.name).arg(c.level));

    // Base HP gain: 2-6 per level
    int hpGain = 2 + QRandomGenerator::global()->bounded(5);

    // Guild-based HP bonuses: Warriors and Paladins get extra HP
    if (c.guildLevel("Warrior") > 0 || c.guildLevel("Paladin") > 0) {
        hpGain += 2;
    }
    // Mages and Wizards get less HP but more mana
    if (c.guildLevel("Mage") > 0 || c.guildLevel("Wizard") > 0) {
        hpGain = qMax(1, hpGain - 1);
    }

    c.maxHp += hpGain;
    c.hp = c.maxHp;  // Full heal on level up

    // Mana gain for casters (Intelligence or Wisdom based)
    if (c.intelligence >= 10 || c.wisdom >= 10) {
        int manaGain = 1 + QRandomGenerator::global()->bounded(4);

        // Guild-based mana bonuses: Mages and Wizards get extra mana
        if (c.guildLevel("Mage") > 0 || c.guildLevel("Wizard") > 0) {
            manaGain += 2;
        }
        // Healers get bonus mana from Wisdom
        if (c.guildLevel("Healer") > 0) {
            manaGain += 1;
        }

        c.maxMana += manaGain;
        c.mana = c.maxMana;  // Full restore on level up
    }

    // Stat point: +1 to a random stat every 2 levels
    if (c.level % 2 == 0) {
        int statRoll = QRandomGenerator::global()->bounded(6);
        switch (statRoll) {
        case 0: c.strength++; break;
        case 1: c.intelligence++; break;
        case 2: c.wisdom++; break;
        case 3: c.constitution++; break;
        case 4: c.charisma++; break;
        case 5: c.dexterity++; break;
        }
    }

    // Guild-based stat bonuses: Warriors get +1 STR, Mages get +1 INT
    if (c.guildLevel("Warrior") > 0 && c.level % 3 == 0) {
        c.strength++;
    }
    if (c.guildLevel("Mage") > 0 && c.level % 3 == 0) {
        c.intelligence++;
    }

    // Spell learning: check each guild the character belongs to for newly
    // available spells at the new level.
    if (SpellBook::instance().isLoaded()) {
        for (auto it = c.guildLevels.constBegin(); it != c.guildLevels.constEnd(); ++it) {
            const QString& guildName = it.key();
            int guildLvl = it.value();
            QList<SpellDef> newSpells = SpellBook::instance().newlyLearned(c, guildName, guildLvl);
            for (const SpellDef& s : newSpells) {
                qDebug() << c.name << "learned" << s.name << "from" << guildName;
            }
        }
    }
}

void PartyManager::updateMemberStatus(int index, bool isAlive) {
    if (index >= 0 && index < m_party.members.size()) {
        //m_party.members[index].isAlive = isAlive;
        if (isAlive) {
            m_party.members[index].removeStatus(StatusFlag::Dead);
        } else {
            m_party.members[index].addStatus(StatusFlag::Dead);
        }
        emit partyUpdated();
    }
}
