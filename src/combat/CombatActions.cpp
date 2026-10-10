#include "CombatActions.h"
#include "src/core/GameConstants.h"
#include "src/core/SoundEffects.h"
#include "src/items/ItemDatabase.h"
#include <QRandomGenerator>

CombatActions::CombatActions(CombatState* state, TurnEngine* engine)
    : m_state(state), m_engine(engine) {}

bool CombatActions::isPlayerTurn() const {
    if (!m_state || !m_engine) return false;
    int idx = m_engine->currentParticipantIndex();
    if (idx < 0) return false;
    return m_state->participant(idx).isPlayer;
}

int CombatActions::attack(int targetIndex, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return -1; }
    int attackerIdx = m_engine->currentParticipantIndex();
    if (attackerIdx < 0) { result = "No attacker"; return -1; }
    if (targetIndex < 0 || targetIndex >= m_state->participantCount()) {
        result = "Invalid target"; return -1;
    }

    CombatParticipant& attacker = m_state->participant(attackerIdx);
    CombatParticipant& target = m_state->participant(targetIndex);

    if (!target.isAlive) { result = "Target is already dead"; return -1; }

    // Confusion: 25% chance to hit a random ally instead
    if (attacker.confusionDuration > 0 && QRandomGenerator::global()->bounded(100) < 25) {
        // Find a random living ally (same type as attacker)
        QList<int> allies;
        for (int i = 0; i < m_state->participantCount(); ++i) {
            if (i != attackerIdx && m_state->participant(i).isAlive && m_state->participant(i).isPlayer == attacker.isPlayer) {
                allies.append(i);
            }
        }
        if (!allies.isEmpty()) {
            int allyIdx = allies[QRandomGenerator::global()->bounded(allies.size())];
            CombatParticipant& ally = m_state->participant(allyIdx);
            int damage = qMax(1, attacker.att * attacker.swings * attacker.damageMod / 100);
            if (ally.isDefending) damage = qMax(1, damage / 2);
            ally.hp -= damage;
            if (ally.hp <= 0) {
                ally.hp = 0;
                ally.isAlive = false;
                result = QString("%1 is confused and hits %2 for %3 damage — %4 is slain!")
                    .arg(attacker.name).arg(ally.name).arg(damage).arg(ally.name);
            } else {
                result = QString("%1 is confused and hits %2 for %3 damage!")
                    .arg(attacker.name).arg(ally.name).arg(damage);
            }
            return damage;
        }
    }

    // To-hit roll: d20 + attacker.dex/2 + attacker.level vs 10 + target.def + target.level
    // Blind reduces to-hit by 5
    int roll = QRandomGenerator::global()->bounded(1, 21);
    int blindPenalty = (attacker.statusFlags & GameConstants::Blinded) ? 5 : 0;
    int toHit = roll + attacker.dex / 2 + attacker.level - blindPenalty;
    int targetAC = 10 + target.def + target.level;

    if (roll == 1) {
        result = QString("%1 fumbles the attack!").arg(attacker.name);
        SoundEffects::instance()->play(SoundEffects::Type::Miss);
        return -1;
    }
    if (roll == 20 || toHit >= targetAC) {
        // Hit: damage = att * swings * damageMod/100 * (1 + levelScale * level / 100)
        double levelMultiplier = 1.0 + (attacker.levelScale * attacker.level) / 100.0;
        int baseDamage = qMax(1, static_cast<int>(attacker.att * attacker.swings * attacker.damageMod / 100.0 * levelMultiplier));
        int damage = baseDamage + QRandomGenerator::global()->bounded(0, 5);
        if (roll == 20) damage *= 2;  // crit
        if (target.isDefending) damage = qMax(1, damage / 2);  // stance halves damage
        target.hp -= damage;
        if (target.hp <= 0) {
            target.hp = 0;
            target.isAlive = false;
            result = QString("%1 hits %2 for %3 damage — %4 is slain!")
                .arg(attacker.name).arg(target.name).arg(damage).arg(target.name);
            SoundEffects::instance()->play(SoundEffects::Type::Death);
        } else {
            result = QString("%1 hits %2 for %3 damage.")
                .arg(attacker.name).arg(target.name).arg(damage);
            SoundEffects::instance()->play(roll == 20 ? SoundEffects::Type::CriticalHit : SoundEffects::Type::Hit);
        }
        return damage;
    } else {
        result = QString("%1 misses %2 (roll %3 vs AC %4).")
            .arg(attacker.name).arg(target.name).arg(roll).arg(targetAC);
        SoundEffects::instance()->play(SoundEffects::Type::Miss);
        return -1;
    }
}

void CombatActions::defend(QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return; }
    int idx = m_engine->currentParticipantIndex();
    if (idx < 0) { result = "No defender"; return; }
    CombatParticipant& defender = m_state->participant(idx);
    defender.hasActed = true;
    // A defensive stance halves all damage taken until the next round.
    defender.isDefending = true;
    result = QString("%1 takes a defensive stance.").arg(defender.name);
}

int CombatActions::castSpell(int targetIndex, int spellPower, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return -1; }
    SoundEffects::instance()->play(SoundEffects::Type::SpellCast);
    int casterIdx = m_engine->currentParticipantIndex();
    if (casterIdx < 0) { result = "No caster"; return -1; }
    if (targetIndex < 0 || targetIndex >= m_state->participantCount()) {
        result = "Invalid target"; return -1;
    }

    CombatParticipant& caster = m_state->participant(casterIdx);
    CombatParticipant& target = m_state->participant(targetIndex);

    if (!target.isAlive) { result = "Target is already dead"; return -1; }

    // Spell damage: spellPower + caster.att
    int damage = spellPower + caster.att;
    target.hp -= damage;
    if (target.hp <= 0) {
        target.hp = 0;
        target.isAlive = false;
        result = QString("%1 casts a spell on %2 for %3 damage — %4 is slain!")
            .arg(caster.name).arg(target.name).arg(damage).arg(target.name);
    } else {
        result = QString("%1 casts a spell on %2 for %3 damage.")
            .arg(caster.name).arg(target.name).arg(damage);
    }
    return damage;
}

int CombatActions::castSpellAdvanced(int targetIndex, const QString& spellName,
                                     int manaCost, const QString& damageRange,
                                     bool isAoE, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return -1; }
    int casterIdx = m_engine->currentParticipantIndex();
    if (casterIdx < 0) { result = "No caster"; return -1; }
    if (targetIndex < 0 || targetIndex >= m_state->participantCount()) {
        result = "Invalid target"; return -1;
    }

    CombatParticipant& caster = m_state->participant(casterIdx);
    CombatParticipant& target = m_state->participant(targetIndex);

    if (!target.isAlive) { result = "Target is already dead"; return -1; }

    // Check mana
    if (caster.mana < manaCost) {
        result = QString("%1 does not have enough mana for %2 (need %3, have %4).")
            .arg(caster.name).arg(spellName).arg(manaCost).arg(caster.mana);
        return -1;
    }

    // Deduct mana
    caster.mana -= manaCost;

    // Parse damage range "min-max"
    int minDmg = 0, maxDmg = 0;
    QStringList parts = damageRange.split('-');
    if (parts.size() == 2) {
        minDmg = parts[0].toInt();
        maxDmg = parts[1].toInt();
    } else {
        minDmg = damageRange.toInt();
        maxDmg = minDmg;
    }
    if (maxDmg < minDmg) maxDmg = minDmg;

    // Roll damage
    int damage = minDmg + QRandomGenerator::global()->bounded(maxDmg - minDmg + 1);

    // Apply damage to target(s)
    int totalDamage = 0;
    QStringList hitNames;

    if (isAoE) {
        // Hit all living participants of the same type as target
        bool targetIsPlayer = target.isPlayer;
        for (int i = 0; i < m_state->participantCount(); ++i) {
            CombatParticipant& p = m_state->participant(i);
            if (p.isAlive && p.isPlayer == targetIsPlayer) {
                p.hp -= damage;
                if (p.hp <= 0) {
                    p.hp = 0;
                    p.isAlive = false;
                }
                totalDamage += damage;
                hitNames.append(p.name);
            }
        }
        result = QString("%1 casts %2 on %3 for %4 damage each!")
            .arg(caster.name).arg(spellName).arg(hitNames.join(", ")).arg(damage);
    } else {
        // Single target
        target.hp -= damage;
        if (target.hp <= 0) {
            target.hp = 0;
            target.isAlive = false;
            result = QString("%1 casts %2 on %3 for %4 damage — %5 is slain!")
                .arg(caster.name).arg(spellName).arg(target.name).arg(damage).arg(target.name);
        } else {
            result = QString("%1 casts %2 on %3 for %4 damage.")
                .arg(caster.name).arg(spellName).arg(target.name).arg(damage);
        }
        totalDamage = damage;
    }

    return totalDamage;
}

int CombatActions::applySpellDamage(int targetIndex, const QString& spellName,
                                    int damage, bool isAoE, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return -1; }
    if (targetIndex < 0 || targetIndex >= m_state->participantCount()) {
        result = "Invalid target"; return -1;
    }

    CombatParticipant& target = m_state->participant(targetIndex);
    if (!target.isAlive) { result = "Target is already dead"; return -1; }

    int totalDamage = 0;
    QStringList hitNames;

    if (isAoE) {
        bool targetIsPlayer = target.isPlayer;
        for (int i = 0; i < m_state->participantCount(); ++i) {
            CombatParticipant& p = m_state->participant(i);
            if (p.isAlive && p.isPlayer == targetIsPlayer) {
                int dmg = damage;
                if (p.isDefending) dmg = qMax(1, dmg / 2);
                p.hp -= dmg;
                if (p.hp <= 0) { p.hp = 0; p.isAlive = false; }
                totalDamage += dmg;
                hitNames.append(p.name);
            }
        }
        result = QString("%1 hits %2 for %3 damage each!")
            .arg(spellName).arg(hitNames.join(", ")).arg(damage);
    } else {
        int dmg = damage;
        if (target.isDefending) dmg = qMax(1, dmg / 2);
        target.hp -= dmg;
        if (target.hp <= 0) {
            target.hp = 0;
            target.isAlive = false;
            result = QString("%1 hits %2 for %3 damage — %2 is slain!")
                .arg(spellName).arg(target.name).arg(dmg);
        } else {
            result = QString("%1 hits %2 for %3 damage.")
                .arg(spellName).arg(target.name).arg(dmg);
        }
        totalDamage = dmg;
    }

    return totalDamage;
}

int CombatActions::castHeal(int targetIndex, int healAmount, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return -1; }
    int casterIdx = m_engine->currentParticipantIndex();
    if (casterIdx < 0) { result = "No caster"; return -1; }
    if (targetIndex < 0 || targetIndex >= m_state->participantCount()) {
        result = "Invalid target"; return -1;
    }

    CombatParticipant& caster = m_state->participant(casterIdx);
    CombatParticipant& target = m_state->participant(targetIndex);

    if (!target.isAlive) { result = "Target is already dead"; return -1; }

    int actualHeal = qMin(healAmount, target.maxHp - target.hp);
    target.hp += actualHeal;

    result = QString("%1 heals %2 for %3 HP.")
        .arg(caster.name).arg(target.name).arg(actualHeal);
    return actualHeal;
}

bool CombatActions::hasEnoughMana(int manaCost) const {
    if (!m_state || !m_engine) return false;
    int casterIdx = m_engine->currentParticipantIndex();
    if (casterIdx < 0) return false;
    return m_state->participant(casterIdx).mana >= manaCost;
}

void CombatActions::applyStatus(int targetIndex, uint statusFlag, int duration, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return; }
    if (targetIndex < 0 || targetIndex >= m_state->participantCount()) {
        result = "Invalid target"; return;
    }

    CombatParticipant& target = m_state->participant(targetIndex);
    target.statusFlags |= statusFlag;

    // Set duration based on status type
    if (statusFlag & GameConstants::Poisoned) {
        target.poisonDuration = duration;
    } else if (statusFlag & GameConstants::Blinded) {
        target.blindDuration = duration;
    } else if (statusFlag & GameConstants::OnFire) {
        target.fireDuration = duration;
    }

    QString statusName;
    if (statusFlag & GameConstants::Poisoned) statusName = "Poisoned";
    else if (statusFlag & GameConstants::Blinded) statusName = "Blinded";
    else if (statusFlag & GameConstants::OnFire) statusName = "On Fire";
    else statusName = "Affected";

    result = QString("%1 is now %2 for %3 rounds!")
        .arg(target.name).arg(statusName).arg(duration);
}

bool CombatActions::hasStatus(int index, uint statusFlag) const {
    if (!m_state || index < 0 || index >= m_state->participantCount()) return false;
    return (m_state->participant(index).statusFlags & statusFlag) != 0;
}

int CombatActions::getStatusDuration(int index, uint statusFlag) const {
    if (!m_state || index < 0 || index >= m_state->participantCount()) return 0;
    const CombatParticipant& p = m_state->participant(index);
    if (statusFlag & GameConstants::Poisoned) return p.poisonDuration;
    if (statusFlag & GameConstants::Blinded) return p.blindDuration;
    if (statusFlag & GameConstants::OnFire) return p.fireDuration;
    return 0;
}

QStringList CombatActions::tickStatusEffects() {
    QStringList messages;
    if (!m_state) return messages;

    for (int i = 0; i < m_state->participantCount(); ++i) {
        CombatParticipant& p = m_state->participant(i);
        if (!p.isAlive) continue;

        // Poison DoT: 1-3 damage per round
        if (p.statusFlags & GameConstants::Poisoned) {
            int damage = 1 + QRandomGenerator::global()->bounded(3);
            p.hp -= damage;
            messages.append(QString("%1 takes %2 poison damage!").arg(p.name).arg(damage));
            if (p.hp <= 0) {
                p.hp = 0;
                p.isAlive = false;
                messages.append(QString("%1 dies from poison!").arg(p.name));
                continue;
            }
            p.poisonDuration--;
            if (p.poisonDuration <= 0) {
                p.statusFlags &= ~GameConstants::Poisoned;
                messages.append(QString("%1 is no longer poisoned.").arg(p.name));
            }
        }

        // On Fire DoT: 2-5 damage per round
        if (p.statusFlags & GameConstants::OnFire) {
            int damage = 2 + QRandomGenerator::global()->bounded(4);
            p.hp -= damage;
            messages.append(QString("%1 takes %2 fire damage!").arg(p.name).arg(damage));
            if (p.hp <= 0) {
                p.hp = 0;
                p.isAlive = false;
                messages.append(QString("%1 dies from fire!").arg(p.name));
                continue;
            }
            p.fireDuration--;
            if (p.fireDuration <= 0) {
                p.statusFlags &= ~GameConstants::OnFire;
                messages.append(QString("%1 is no longer on fire.").arg(p.name));
            }
        }

        // Blind duration
        if (p.statusFlags & GameConstants::Blinded) {
            p.blindDuration--;
            if (p.blindDuration <= 0) {
                p.statusFlags &= ~GameConstants::Blinded;
                messages.append(QString("%1 is no longer blinded.").arg(p.name));
            }
        }

        // Confusion duration
        if (p.confusionDuration > 0) {
            p.confusionDuration--;
            if (p.confusionDuration <= 0) {
                messages.append(QString("%1 is no longer confused.").arg(p.name));
            }
        }

        // Regeneration: monsters with canRegenerate heal each round.
        if (p.canRegenerate && p.isAlive && p.hp < p.maxHp) {
            int heal = qMin(p.regenerateAmount, p.maxHp - p.hp);
            p.hp += heal;
            messages.append(QString("%1 regenerates %2 HP.").arg(p.name).arg(heal));
        }
    }

    return messages;
}

bool CombatActions::isBlinded(int index) const {
    return hasStatus(index, GameConstants::Blinded);
}

bool CombatActions::isConfused(int index) const {
    if (!m_state || index < 0 || index >= m_state->participantCount()) return false;
    return m_state->participant(index).confusionDuration > 0;
}

bool CombatActions::isPoisoned(int index) const {
    return hasStatus(index, GameConstants::Poisoned);
}

bool CombatActions::isOnFire(int index) const {
    return hasStatus(index, GameConstants::OnFire);
}

void CombatActions::cureStatus(int targetIndex, uint statusFlag, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return; }
    if (targetIndex < 0 || targetIndex >= m_state->participantCount()) {
        result = "Invalid target"; return;
    }

    CombatParticipant& target = m_state->participant(targetIndex);
    if ((target.statusFlags & statusFlag) == 0) {
        result = QString("%1 is not affected.").arg(target.name);
        return;
    }

    target.statusFlags &= ~statusFlag;
    if (statusFlag & GameConstants::Poisoned) target.poisonDuration = 0;
    if (statusFlag & GameConstants::Blinded) target.blindDuration = 0;
    if (statusFlag & GameConstants::OnFire) target.fireDuration = 0;

    QString statusName;
    if (statusFlag & GameConstants::Poisoned) statusName = "poisoned";
    else if (statusFlag & GameConstants::Blinded) statusName = "blinded";
    else if (statusFlag & GameConstants::OnFire) statusName = "on fire";
    else statusName = "affected";

    result = QString("%1 is no longer %2.").arg(target.name).arg(statusName);
}

bool CombatActions::useItem(int itemIndex, Character& user, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return false; }
    int userIdx = m_engine->currentParticipantIndex();
    if (userIdx < 0) { result = "No user"; return false; }

    if (itemIndex < 0 || itemIndex >= user.inventory.size()) {
        result = "No such item.";
        return false;
    }

    const QString itemName = user.inventory[itemIndex].name;
    const QString lower = itemName.toLower();

    // Resolve the effect on the combat participant, then consume the item via
    // Character::useConsumable so inventory HP/charges stay authoritative.
    CombatParticipant& participant = m_state->participant(userIdx);

    if (lower.contains("healing") || lower.contains("health")) {
        const ItemDef* def = ItemDatabase::instance().byName(itemName);
        int healAmount = 10 + (def ? def->spellLvl * 5 : 0);
        int actual = qMin(healAmount, participant.maxHp - participant.hp);
        participant.hp += actual;
        result = QString("%1 uses %2 and restores %3 HP.")
                     .arg(participant.name).arg(itemName).arg(actual);
    } else if (lower.contains("mana")) {
        const ItemDef* def = ItemDatabase::instance().byName(itemName);
        int manaAmount = 10 + (def ? def->spellLvl * 5 : 0);
        int actual = qMin(manaAmount, participant.maxMana - participant.mana);
        participant.mana += actual;
        result = QString("%1 uses %2 and restores %3 mana.")
                     .arg(participant.name).arg(itemName).arg(actual);
    } else if (lower.contains("cure") || lower.contains("poison")) {
        bool had = (participant.statusFlags & GameConstants::Poisoned) != 0;
        participant.statusFlags &= ~GameConstants::Poisoned;
        participant.poisonDuration = 0;
        result = had ? QString("%1 uses %2 — the poison is cured.")
                           .arg(participant.name).arg(itemName)
                     : QString("%1 uses %2 — nothing to cure.")
                           .arg(participant.name).arg(itemName);
    } else {
        result = QString("%1 uses %2.").arg(participant.name).arg(itemName);
    }

    // Consume: apply the real inventory effect (decrements charges / removes).
    QString effect;
    user.useConsumable(itemIndex, effect);

    participant.hasActed = true;
    return true;
}

bool CombatActions::flee(QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return false; }
    int idx = m_engine->currentParticipantIndex();
    if (idx < 0) { result = "No fleer"; return false; }

    CombatParticipant& fleer = m_state->participant(idx);

    // Flee roll: d20 + speed vs a base of 12, adjusted by the pursuers. A
    // faster pack is harder to outrun and being surrounded is harder still,
    // but the speed term is capped so an extreme gap does not make escape
    // impossible outright.
    int roll = QRandomGenerator::global()->bounded(1, 21);
    int fastestPursuer = 0;
    int pursuers = 0;
    for (int i = 0; i < m_state->participantCount(); ++i) {
        const CombatParticipant& p = m_state->participant(i);
        if (p.isAlive && p.isPlayer != fleer.isPlayer) {
            fastestPursuer = qMax(fastestPursuer, p.speed);
            pursuers++;
        }
    }
    int speedGap = qMax(0, fastestPursuer - fleer.speed);
    int escapeTarget = 12 + qMin(8, speedGap / 5) + qMax(0, pursuers - 1) * 2;
    int fleeRoll = roll + fleer.speed;

    if (fleeRoll >= escapeTarget) {
        // A fleeing monster leaves the fight for good; a fleeing player is
        // handled by the caller (the combat UI closes), so it must not be
        // marked dead here.
        if (!fleer.isPlayer) {
            fleer.hasFled = true;
            fleer.isAlive = false;
        }
        result = QString("%1 flees successfully!").arg(fleer.name);
        return true;
    } else {
        result = QString("%1 fails to flee!").arg(fleer.name);
        return false;
    }
}
