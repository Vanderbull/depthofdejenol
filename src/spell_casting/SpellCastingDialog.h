#ifndef SPELLCASTINGDIALOG_H
#define SPELLCASTINGDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QJsonObject>
#include <QJsonArray>

// Forward declaration
class GameStateManager;

// Spell effect result structure
struct SpellResult {
    bool success = false;
    QString message;
    int damageDealt = 0;
    int healingDone = 0;
    QString effectApplied;
};

class SpellCastingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpellCastingDialog(QWidget *parent = nullptr, bool inCombat = false);
    ~SpellCastingDialog();

    // Get the result of the last cast spell (for DungeonDialog to use)
    SpellResult getLastSpellResult() const { return m_lastResult; }
    bool wasSpellCast() const { return m_spellWasCast; }

signals:
    void spellCast(const QString& spellName, const SpellResult& result);
    void damageDealt(int damage);
    void healingDone(int healing);
    void buffApplied(const QString& buffName, int duration);

private slots:
    void onCategoryChanged(int index);
    void onSpellSelected(QListWidgetItem* item);
    void onCastClicked();
    void onCancelClicked();

private:
    void loadSpellsFromJson();
    void populateCategories();
    void populateSpellList(const QString& category);
    void updateSpellDetails(const QJsonObject& spell);
    void updateManaDisplay();
    
    // Spell effect implementations
    SpellResult castDamageSpell(const QJsonObject& spell);
    SpellResult castHealingSpell(const QJsonObject& spell);
    SpellResult castBuffSpell(const QJsonObject& spell);
    SpellResult castUtilitySpell(const QJsonObject& spell);
    
    int calculateSpellDamage(const QString& damageRange, int intelligence);
    int calculateHealing(const QString& healAmount, int wisdom);

    // UI Elements
    QComboBox* m_categoryCombo;
    QListWidget* m_spellList;
    QTextEdit* m_spellDescription;
    QLabel* m_manaLabel;
    QLabel* m_spellNameLabel;
    QLabel* m_manaCostLabel;
    QLabel* m_levelRequiredLabel;
    QPushButton* m_castButton;
    QPushButton* m_cancelButton;
    QGroupBox* m_detailsGroup;

    // Data
    QJsonObject m_spellsData;
    QJsonObject m_selectedSpell;
    SpellResult m_lastResult;
    bool m_spellWasCast = false;
    bool m_inCombat = false;
    
    // Spell categories from spells.json
    QStringList m_categories;
};

#endif // SPELLCASTINGDIALOG_H
