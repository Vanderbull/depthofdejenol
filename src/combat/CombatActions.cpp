#include "CombatActions.h"
#include "src/core/GameConstants.h"
#include "src/core/SoundEffects.h"
#include "src/items/ItemDatabase.h"
#include "src/items/ConsumableResolver.h"
#include "src/core/gameStateManager.h"
#include "src/spell_casting/SpellBook.h"
#include "src/spell_casting/SpellMechanics.h"
#include <QRandomGenerator>

CombatActions::CombatActions(CombatState* state, TurnEngine* engine)
    : m_state(state), m_engine(engine) {}

// Wrap a combat message in HTML so the log renders it in a distinct colour.
// The message log shows a QLabel per line and honours Qt rich text, so this
// is how crits and misses stand out from ordinary hits. Centralised here
// rather than inlined at every call site so the wording cannot drift apart.
QString CombatActions::stylize(const QString& message, const char* color) {
    return QString("<font color='%1'>%2</font>").arg(color, message);
}

QString CombatActions::attackMessage(int roll, const QString& attacker,
                                     const QString& target, int damage) {
    const QString body = QString("%1 hits %2 for %3 damage.")
                             .arg(attacker, target).arg(damage);
    if (roll == 20) return stylize(QString("CRITICAL — %1").arg(body), "red");
    return stylize(body, "orange");
}

QString CombatActions::missMessage(const QString& attacker, const QString& target,
                                   int roll, int targetAC) {
    return stylize(QString("%1 misses %2 (roll %3 vs AC %4).")
                       .arg(attacker, target).arg(roll).arg(targetAC),
                   "gray");
}

bool CombatActions::isPlayerTurn() const {
    if (!m_state || !m_engine) return false;
    int idx = m_engine->currentParticipantIndex();
    if (idx < 0) return false;
    return m_state->participant(idx).isPlayer;
}

int CombatActions::attackerSkill(const CombatParticipant& attacker) {
    // Level is the backbone; dexterity adds up to +6, then a hard ceiling so a
    // runaway dex cannot make every attack auto-hit.
    return attacker.level + qBound(0, attacker.dex / 3, 6);
}

int CombatActions::targetArmorClass(const CombatParticipant& target) {
    // Level is the backbone; raw defense is compressed (monster def runs past
    // 300) and capped at +8, so AC stays inside the d20 band at every depth.
    return 10 + target.level / 2 + qBound(0, target.def / 12, 8);
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

    // To-hit. Att/Def in this game's data live on wildly different scales: a
    // monster's raw `def` column runs past 300 while a character's armor
    // defense tops out near 50, and monster rows carry no usable Dexterity at
    // all. A raw `10 + def + level` AC therefore made deep floors impossible to
    // hit in EITHER direction (floor 15 needed a 744 on a d20). Both sides are
    // compressed into a bounded band instead: skill grows with level and
    // dexterity, AC with level and defense. Calibrated to a ~55-85% hit rate at
    // every depth — see the balance tests.
    // Blind reduces to-hit by 5.
    int roll = QRandomGenerator::global()->bounded(1, 21);
    int blindPenalty = (attacker.statusFlags & GameConstants::Blinded) ? 5 : 0;
    int toHit = roll + attackerSkill(attacker) - blindPenalty;
    int targetAC = targetArmorClass(target);

    if (roll == 1) {
        result = stylize(QString("%1 fumbles the attack!").arg(attacker.name), "gray");
        SoundEffects::instance()->play(SoundEffects::Type::Miss);
        return -1;
    }
    if (roll == 20 || toHit >= targetAC) {
        // Hit: damage = att * swings * damageMod/100 * (1 + levelScale * level / 100)
        double levelMultiplier = 1.0 + (attacker.levelScale * attacker.level) / 100.0;
        int baseDamage = qMax(1, static_cast<int>(attacker.att * attacker.swings * attacker.damageMod / 100.0 * levelMultiplier));
        int damage = baseDamage + QRandomGenerator::global()->bounded(0, 5);
        const bool isCrit = (roll == 20);
        if (isCrit) damage *= 2;  // crit
        if (target.isDefending) damage = qMax(1, damage / 2);  // stance halves damage
        target.hp -= damage;
        if (target.hp <= 0) {
            target.hp = 0;
            target.isAlive = false;
            result = QString("%1 hits %2 for %3 damage — %4 is slain!")
                .arg(attacker.name).arg(target.name).arg(damage).arg(target.name);
            SoundEffects::instance()->play(SoundEffects::Type::Death);
        } else {
            result = attackMessage(roll, attacker.name, target.name, damage);
            SoundEffects::instance()->play(isCrit ? SoundEffects::Type::CriticalHit : SoundEffects::Type::Hit);
        }
        // A natural 20 doubles damage but said nothing about it — the log now
        // says so, in a colour the player cannot miss. Misses are marked the
        // same way so the three outcomes read differently at a glance.
        return damage;
    } else {
        result = missMessage(attacker.name, target.name, roll, targetAC);
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

// Applies a spell's damage using the mechanic its school dictates. Fire spells
// splash to extra targets, cold slows, lightning chains, mind stuns/confuses.
int CombatActions::applySpellDamageBySchool(int targetIndex, const QString& spellName,
                                            int damage, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return -1; }
    if (targetIndex < 0 || targetIndex >= m_state->participantCount()) {
        result = "Invalid target"; return -1;
    }

    const SpellDef* spell = SpellBook::instance().byName(spellName);

    // Unknown spell: fall back to a plain single-target hit.
    if (!spell) {
        return applySpellDamage(targetIndex, spellName, damage, false, result);
    }

    const SpellMechanics::School school = SpellMechanics::schoolFor(spell->category);
    const bool targetIsPlayer = m_state->participant(targetIndex).isPlayer;

    // Instant Kill/Banish spells: deal massive damage instead of 9999 one-shot.
    if (spell->damage == "Instant Kill" || spell->damage == "Instant Banish") {
        const int casterIdx = m_engine->currentParticipantIndex();
        const int casterLevel = m_state->participant(casterIdx).level;
        const int massiveDmg = SpellMechanics::instantKillDamage(casterLevel);
        CombatParticipant& target = m_state->participant(targetIndex);
        target.hp -= massiveDmg;
        if (target.hp <= 0) { target.hp = 0; target.isAlive = false; }
        result = QString("%1 hits %2 for %3 damage!")
            .arg(spellName).arg(target.name).arg(massiveDmg);
        return massiveDmg;
    }

    // --- Damage: Poison/Wound apply a damage-over-time, Drain Life heals ---
    if (school == SpellMechanics::School::Damage) {
        CombatParticipant& tgt = m_state->participant(targetIndex);
        if (spellName == "Drain Life") {
            // Instant damage, half of it transferred to the caster.
            int dmg = qMax(1, damage);
            if (tgt.isDefending) dmg = qMax(1, dmg / 2);
            tgt.hp -= dmg;
            if (tgt.hp <= 0) { tgt.hp = 0; tgt.isAlive = false; }
            int healed = qMax(1, dmg / 2);
            const int casterIdx = m_engine->currentParticipantIndex();
            if (casterIdx >= 0 && casterIdx != targetIndex) {
                CombatParticipant& caster = m_state->participant(casterIdx);
                caster.hp = qMin(caster.maxHp, caster.hp + healed);
            }
            result = QString("%1 drains %2 life from %3 (%4 healed)!")
                .arg(spellName).arg(dmg).arg(tgt.name).arg(healed);
            return dmg;
        }

        // Poison / Wound: apply a poison-style damage-over-time via the
        // existing status tick, so tickStatusEffects() deals 1-3/round.
        const int duration = qMax(3, damage > 0 ? damage : 3);
        tgt.statusFlags |= GameConstants::Poisoned;
        tgt.poisonDuration = qMax(tgt.poisonDuration, duration);
        result = QString("%1 infects %2 — taking damage over %3 rounds!")
            .arg(spellName).arg(tgt.name).arg(duration);
        return 0;
    }

    // --- Fire: area of effect, +10% damage per extra target ---
    if (school == SpellMechanics::School::Fire && SpellMechanics::isAoe(*spell)) {
        const int targets = SpellMechanics::fireTargets(*spell);
        const int bonusPct = SpellMechanics::fireBonusPerTarget(*spell);
        const int perTarget = qMax(1, damage * (100 + bonusPct) / 100);

        int hit = 0;
        int total = 0;
        QStringList names;
        for (int i = 0; i < m_state->participantCount() && hit < targets; ++i) {
            CombatParticipant& p = m_state->participant(i);
            if (!p.isAlive || p.isPlayer != targetIsPlayer) continue;
            int dmg = perTarget;
            if (p.isDefending) dmg = qMax(1, dmg / 2);
            p.hp -= dmg;
            if (p.hp <= 0) { p.hp = 0; p.isAlive = false; }
            total += dmg;
            names.append(p.name);
            hit++;
        }
        result = QString("%1 erupts in fire, hitting %2 for %3 damage each!")
            .arg(spellName).arg(names.join(", ")).arg(perTarget);
        return total;
    }

    // --- Lightning: chains to extra targets, -20% damage per jump ---
    if (school == SpellMechanics::School::Lightning) {
        const int chains = SpellMechanics::lightningChainTargets(*spell);
        if (chains > 0) {
            const int falloff = SpellMechanics::lightningChainFalloff(*spell);
            int total = 0;
            QStringList names;
            int jump = 0;
            int dmg = damage;
            for (int i = 0; i < m_state->participantCount() && jump <= chains; ++i) {
                CombatParticipant& p = m_state->participant(i);
                if (!p.isAlive || p.isPlayer != targetIsPlayer) continue;
                int hitDmg = qMax(1, dmg * (100 - falloff * jump) / 100);
                if (p.isDefending) hitDmg = qMax(1, hitDmg / 2);
                p.hp -= hitDmg;
                if (p.hp <= 0) { p.hp = 0; p.isAlive = false; }
                total += hitDmg;
                names.append(QString("%1 (%2)").arg(p.name).arg(hitDmg));
                jump++;
            }
            result = QString("%1 arcs between %2 for %3 total damage!")
                .arg(spellName).arg(names.join(", ")).arg(total);
            return total;
        }
    }

    // --- Single-target schools (cold slow, mind stun/confuse, plain damage) ---
    CombatParticipant& target = m_state->participant(targetIndex);
    int dmg = damage;
    if (target.isDefending) dmg = qMax(1, dmg / 2);
    target.hp -= dmg;
    bool slain = false;
    if (target.hp <= 0) {
        target.hp = 0;
        target.isAlive = false;
        slain = true;
    }

    QString extra;

    // --- Cold: slow the target ---
    if (school == SpellMechanics::School::Cold) {
        const int turns = SpellMechanics::coldSlowTurns(*spell);
        const int pct = SpellMechanics::coldSlowPercent(*spell);
        // Restore any previous slow before applying a fresh one, so the speed
        // penalty never stacks with itself.
        target.speed += target.slowAmount;
        target.slowAmount = qMax(1, target.speed * pct / 100);
        target.speed = qMax(1, target.speed - target.slowAmount);
        target.slowDuration = turns;
        extra = QString(" %1 is slowed by %2% for %3 rounds!")
            .arg(target.name).arg(pct).arg(turns);
    }

    // --- Mind: stun, with a chance to confuse ---
    if (school == SpellMechanics::School::Mind && !slain) {
        const int stun = SpellMechanics::mindStunTurns(*spell);
        target.stunDuration = qMax(target.stunDuration, stun);
        extra = QString(" %1 is stunned for %2 rounds!").arg(target.name).arg(stun);

        const int confusePct = SpellMechanics::mindConfuseChance(*spell);
        if (QRandomGenerator::global()->bounded(100) < confusePct) {
            target.confusionDuration = qMax(target.confusionDuration, stun + 1);
            extra += QString(" %1 is confused!").arg(target.name);
        }
    }

    if (slain) {
        result = QString("%1 hits %2 for %3 damage — %2 is slain!%4")
            .arg(spellName).arg(target.name).arg(dmg).arg(extra);
    } else {
        result = QString("%1 hits %2 for %3 damage.%4")
            .arg(spellName).arg(target.name).arg(dmg).arg(extra);
    }
    return dmg;
}

int CombatActions::castSpellBySchool(int targetIndex, const QString& spellName, QString& result) {
    if (!m_state || !m_engine) { result = "No combat"; return -1; }
    int casterIdx = m_engine->currentParticipantIndex();
    if (casterIdx < 0) { result = "No caster"; return -1; }

    const SpellDef* spell = SpellBook::instance().byName(spellName);
    if (!spell) { result = QString("Unknown spell: %1").arg(spellName); return -1; }

    CombatParticipant& caster = m_state->participant(casterIdx);
    if (caster.mana < spell->mana) {
        result = QString("%1 does not have enough mana for %2 (need %3, have %4).")
            .arg(caster.name).arg(spellName).arg(spell->mana).arg(caster.mana);
        return -1;
    }

    // Roll damage from the spell's own range; non-damage spells carry no range.
    int damage = 0;
    if (!spell->damage.isEmpty()) {
        QStringList parts = spell->damage.split('-');
        int minDmg = parts.value(0).toInt();
        int maxDmg = parts.size() == 2 ? parts.value(1).toInt() : minDmg;
        if (maxDmg < minDmg) maxDmg = minDmg;
        damage = minDmg + QRandomGenerator::global()->bounded(maxDmg - minDmg + 1);
    }

    caster.mana -= spell->mana;
    SoundEffects::instance()->play(SoundEffects::Type::SpellCast);

    // A control-only spell (no damage) still applies its mechanic.
    if (damage <= 0) {
        return applySpellDamageBySchool(targetIndex, spellName, 0, result);
    }
    return applySpellDamageBySchool(targetIndex, spellName, damage, result);
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

        // Tick down consumable-effect durations
        if (p.teleportTurns > 0) {
            p.teleportTurns--;
            if (p.teleportTurns <= 0)
                messages.append(QString("%1's teleport effect fades.").arg(p.name));
        }
        if (p.levitationTurns > 0) {
            p.levitationTurns--;
            if (p.levitationTurns <= 0)
                messages.append(QString("%1's levitation effect fades.").arg(p.name));
        }
        if (p.invisibilityTurns > 0) {
            p.invisibilityTurns--;
            if (p.invisibilityTurns <= 0)
                messages.append(QString("%1 is no longer invisible.").arg(p.name));
        }
        if (p.seeInvisibleTurns > 0) {
            p.seeInvisibleTurns--;
            if (p.seeInvisibleTurns <= 0)
                messages.append(QString("%1's true sight fades.").arg(p.name));
        }
        if (p.bindTurns > 0) {
            p.bindTurns--;
            if (p.bindTurns <= 0)
                messages.append(QString("%1 is no longer bound.").arg(p.name));
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

        // Cold slow: the speed penalty is restored when it expires.
        if (p.slowDuration > 0) {
            p.slowDuration--;
            if (p.slowDuration <= 0) {
                p.speed += p.slowAmount;
                p.slowAmount = 0;
                messages.append(QString("%1 is no longer slowed.").arg(p.name));
            }
        }

        // Mind stun: the participant loses its turn while it lasts.
        if (p.stunDuration > 0) {
            p.stunDuration--;
            if (p.stunDuration <= 0) {
                messages.append(QString("%1 recovers from the stun.").arg(p.name));
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

    // Resolve the effect from the item data, exactly as the out-of-combat path
    // does — the two must not disagree about what an item does.
    const ItemDef* def = ItemDatabase::instance().byName(itemName);
    const QVariantMap spellRow = (def && def->spellID > 0)
        ? gameStateManager::instance()->getSpell(def->spellID)
        : QVariantMap();
    const ConsumableResolver::Resolved effect =
        def ? ConsumableResolver::resolve(*def, spellRow)
            : ConsumableResolver::Resolved();

    // Resolve the effect on the combat participant, then consume the item via
    // Character::useConsumable so inventory HP/charges stay authoritative.
    CombatParticipant& participant = m_state->participant(userIdx);
    bool handledHere = false;

    switch (effect.effect) {
    case ConsumableResolver::Effect::Heal: {
        int roll = effect.amountMin;
        if (effect.amountMax > effect.amountMin) {
            roll += QRandomGenerator::global()->bounded(effect.amountMax - effect.amountMin + 1);
        }
        const int actual = qMin(qMax(0, roll), participant.maxHp - participant.hp);
        participant.hp += actual;
        result = QString("%1 uses %2 and restores %3 HP.")
                     .arg(participant.name).arg(itemName).arg(actual);
        handledHere = true;
        break;
    }

    case ConsumableResolver::Effect::CurePoison: {
        const bool had = (participant.statusFlags & GameConstants::Poisoned) != 0;
        participant.statusFlags &= ~GameConstants::Poisoned;
        participant.poisonDuration = 0;
        result = had ? QString("%1 uses %2 — the poison is cured.")
                           .arg(participant.name).arg(itemName)
                     : QString("%1 uses %2 — nothing to cure.")
                           .arg(participant.name).arg(itemName);
        handledHere = true;
        break;
    }

    case ConsumableResolver::Effect::Damage:
    case ConsumableResolver::Effect::Charm:
    case ConsumableResolver::Effect::Banish:
    case ConsumableResolver::Effect::Dispel:
    case ConsumableResolver::Effect::Bind:
        // Targeted effects need a target; useItem() carries none, so say so
        // rather than acting on an arbitrary creature.
        result = QString("%1 readies %2 — choose a target to use it.")
                     .arg(participant.name).arg(effect.spellName);
        handledHere = true;
        break;

    default:
        break;
    }

    // Consume: apply the real inventory effect (decrements charges / removes).
    // This also applies stat boosts, so the combat layer must not apply them
    // itself or they would land twice.
    QString effectDesc;
    const bool consumed = user.useConsumable(itemIndex, effectDesc);

    if (!handledHere) {
        result = QString("%1 uses %2 — %3")
                     .arg(participant.name).arg(itemName).arg(effectDesc);
    }

    if (!consumed) {
        return false;
    }

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
