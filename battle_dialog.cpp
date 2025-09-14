#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QRandomGenerator>
#include <QTimer>
#include <QDebug>

// Simple class to represent a character in the battle
class Character {
public:
    QString name;
    int health;
    int maxHealth;
    int mana;
    int maxMana;
    int minDamage;
    int maxDamage;
    bool isAlive() const { return health > 0; }
};

// Main battle dialog class
class BattleDialog : public QDialog {
    Q_OBJECT

public:
    explicit BattleDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Mordor: The Depths of Dejenol");
        setupUI();
        setupCharacters();
        startBattle();
    }

private slots:
    // Slot to handle the player's attack action
    void onAttackClicked() {
        // Disable buttons to prevent spamming while the turn is processing
        setButtonsEnabled(false);

        // Player's turn to attack
        int damage = QRandomGenerator::global()->bounded(player.minDamage, player.maxDamage + 1);
        enemy.health -= damage;
        logAppend(QString("%1 hits %2 for %3 damage.").arg(player.name, enemy.name).arg(damage));

        // Update UI status
        updateStatusLabels();

        // Check if the battle is over
        if (!enemy.isAlive()) {
            endBattle(true);
            return;
        }

        // Use a timer to delay the enemy's turn, making it feel more like a turn-based game
        QTimer::singleShot(1000, this, &BattleDialog::processEnemyTurn);
    }

    // Slot to handle the player's magic action
    void onMagicClicked() {
        setButtonsEnabled(false);

        const int magicCost = 10;
        if (player.mana >= magicCost) {
            player.mana -= magicCost;
            int damage = QRandomGenerator::global()->bounded(30, 50);
            enemy.health -= damage;
            logAppend(QString("%1 casts a fiery spell on %2 for %3 magical damage.").arg(player.name, enemy.name).arg(damage));
        } else {
            logAppend(QString("%1 tries to cast a spell, but has insufficient mana!").arg(player.name));
        }

        updateStatusLabels();

        if (!enemy.isAlive()) {
            endBattle(true);
            return;
        }

        QTimer::singleShot(1000, this, &BattleDialog::processEnemyTurn);
    }

    // Slot to handle the player's run action
    void onRunClicked() {
        logAppend(QString("%1 attempts to flee... and successfully escapes the battle!").arg(player.name));
        endBattle(false);
    }

    // Slot to process the enemy's turn
    void processEnemyTurn() {
        // Enemy's turn to attack
        int damage = QRandomGenerator::global()->bounded(enemy.minDamage, enemy.maxDamage + 1);
        player.health -= damage;
        logAppend(QString("%1 lashes out at %2 for %3 damage.").arg(enemy.name, player.name).arg(damage));

        // Update UI status
        updateStatusLabels();

        // Check if the battle is over
        if (!player.isAlive()) {
            endBattle(false);
            return;
        }

        // Re-enable buttons for the player's next turn
        setButtonsEnabled(true);
    }

private:
    // UI components
    QTextEdit *battleLog;
    QLabel *playerStatusLabel;
    QLabel *enemyStatusLabel;
    QPushButton *attackButton;
    QPushButton *magicButton;
    QPushButton *runButton;

    // Game state
    Character player;
    Character enemy;

    // Sets up the main UI layout
    void setupUI() {
        // Set a fixed size for the dialog
        setFixedSize(600, 400);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Status bar for characters
        QHBoxLayout *statusBarLayout = new QHBoxLayout();
        playerStatusLabel = new QLabel("Player Status", this);
        playerStatusLabel->setStyleSheet("font-weight: bold; color: #4CAF50;");
        enemyStatusLabel = new QLabel("Enemy Status", this);
        enemyStatusLabel->setStyleSheet("font-weight: bold; color: #F44336;");
        statusBarLayout->addWidget(playerStatusLabel);
        statusBarLayout->addWidget(enemyStatusLabel);
        mainLayout->addLayout(statusBarLayout);

        // Battle log area
        battleLog = new QTextEdit(this);
        battleLog->setReadOnly(true);
        battleLog->setStyleSheet("background-color: #212121; color: #E0E0E0; font-family: 'Consolas', 'Courier New', monospace; border: 1px solid #757575;");
        mainLayout->addWidget(battleLog);

        // Action buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        attackButton = new QPushButton("Attack", this);
        magicButton = new QPushButton("Magic", this);
        runButton = new QPushButton("Run", this);

        // Connect button signals to slots
        connect(attackButton, &QPushButton::clicked, this, &BattleDialog::onAttackClicked);
        connect(magicButton, &QPushButton::clicked, this, &BattleDialog::onMagicClicked);
        connect(runButton, &QPushButton::clicked, this, &BattleDialog::onRunClicked);

        buttonLayout->addWidget(attackButton);
        buttonLayout->addWidget(magicButton);
        buttonLayout->addWidget(runButton);
        mainLayout->addLayout(buttonLayout);

        setLayout(mainLayout);
    }

    // Initializes the player and enemy characters
    void setupCharacters() {
        player.name = "Valiant Hero";
        player.health = 100;
        player.maxHealth = 100;
        player.mana = 50;
        player.maxMana = 50;
        player.minDamage = 8;
        player.maxDamage = 15;

        enemy.name = "Foul Orc";
        enemy.health = 80;
        enemy.maxHealth = 80;
        enemy.mana = 0;
        enemy.maxMana = 0;
        enemy.minDamage = 5;
        enemy.maxDamage = 12;
    }

    // Starts the battle sequence
    void startBattle() {
        battleLog->clear();
        logAppend(QString("A wild %1 appears!").arg(enemy.name));
        logAppend(QString("The battle for Mordor begins!"));
        updateStatusLabels();
        setButtonsEnabled(true);
    }

    // Appends a message to the battle log
    void logAppend(const QString& message) {
        battleLog->append(message);
        // Scroll to the bottom of the log
        battleLog->verticalScrollBar()->setValue(battleLog->verticalScrollBar()->maximum());
    }

    // Updates the health and mana labels
    void updateStatusLabels() {
        playerStatusLabel->setText(QString("%1: H:%2/%3 M:%4/%5").arg(player.name).arg(player.health).arg(player.maxHealth).arg(player.mana).arg(player.maxMana));
        enemyStatusLabel->setText(QString("%1: H:%2/%3").arg(enemy.name).arg(enemy.health).arg(enemy.maxHealth));
    }

    // Ends the battle and shows a final message
    void endBattle(bool playerWon) {
        setButtonsEnabled(false);
        if (playerWon) {
            logAppend("\n*********************************");
            logAppend("Victory! The enemy has been defeated!");
            logAppend("*********************************");
        } else {
            logAppend("\n*********************************");
            logAppend("Defeat! You have fallen in battle.");
            logAppend("*********************************");
        }
    }

    // Helper function to enable/disable buttons
    void setButtonsEnabled(bool enabled) {
        attackButton->setEnabled(enabled);
        magicButton->setEnabled(enabled);
        runButton->setEnabled(enabled);
    }
};

// Main function to run the application
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    BattleDialog dialog;
    dialog.show();
    return app.exec();
}

#include "mordor_battle_dialog.moc"
