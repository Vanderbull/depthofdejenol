#include "createcharacterdialog.h"

#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>
#include <QFile>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

// Structure to hold min/max values for a stat (e.g., Str, Int)
struct StatRange {
    int min;
    int max;
};

// Enum for alignment/characteristic columns (G, N, E)
enum AlignmentCharacteristic {
    Allowed, // Represents 'X'
    Neutral, // Represents 'N' - Used only as a placeholder if 'X' and 'N' mean different things, but based on the table 'N' is a column name, not a value. Let's stick to X, -, and a default.
    NotAllowed // Represents '-'
};

// Simplified Enum based on the table values ('X' and '-')
enum AlignmentStatus {
    AS_Allowed,   // Represents 'X'
    AS_NotAllowed // Represents '-'
};

// Structure to hold all stats for a single race
struct RaceStats {
    QString raceName;
    int maxAge;
    int experience;
    StatRange strength;
    StatRange intelligence;
    StatRange wisdom;
    StatRange constitution;
    StatRange charisma;
    StatRange dexterity;
    AlignmentStatus good; // G
    AlignmentStatus neutral; // N (Note: N is a column header, all races have X, so let's keep it for completeness)
    AlignmentStatus evil; // E
};

void populateComboBox(QComboBox *raceBox, const QVector<RaceStats>& data) {
    QStringList raceNames;

    // Iterate through the vector and extract only the raceName
    for (const RaceStats& race : data) {
        raceNames.append(race.raceName);
    }

    // Add the list of names to the QComboBox
    raceBox->addItems(raceNames);
}

CreateCharacterDialog::CreateCharacterDialog(QWidget *parent) : QDialog(parent) {

    QVector<RaceStats> raceData = {
        // RaceName, MaxAge, Exp, {Str}, {Int}, {Wis}, {Con}, {Cha}, {Dex}, G, N, E
        {"Human",   100, 3,  {4, 17}, {4, 18}, {4, 18}, {6, 17}, {5, 18}, {6, 18}, AS_Allowed, AS_Allowed, AS_Allowed},
        {"Elf",     400, 7,  {3, 15}, {7, 20}, {7, 20}, {3, 16}, {3, 18}, {3, 18}, AS_Allowed, AS_Allowed, AS_Allowed},
        {"Giant",   225, 7,  {12, 25},{4, 17}, {3, 17}, {9, 19}, {2, 16}, {3, 18}, AS_NotAllowed, AS_Allowed, AS_NotAllowed},
        {"Gnome",   300, 7,  {4, 17}, {7, 19}, {5, 19}, {3, 17}, {9, 22}, {6, 18}, AS_Allowed, AS_Allowed, AS_Allowed},
        {"Dwarf",   275, 5,  {3, 18}, {3, 18}, {7, 19}, {3, 19}, {3, 17}, {5, 18}, AS_NotAllowed, AS_Allowed, AS_NotAllowed},
        {"Ogre",    265, 6,  {9, 20}, {3, 16}, {3, 16}, {9, 21}, {3, 18}, {5, 17}, AS_Allowed, AS_Allowed, AS_Allowed},
        {"Morloch", 175, 4,  {6, 20}, {3, 17}, {5, 19}, {3, 15}, {2, 14}, {5, 20}, AS_Allowed, AS_NotAllowed, AS_Allowed},
        {"Osiri",   325, 8,  {5, 17}, {3, 18}, {3, 17}, {7, 19}, {3, 18}, {10, 22}, AS_NotAllowed, AS_Allowed, AS_NotAllowed},
        {"Troll",   285, 9,  {6, 20}, {3, 18}, {3, 18}, {6, 19}, {3, 17}, {6, 20}, AS_Allowed, AS_Allowed, AS_Allowed}
    };

    setWindowTitle("Create Character");
    
    // Load external style sheet
    QFile styleSheetFile("style.qss");
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }

    // Get screen geometry to make the window scale with screen resolution
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int windowWidth = screenGeometry.width() * 0.70;
    int windowHeight = screenGeometry.height() * 0.65;
    setMinimumSize(800, 600);
    resize(windowWidth, windowHeight);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // Main content layout (three columns)
    QHBoxLayout *contentLayout = new QHBoxLayout();

    // Left Column: Character Info
    QGridLayout *infoLayout = new QGridLayout();
    infoLayout->setSpacing(10);
    infoLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    infoLayout->addWidget(new QLabel("Character Name"), 0, 0);
    QLineEdit *nameEdit = new QLineEdit("Moby");
    infoLayout->addWidget(nameEdit, 1, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Race"), 2, 0);
    QComboBox *raceBox = new QComboBox();
    populateComboBox(raceBox, raceData);
    infoLayout->addWidget(raceBox, 3, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Sex"), 4, 0);
    QComboBox *sexBox = new QComboBox();
    sexBox->addItems({"Male", "Female"});
    infoLayout->addWidget(sexBox, 5, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Alignment"), 6, 0);
    QComboBox *alignmentBox = new QComboBox();
    alignmentBox->addItems({"Good", "Neutral", "Evil"});
    infoLayout->addWidget(alignmentBox, 7, 0, 1, 2);
    
    contentLayout->addLayout(infoLayout);

    // Middle Column: Stats
    QGridLayout *statsLayout = new QGridLayout();
    statsLayout->setSpacing(10);
    statsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    statsLayout->addWidget(new QLabel("Stats"), 0, 0, 1, 3);
    
    // Placeholder stat values
    const QStringList statNames = {"Strength", "Intelligence", "Wisdom", "Constitution", "Charisma", "Dexterity"};
    for (int i = 0; i < statNames.size(); ++i) {
        statsLayout->addWidget(new QLabel(statNames.at(i)), i + 1, 0);
        QSpinBox *spinBox = new QSpinBox();
        spinBox->setRange(1, 18);
        statsLayout->addWidget(spinBox, i + 1, 1);
        QLabel *rangeLabel = new QLabel("(4-18)");
        QString rangeText = QString("(%1-%2)").arg(raceData.at(i).strength.min).arg(raceData.at(i).strength.max);
        rangeLabel->setText(rangeText);
        statsLayout->addWidget(rangeLabel, i + 1, 2);


        // --- NEW CODE STARTS HERE ---

        // 1. Create a QLabel to display the current value/modifier/etc.
        // Initialize it with the SpinBox's starting value
        QLabel *currentValueLabel = new QLabel(QString::number(spinBox->value()));
        currentValueLabel->setAlignment(Qt::AlignCenter); // Center the text for better visibility
        currentValueLabel->setStyleSheet("color: blue; font-weight: bold;");

        // 2. Add the new label to the layout (e.g., in column 3)
        statsLayout->addWidget(currentValueLabel, i + 1, 3);

        // 3. Connect the QSpinBox's valueChanged signal to the new QLabel's text update
        // We use a lambda and capture 'currentValueLabel' to ensure we update the correct label in the loop.
        QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                         [currentValueLabel](int newValue) {

                             // This simple version just displays the new value.
                             currentValueLabel->setText(QString::number(newValue));

                             // You could also calculate a modifier here, for example:
                             /*
        int modifier = (newValue - 10) / 2;
        QString modifierText = (modifier >= 0) ? QString("+%1").arg(modifier) : QString::number(modifier);
        currentValueLabel->setText(QString("%1 (%2)").arg(newValue).arg(modifierText));
        */
                         });
    }


    QLabel *statPointsLeft = new QLabel("3 Stat Points Left");
    statsLayout->addWidget(statPointsLeft, statNames.size() + 2, 0, 1, 3);
    
    contentLayout->addLayout(statsLayout);

    // Right Column: Guilds
    QVBoxLayout *guildLayout = new QVBoxLayout();
    guildLayout->setAlignment(Qt::AlignTop);
    
    QLabel *guildsTitle = new QLabel("Guilds Allowed");
    guildLayout->addWidget(guildsTitle);
    
//    QLabel *guildsList = new QLabel("Nomad\nWarrior\nPaladin\nNinja\nVillain\nSeeker\nThief\nScavenger\nMage\nSorcerer\nWizard\nHealer");
//    guildLayout->addWidget(guildsList);

    // This code assumes you are working within a QWidget or QMainWindow
    // and have a QVBoxLayout named 'guildLayout' already set up.

    // Create a QListWidget object
    QListWidget *guildsListWidget = new QListWidget();

    // Add the items to the list
    guildsListWidget->addItem("Nomad");
    guildsListWidget->addItem("Warrior");
    guildsListWidget->addItem("Paladin");
    guildsListWidget->addItem("Ninja");
    guildsListWidget->addItem("Villain");
    guildsListWidget->addItem("Seeker");
    guildsListWidget->addItem("Thief");
    guildsListWidget->addItem("Scavenger");
    guildsListWidget->addItem("Mage");
    guildsListWidget->addItem("Sorcerer");
    guildsListWidget->addItem("Wizard");
    guildsListWidget->addItem("Healer");

    // Optionally, you can ensure only a single item can be selected at a time (default behavior)
    guildsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Add the list widget to your layout
    guildLayout->addWidget(guildsListWidget);

    // To get the selected item:

    QListWidgetItem *selectedItem = guildsListWidget->currentItem();
    if (selectedItem) {
        QString selectedGuild = selectedItem->text();
        // Use the selectedGuild string
    }

    contentLayout->addLayout(guildLayout);
    
    mainLayout->addLayout(contentLayout);

    // Bottom Buttons
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(20);
    bottomLayout->setAlignment(Qt::AlignCenter);

    QPushButton *raceStatsButton = new QPushButton("Race Stats");
    QPushButton *guildStatsButton = new QPushButton("Guild Stats");
    QPushButton *saveButton = new QPushButton("Save Character");
    QPushButton *tutorialButton = new QPushButton("Tutorial");
    QPushButton *exitButton = new QPushButton("Exit");

    bottomLayout->addWidget(raceStatsButton);
    bottomLayout->addWidget(guildStatsButton);
    bottomLayout->addWidget(saveButton);
    bottomLayout->addWidget(tutorialButton);
    bottomLayout->addWidget(exitButton);

    // DEBUG INFO
    // --- New Code Starts Here ---

    // 2. Create the QLabel that will display the updated text
    QLabel *updateLabel = new QLabel("Current Name: Moby"); // Initialize with the starting text

    // 3. Add the new label to your layout (adjust row/column as needed)
    // Example: Adding it below the QLineEdit, spanning two columns
    guildLayout->addWidget(updateLabel);

    // 4. Connect the QLineEdit's signal to the QLabel's slot
    // When the text in 'nameEdit' changes, the 'setText' method of 'updateLabel' is called.
    QObject::connect(nameEdit, &QLineEdit::textChanged,
                     updateLabel, [updateLabel](const QString &newText) {
                         // We use a lambda here to prepend a fixed string "Current Name: "
                         updateLabel->setText("Current Name: " + newText);
                     });

    //////  SECOND
    // 2. Create the QLabel that will display the updated text
    QLabel *strengthLabel = new QLabel("10"); // Initialize with the starting text

    // 3. Add the new label to your layout (adjust row/column as needed)
    // Example: Adding it below the QLineEdit, spanning two columns
    guildLayout->addWidget(strengthLabel);

    /////////////////////////////////// 1. Create the QLabel to display the selected item
    QLabel *selectionStatusLabel = new QLabel("No Guild Selected");
    selectionStatusLabel->setStyleSheet("font-weight: bold; padding: 10px; background-color: #e0f7fa; border-radius: 5px;");

    // 2. Add the status label to your layout
    guildLayout->addWidget(selectionStatusLabel);

    // 3. Connect the QListWidget's signal to an action
    QObject::connect(guildsListWidget, &QListWidget::currentItemChanged,
                     selectionStatusLabel, [selectionStatusLabel](QListWidgetItem *current, QListWidgetItem *previous) {

                         // Check if an item is currently selected
                         if (current) {
                             QString selectedGuild = current->text();

                             // Update the label with the selected choice
                             selectionStatusLabel->setText("Selected Guild: " + selectedGuild);

                             // --- Your original logic is now inside this block:
                             // // Use the selectedGuild string
                             // ...
                             // ----------------------------------------------
                         } else {
                             // Handle case where selection is cleared
                             selectionStatusLabel->setText("No Guild Selected");
                         }
                     });/////

    mainLayout->addLayout(bottomLayout);

    // Connections
    connect(exitButton, &QPushButton::clicked, this, &QDialog::close);
    connect(raceStatsButton, &QPushButton::clicked, this, &CreateCharacterDialog::onRaceStatsClicked);
    connect(guildStatsButton, &QPushButton::clicked, this, &CreateCharacterDialog::onGuildStatsClicked);
    connect(saveButton, &QPushButton::clicked, this, &CreateCharacterDialog::onSaveCharacterClicked);
    connect(tutorialButton, &QPushButton::clicked, this, &CreateCharacterDialog::onTutorialClicked);
}

CreateCharacterDialog::~CreateCharacterDialog() {}

void CreateCharacterDialog::onRaceStatsClicked() {
    QMessageBox::information(this, "Race Stats", "Displaying Race Stats...");
}

void CreateCharacterDialog::onGuildStatsClicked() {
    QMessageBox::information(this, "Guild Stats", "Displaying Guild Stats...");
}

void CreateCharacterDialog::onSaveCharacterClicked() {
    QMessageBox::information(this, "Save Character", "Saving the character...");
}

void CreateCharacterDialog::onTutorialClicked() {
    QMessageBox::information(this, "Tutorial", "Opening the tutorial...");
}

void CreateCharacterDialog::onExitClicked() {
    this->close();
}
