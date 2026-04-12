#include "SpellCastingDialog.h"
#include "../../GameStateManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QDebug>
#include <QCoreApplication>

SpellCastingDialog::SpellCastingDialog(QWidget *parent, bool inCombat)
    : QDialog(parent)
    , m_inCombat(inCombat)
{
    setWindowTitle("Cast Spell");
    setMinimumSize(600, 450);

    // --- Main Layout ---
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- Mana Display ---
    m_manaLabel = new QLabel(this);
    m_manaLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #4488ff;");
    updateManaDisplay();
    mainLayout->addWidget(m_manaLabel);

    // --- Category Selection ---
    QHBoxLayout* categoryLayout = new QHBoxLayout;
    QLabel* categoryLabel = new QLabel("School:", this);
    m_categoryCombo = new QComboBox(this);
    categoryLayout->addWidget(categoryLabel);
    categoryLayout->addWidget(m_categoryCombo, 1);
    mainLayout->addLayout(categoryLayout);

    // --- Main Content Area ---
    QHBoxLayout* contentLayout = new QHBoxLayout;

    // Spell List (left side)
    QVBoxLayout* listLayout = new QVBoxLayout;
    QLabel* spellListLabel = new QLabel("Available Spells:", this);
    m_spellList = new QListWidget(this);
    m_spellList->setMinimumWidth(200);
    listLayout->addWidget(spellListLabel);
    listLayout->addWidget(m_spellList);
    contentLayout->addLayout(listLayout);

    // Spell Details (right side)
    m_detailsGroup = new QGroupBox("Spell Details", this);
    QVBoxLayout* detailsLayout = new QVBoxLayout(m_detailsGroup);
    
    m_spellNameLabel = new QLabel("Select a spell", this);
    m_spellNameLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    
    m_manaCostLabel = new QLabel("Mana Cost: -", this);
    m_levelRequiredLabel = new QLabel("Required Level: -", this);
    
    m_spellDescription = new QTextEdit(this);
    m_spellDescription->setReadOnly(true);
    m_spellDescription->setMaximumHeight(120);
    
    detailsLayout->addWidget(m_spellNameLabel);
    detailsLayout->addWidget(m_manaCostLabel);
    detailsLayout->addWidget(m_levelRequiredLabel);
    detailsLayout->addWidget(m_spellDescription);
    detailsLayout->addStretch();
    
    contentLayout->addWidget(m_detailsGroup);
    mainLayout->addLayout(contentLayout);

    // --- Buttons ---
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_castButton = new QPushButton("Cast Spell", this);
    m_castButton->setEnabled(false);
    m_castButton->setStyleSheet("QPushButton { background-color: #4488ff; color: white; padding: 8px 16px; }");
    
    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setStyleSheet("padding: 8px 16px;");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_castButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    // --- Connections ---
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SpellCastingDialog::onCategoryChanged);
    connect(m_spellList, &QListWidget::itemClicked,
            this, &SpellCastingDialog::onSpellSelected);
    connect(m_spellList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem* item) {
                onSpellSelected(item);
                onCastClicked();
            });
    connect(m_castButton, &QPushButton::clicked, this, &SpellCastingDialog::onCastClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SpellCastingDialog::onCancelClicked);

    // --- Load Data ---
    loadSpellsFromJson();
    populateCategories();
}

SpellCastingDialog::~SpellCastingDialog()
{
}

void SpellCastingDialog::loadSpellsFromJson()
{
    // Try multiple paths to find spells.json
    QStringList paths = {
        "data/spells.json",
        ":/data/spells.json",
        QCoreApplication::applicationDirPath() + "/data/spells.json",
        "../data/spells.json"
    };
    
    QFile file;
    for (const QString& path : paths) {
        file.setFileName(path);
        if (file.exists()) {
            break;
        }
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open spells.json from any path";
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return;
    }
    
    m_spellsData = doc.object().value("spells").toObject();
    qDebug() << "Loaded" << m_spellsData.keys().count() << "spell categories";
}

void SpellCastingDialog::populateCategories()
{
    m_categoryCombo->clear();
    m_categories = m_spellsData.keys();
    
    // Sort categories for better UX
    m_categories.sort();
    
    for (const QString& category : m_categories) {
        m_categoryCombo->addItem(category);
    }
    
    if (!m_categories.isEmpty()) {
        populateSpellList(m_categories.first());
    }
}

void SpellCastingDialog::populateSpellList(const QString& category)
{
    m_spellList->clear();
    
    QJsonArray spells = m_spellsData.value(category).toArray();
    GameStateManager* gsm = GameStateManager::instance();
    
    // Get the current values from the variables you specified
    int currentMana = gsm->getCurrentMana(); // Pulls m_gameStateData["CurrentMana"]
    int playerLevel = gsm->getGameValue("CurrentCharacterLevel").toInt();

    for (const QJsonValue& spellVal : spells) {
        QJsonObject spell = spellVal.toObject();
        QString name = spell.value("name").toString();
        int manaCost = spell.value("mana").toInt();
        int reqLevel = spell.value("base_level").toInt();

        QListWidgetItem* item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, spell);

        // --- Logic for Colors and Disabling ---
        if (manaCost > currentMana) {
            // Not enough mana: Gray out and disable
            item->setForeground(Qt::gray);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled); // Make it unclickable
            item->setToolTip(QString("Requires %1 Mana (You have %2)").arg(manaCost).arg(currentMana));
        } 
        else if (reqLevel > playerLevel) {
            // Level too low: Dark Gray
            item->setForeground(Qt::gray);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setToolTip(QString("Requires Level %1").arg(reqLevel));
        } 
        else {
            // Affordable: Normal White text
            //item->setForeground(Qt::Black); 
            item->setForeground(this->palette().text());
            item->setFlags(item->flags() | Qt::ItemIsEnabled); // Ensure it's clickable
            item->setToolTip("");
        }

        m_spellList->addItem(item);
    }
}

void SpellCastingDialog::onCategoryChanged(int index)
{
    if (index >= 0 && index < m_categories.size()) {
        populateSpellList(m_categories.at(index));
    }
}

void SpellCastingDialog::onSpellSelected(QListWidgetItem* item)
{
    if (!item) return;
    
    m_selectedSpell = item->data(Qt::UserRole).toJsonObject();
    updateSpellDetails(m_selectedSpell);
    
    GameStateManager* gsm = GameStateManager::instance();
    
    // CHANGE: Use "mana" consistently with your populateSpellList function
    int manaCost = m_selectedSpell.value("mana").toInt(); 
    int reqLevel = m_selectedSpell.value("base_level").toInt();
    int playerLevel = gsm->getGameValue("CurrentCharacterLevel").toInt();
    int currentMana = gsm->getCurrentMana(); 
    
    bool canCast = (manaCost <= currentMana) && (reqLevel <= playerLevel);
    
    if (m_inCombat && !m_selectedSpell.value("is_combat_spell").toBool()) {
        canCast = false;
    }
    
    m_castButton->setEnabled(canCast);
}

void SpellCastingDialog::updateSpellDetails(const QJsonObject& spell)
{
    m_spellNameLabel->setText(spell.value("name").toString());
    m_manaCostLabel->setText(QString("Mana Cost: %1").arg(spell.value("mana").toInt()));
    m_levelRequiredLabel->setText(QString("Required Level: %1").arg(spell.value("base_level").toInt()));
    
    QString description = spell.value("description").toString();
    
    // Add damage info if applicable
    QJsonValue damageVal = spell.value("damage");
    if (!damageVal.isNull() && damageVal.toString() != "null") {
        description += QString("\n\nDamage: %1").arg(damageVal.toString());
    }
    
    // Add guild requirements
    QJsonArray guilds = spell.value("guilds").toArray();
    if (!guilds.isEmpty()) {
        QStringList guildNames;
        for (const QJsonValue& g : guilds) {
            guildNames << g.toString();
        }
        description += QString("\n\nTaught by: %1").arg(guildNames.join(", "));
    }
    
    m_spellDescription->setText(description);
}


void SpellCastingDialog::updateManaDisplay()
{
    GameStateManager* gsm = GameStateManager::instance();
    int current = gsm->getCurrentMana();
    int max = gsm->getMaxMana();
    
    m_manaLabel->setText(QString("Mana: %1 / %2").arg(current).arg(max));
    
    // Visually warn the player if they are low on mana
    if (current < 10) {
        m_manaLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #ff4444;");
    } else {
        m_manaLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #4488ff;");
    }
}

void SpellCastingDialog::onCastClicked()
{
    // 1. Safety check: ensure a spell is actually selected
    if (m_selectedSpell.isEmpty()) {
        return;
    }

    GameStateManager* gsm = GameStateManager::instance();
    
    // 2. Fetch mana cost using the correct JSON key "mana" 
    // (This matches your populateSpellList and updateSpellDetails functions)
    int manaCost = m_selectedSpell.value("mana").toInt();
    
    // 3. Perform the Mana Check and Reduction
    // spendMana() returns false if the player can't afford it
    if (!gsm->spendMana(manaCost)) {
        QMessageBox::warning(this, "Insufficient Mana", 
                             QString("You need %1 mana, but only have %2.")
                             .arg(manaCost).arg(gsm->getCurrentMana()));
        return;
    }

    // 4. Determine Spell Metadata
    QString spellName = m_selectedSpell.value("name").toString();
    QString category = m_categoryCombo->currentText();
    SpellResult result;

    // 5. Route to the appropriate effect logic
    if (category == "Heal") {
        result = castHealingSpell(m_selectedSpell);
    } 
    else if (category == "Fire" || category == "Cold" || category == "Electrical" || 
             category == "Mind" || category == "Magical" || category == "Damage") {
        result = castDamageSpell(m_selectedSpell);
    } 
    else if (category == "Element" || category == "Resistant" || category == "Protection") {
        result = castBuffSpell(m_selectedSpell);
    } 
    else {
        result = castUtilitySpell(m_selectedSpell);
    }

    // 6. Store result and set the cast flag
    m_lastResult = result;
    m_spellWasCast = true;

    // 7. Emit signals so DungeonDialog can update the game world
    emit spellCast(spellName, result);
    
    if (result.damageDealt > 0) {
        emit damageDealt(result.damageDealt);
    }
    if (result.healingDone > 0) {
        emit healingDone(result.healingDone);
    }
    if (!result.effectApplied.isEmpty()) {
        emit buffApplied(result.effectApplied, 10);
    }

    // 8. CRITICAL: Refresh the UI label to show the new lower mana amount
    updateManaDisplay();

    // 9. Feedback and close
    QMessageBox::information(this, "Spell Cast", result.message);

    updateManaDisplay(); // Updates the "Mana: X/Y" label
    populateSpellList(m_categoryCombo->currentText()); // Refreshes the list (greys out spells you can no longer afford)
    
    // Close the dialog with an "Accepted" result so the main game knows to process
    //accept(); 
}

void SpellCastingDialog::onCancelClicked()
{
    m_spellWasCast = false;
    reject();
}

SpellResult SpellCastingDialog::castDamageSpell(const QJsonObject& spell)
{
    SpellResult result;
    QString spellName = spell.value("name").toString();
    QString damageStr = spell.value("damage").toString();
    
    GameStateManager* gsm = GameStateManager::instance();
    int intelligence = gsm->getGameValue("CurrentCharacterIntelligence").toInt();
    
    int damage = calculateSpellDamage(damageStr, intelligence);
    
    if (damage > 0) {
        result.success = true;
        result.damageDealt = damage;
        result.message = QString("You cast %1 for %2 damage!").arg(spellName).arg(damage);
    } else if (damageStr == "Instant Kill" || damageStr == "Instant Banish") {
        // Special instant-kill spells have a chance to work
        int chance = 30 + intelligence; // 30% base + intelligence
        if (QRandomGenerator::global()->bounded(100) < chance) {
            result.success = true;
            result.damageDealt = 9999;
            result.message = QString("You cast %1! The creature is destroyed!").arg(spellName);
        } else {
            result.success = false;
            result.message = QString("%1 failed to take effect.").arg(spellName);
        }
    } else {
        result.success = false;
        result.message = QString("You cast %1, but it has no effect.").arg(spellName);
    }
    
    return result;
}

SpellResult SpellCastingDialog::castHealingSpell(const QJsonObject& spell)
{
    SpellResult result;
    QString spellName = spell.value("name").toString();
    
    GameStateManager* gsm = GameStateManager::instance();
    int wisdom = gsm->getGameValue("CurrentCharacterWisdom").toInt();
    int currentHP = gsm->getGameValue("CurrentCharacterHP").toInt();
    int maxHP = gsm->getGameValue("MaxCharacterHP").toInt();
    
    // Determine healing amount based on spell
    int healing = 0;
    
    if (spellName == "Cure Wounds") {
        healing = 10 + (wisdom / 2);
    } else if (spellName == "Heal") {
        healing = 30 + wisdom;
    } else if (spellName == "Cure Poison") {
        gsm->setCharacterPoisoned(false);
        result.success = true;
        result.effectApplied = "Poison Cured";
        result.message = "The poison has been purged from your body.";
        return result;
    } else if (spellName == "Cure Disease") {
        gsm->setGameValue("CharacterDiseased", false);
        result.success = true;
        result.effectApplied = "Disease Cured";
        result.message = "You feel healthy again.";
        return result;
    } else if (spellName == "Resurrect") {
        // This would be used on party members, not self
        result.success = true;
        result.effectApplied = "Resurrection";
        result.message = "The resurrection spell is prepared. Target a fallen ally.";
        return result;
    } else {
        healing = 15 + (wisdom / 3);
    }
    
    // Apply healing
    int newHP = qMin(currentHP + healing, maxHP);
    int actualHealing = newHP - currentHP;
    gsm->setGameValue("CurrentCharacterHP", newHP);
    
    result.success = true;
    result.healingDone = actualHealing;
    result.message = QString("You cast %1 and recover %2 HP!").arg(spellName).arg(actualHealing);
    
    return result;
}

SpellResult SpellCastingDialog::castBuffSpell(const QJsonObject& spell)
{
    SpellResult result;
    QString spellName = spell.value("name").toString();
    
    result.success = true;
    result.effectApplied = spellName;
    
    // Different buffs have different effects
    if (spellName.contains("Skin")) {
        result.message = QString("Your skin hardens! %1 is now active.").arg(spellName);
    } else if (spellName.contains("Resist")) {
        result.message = QString("You feel protected. %1 is now active.").arg(spellName);
    } else if (spellName.contains("Protection")) {
        result.message = QString("A protective aura surrounds you. %1 is now active.").arg(spellName);
    } else if (spellName == "Magic Shield") {
        result.message = "A shimmering shield of magical energy surrounds you!";
    } else {
        result.message = QString("%1 is now active.").arg(spellName);
    }
    
    return result;
}

SpellResult SpellCastingDialog::castUtilitySpell(const QJsonObject& spell)
{
    SpellResult result;
    QString spellName = spell.value("name").toString();
    
    GameStateManager* gsm = GameStateManager::instance();
    
    if (spellName == "Light") {
        result.success = true;
        result.effectApplied = "Light";
        result.message = "A magical light illuminates the area!";
    } else if (spellName == "See Invisible") {
        result.success = true;
        result.effectApplied = "See Invisible";
        result.message = "Your eyes can now perceive the invisible!";
    } else if (spellName == "Teleport") {
        result.success = true;
        result.effectApplied = "Teleport";
        result.message = "You feel the fabric of space twist around you...";
    } else if (spellName == "Levitate") {
        result.success = true;
        result.effectApplied = "Levitate";
        result.message = "You begin to float above the ground!";
    } else if (spellName == "Dispel Magic") {
        result.success = true;
        result.effectApplied = "Dispel";
        result.message = "Magical energies dissipate around you!";
    } else if (spellName == "Locate Creature" || spellName == "Locate Item") {
        result.success = true;
        result.message = QString("The %1 spell reveals hidden locations...").arg(spellName);
    } else if (spellName == "Charm Creature" || spellName == "Charm Monster") {
        int chance = 40 + gsm->getGameValue("CurrentCharacterCharisma").toInt();
        if (QRandomGenerator::global()->bounded(100) < chance) {
            result.success = true;
            result.effectApplied = "Charm";
            result.message = "The creature is now under your influence!";
        } else {
            result.success = false;
            result.message = "The charm spell failed to take hold.";
        }
    } else if (spellName == "Confusion" || spellName == "Fear") {
        int chance = 50 + gsm->getGameValue("CurrentCharacterIntelligence").toInt() / 2;
        if (QRandomGenerator::global()->bounded(100) < chance) {
            result.success = true;
            result.effectApplied = spellName;
            result.message = QString("The creature is affected by %1!").arg(spellName);
        } else {
            result.success = false;
            result.message = QString("The creature resists your %1 spell.").arg(spellName);
        }
    } else {
        result.success = true;
        result.message = QString("You cast %1.").arg(spellName);
    }
    
    return result;
}

int SpellCastingDialog::calculateSpellDamage(const QString& damageRange, int intelligence)
{
    // Parse damage range like "5-10" or "15-30"
    QStringList parts = damageRange.split('-');
    if (parts.size() != 2) {
        return 0;
    }
    
    bool ok1, ok2;
    int minDamage = parts[0].toInt(&ok1);
    int maxDamage = parts[1].toInt(&ok2);
    
    if (!ok1 || !ok2) {
        return 0;
    }
    
    // Base damage roll
    int baseDamage = QRandomGenerator::global()->bounded(minDamage, maxDamage + 1);
    
    // Intelligence bonus (10% per 10 points of intelligence)
    int bonus = (baseDamage * intelligence) / 100;
    
    return baseDamage + bonus;
}

int SpellCastingDialog::calculateHealing(const QString& healAmount, int wisdom)
{
    // Similar to damage calculation
    QStringList parts = healAmount.split('-');
    if (parts.size() == 2) {
        int min = parts[0].toInt();
        int max = parts[1].toInt();
        int baseHeal = QRandomGenerator::global()->bounded(min, max + 1);
        int bonus = (baseHeal * wisdom) / 100;
        return baseHeal + bonus;
    }
    return healAmount.toInt();
}
