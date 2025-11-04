#include "createcharacterdialog.h"

#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>
#include <QFile>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QDebug> 
#include <QBrush> 
#include <QColor> 


// Helper to load and return the list of all playable guilds
QVector<QString> CreateCharacterDialog::loadGuildData() {
    return {
        "Nomad",
        "Warrior",
        "Paladin",
        "Ninja",
        "Villain",
        "Seeker",
        "Thief",
        "Scavenger",
        "Mage",
        "Sorcerer",
        "Wizard",
        "Healer"
    };
}

// Helper to load and return the race data vector
QVector<RaceStats> CreateCharacterDialog::loadRaceData() {
    QVector<QString> allGuilds = loadGuildData();
    
    QVector<RaceStats> data = {
        // RaceName, MaxAge, Exp, {Start, Min, Max}, {Int}, {Wis}, {Con}, {Cha}, {Dex}, G, N, E, {}
        {"Human",   100, 3,  {4, 4, 17}, {4, 4, 18}, {4, 4, 18}, {6, 6, 17}, {5, 5, 18}, {6, 6, 18}, AS_Allowed, AS_Allowed, AS_Allowed, {}},
        {"Elf",     400, 7,  {3, 3, 15}, {7, 7, 20}, {7, 7, 20}, {3, 3, 16}, {3, 3, 18}, {3, 3, 18}, AS_Allowed, AS_Allowed, AS_Allowed, {}},
        {"Giant",   225, 7,  {12, 12, 25},{4, 4, 17}, {3, 3, 17}, {9, 9, 19}, {2, 2, 16}, {3, 3, 18}, AS_NotAllowed, AS_Allowed, AS_NotAllowed, {}},
        {"Gnome",   300, 7,  {4, 4, 17}, {7, 7, 19}, {5, 5, 19}, {3, 3, 17}, {9, 9, 22}, {6, 6, 18}, AS_Allowed, AS_Allowed, AS_Allowed, {}},
        {"Dwarf",   275, 5,  {3, 3, 18}, {3, 3, 18}, {7, 7, 19}, {3, 3, 19}, {3, 3, 17}, {5, 5, 18}, AS_NotAllowed, AS_Allowed, AS_NotAllowed, {}},
        {"Ogre",    265, 6,  {9, 9, 20}, {3, 3, 16}, {3, 3, 16}, {9, 9, 21}, {3, 3, 18}, {5, 5, 17}, AS_Allowed, AS_Allowed, AS_Allowed, {}},
        {"Morloch", 175, 4,  {6, 6, 20}, {3, 3, 17}, {5, 5, 19}, {3, 3, 15}, {2, 2, 14}, {5, 5, 20}, AS_Allowed, AS_NotAllowed, AS_Allowed, {}},
        {"Osiri",   325, 8,  {5, 5, 17}, {3, 3, 18}, {3, 3, 17}, {7, 7, 19}, {3, 3, 18}, {10, 10, 22}, AS_NotAllowed, AS_Allowed, AS_NotAllowed, {}},
        {"Troll",   285, 9,  {6, 6, 20}, {3, 3, 18}, {3, 3, 18}, {6, 6, 19}, {3, 3, 17}, {6, 6, 20}, AS_Allowed, AS_Allowed, AS_Allowed, {}}
    };
    
    // Initialize Guild Eligibility (Placeholder Logic)
    for (RaceStats& race : data) {
        // Default: All races are allowed in all guilds
        for (const QString& guild : allGuilds) {
            race.guildEligibility[guild] = AS_Allowed;
        }

        // Custom Race Restrictions (Example based on common RPG tropes)
        if (race.raceName == "Giant") {
            race.guildEligibility["Mage"] = AS_NotAllowed;
            race.guildEligibility["Wizard"] = AS_NotAllowed;
            race.guildEligibility["Thief"] = AS_NotAllowed;
        } else if (race.raceName == "Dwarf") {
            race.guildEligibility["Mage"] = AS_NotAllowed;
            race.guildEligibility["Sorcerer"] = AS_NotAllowed;
            race.guildEligibility["Wizard"] = AS_NotAllowed;
        } else if (race.raceName == "Elf") {
            race.guildEligibility["Villain"] = AS_NotAllowed;
        }
        // ... add real Mordor rules here
    }
    return data;
}

// Helper to populate the race ComboBox
void populateComboBox(QComboBox *raceBox, const QVector<RaceStats>& data) {
    QStringList raceNames;
    for (const RaceStats& race : data) {
        raceNames.append(race.raceName);
    }
    raceBox->addItems(raceNames);
}

// Helper function to update alignment options
void CreateCharacterDialog::updateAlignmentOptions(const RaceStats& race) {
    this->alignmentBox->clear();
    if (race.good == AS_Allowed) {
        this->alignmentBox->addItem("Good");
    }
    if (race.neutral == AS_Allowed) {
        this->alignmentBox->addItem("Neutral");
    }
    if (race.evil == AS_Allowed) {
        this->alignmentBox->addItem("Evil");
    }
    // Set a default if options exist
    if (this->alignmentBox->count() > 0) {
        this->alignmentBox->setCurrentIndex(0);
    }
}

// Helper to update guild list styling based on race eligibility
void CreateCharacterDialog::updateGuildListStyle(const RaceStats& race) {
    if (!this->guildsListWidget) return;

    for (int i = 0; i < this->guildsListWidget->count(); ++i) {
        QListWidgetItem *item = this->guildsListWidget->item(i);
        QString guildName = item->text();
        
        AlignmentStatus status = race.guildEligibility.value(guildName, AS_Allowed);

        if (status == AS_Allowed) {
            item->setForeground(QBrush(Qt::black));
            item->setFlags(item->flags() | Qt::ItemIsEnabled); // Enable selection
        } else {
            item->setForeground(QBrush(Qt::darkGray));
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled); // Disable selection
        }
    }
}

// Slot implementation to update all race-specific UI elements
void CreateCharacterDialog::updateRaceStats(int index) {
    if (index < 0 || index >= raceData.size()) {
        return; 
    }

    const RaceStats& selectedRace = raceData.at(index);
    
    QMap<QString, RaceStat> statMap;
    statMap["Strength"]     = selectedRace.strength;
    statMap["Intelligence"] = selectedRace.intelligence;
    statMap["Wisdom"]       = selectedRace.wisdom;
    statMap["Constitution"] = selectedRace.constitution;
    statMap["Charisma"]     = selectedRace.charisma;
    statMap["Dexterity"]    = selectedRace.dexterity;

    for (auto it = statSpinBoxes.constBegin(); it != statSpinBoxes.constEnd(); ++it) {
        QString statName = it.key();
        QSpinBox* spinBox = it.value();
        RaceStat raceStat = statMap.value(statName);

        // Update SpinBox constraints and value
        spinBox->blockSignals(true); 
        spinBox->setRange(raceStat.min, raceStat.max);
        spinBox->setValue(raceStat.start); 
        spinBox->blockSignals(false);

        // Update Range Label
        QLabel* rangeLabel = statRangeLabels.value(statName);
        if (rangeLabel) {
            rangeLabel->setText(QString("(%1-%2)").arg(raceStat.min).arg(raceStat.max));
        }
        
        // Update Value Label
        QLabel* valueLabel = statValueLabels.value(statName);
        if (valueLabel) {
             valueLabel->setText(QString::number(raceStat.start));
        }
    }
    
    // Update alignment and guild eligibility
    updateAlignmentOptions(selectedRace);
    updateGuildListStyle(selectedRace);

    // Reset stat points 
    this->statPoints = 5;
    this->statPointsLeftLabel->setText(QString("%1 Stat Points Left").arg(this->statPoints));
}

CreateCharacterDialog::CreateCharacterDialog(QWidget *parent) : QDialog(parent) {
    // 1. Load data
    raceData = loadRaceData();
    guildData = loadGuildData();

    // 2. Setup Window
    setWindowTitle("Create Character");
    
    QFile styleSheetFile("style.qss");
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int windowWidth = screenGeometry.width() * 0.70;
    int windowHeight = screenGeometry.height() * 0.65;
    setMinimumSize(800, 600);
    resize(windowWidth, windowHeight);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    QHBoxLayout *contentLayout = new QHBoxLayout();

    // ==========================================================
    // Left Column: Character Info
    // ==========================================================
    QGridLayout *infoLayout = new QGridLayout();
    infoLayout->setSpacing(10);
    infoLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    infoLayout->addWidget(new QLabel("Character Name"), 0, 0);
    QLineEdit *nameEdit = new QLineEdit("");
    infoLayout->addWidget(nameEdit, 1, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Race"), 2, 0);
    raceBox = new QComboBox();
    populateComboBox(raceBox, raceData);
    infoLayout->addWidget(raceBox, 3, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Sex"), 4, 0);
    QComboBox *sexBox = new QComboBox();
    sexBox->addItems({"Male", "Female", "Neuter"});
    infoLayout->addWidget(sexBox, 5, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Alignment"), 6, 0);
    alignmentBox = new QComboBox();
    infoLayout->addWidget(alignmentBox, 7, 0, 1, 2);
    
    contentLayout->addLayout(infoLayout);

    // ==========================================================
    // Middle Column: Stats
    // ==========================================================
    QGridLayout *statsLayout = new QGridLayout();
    statsLayout->setSpacing(10);
    statsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    statsLayout->addWidget(new QLabel("Stats"), 0, 0, 1, 3);
    
    this->statPointsLeftLabel = new QLabel(QString("%1 Stat Points Left").arg(this->statPoints));
    statsLayout->addWidget(this->statPointsLeftLabel, 7, 0, 1, 3);
    
    const QStringList statNames = {"Strength", "Intelligence", "Wisdom", "Constitution", "Charisma", "Dexterity"};
    for (int i = 0; i < statNames.size(); ++i) {
        QString statName = statNames.at(i);
        statsLayout->addWidget(new QLabel(statName), i + 1, 0);
        
        QSpinBox *spinBox = new QSpinBox();
        const RaceStats& initialRace = raceData.at(0);
        RaceStat initialRaceStat;

        // Map stat name to the correct RaceStat object
        if (statName == "Strength") initialRaceStat = initialRace.strength;
        else if (statName == "Intelligence") initialRaceStat = initialRace.intelligence;
        else if (statName == "Wisdom") initialRaceStat = initialRace.wisdom;
        else if (statName == "Constitution") initialRaceStat = initialRace.constitution;
        else if (statName == "Charisma") initialRaceStat = initialRace.charisma;
        else if (statName == "Dexterity") initialRaceStat = initialRace.dexterity;
        else initialRaceStat = {1, 1, 18}; 

        spinBox->setRange(initialRaceStat.min, initialRaceStat.max);
        spinBox->setValue(initialRaceStat.start); 
        statsLayout->addWidget(spinBox, i + 1, 1);
        
        QLabel *rangeLabel = new QLabel(QString("(%1-%2)").arg(initialRaceStat.min).arg(initialRaceStat.max));
        statsLayout->addWidget(rangeLabel, i + 1, 2);
        
        QLabel *currentValueLabel = new QLabel(QString::number(spinBox->value()));
        currentValueLabel->setAlignment(Qt::AlignCenter); 
        currentValueLabel->setStyleSheet("color: blue; font-weight: bold;");
        statsLayout->addWidget(currentValueLabel, i + 1, 3);
        
        statSpinBoxes.insert(statName, spinBox);
        statRangeLabels.insert(statName, rangeLabel);
        statValueLabels.insert(statName, currentValueLabel);

        QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                         [currentValueLabel, this, spinBox](int newValue) {
                             currentValueLabel->setText(QString::number(newValue));
                         });
    }

    QObject::connect(raceBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     this, &CreateCharacterDialog::updateRaceStats);

    contentLayout->addLayout(statsLayout);

    // ==========================================================
    // Right Column: Guilds
    // ==========================================================
    QVBoxLayout *guildLayout = new QVBoxLayout();
    guildLayout->setAlignment(Qt::AlignTop);
    
    QLabel *guildsTitle = new QLabel("Guilds Allowed");
    guildLayout->addWidget(guildsTitle);
    
    guildsListWidget = new QListWidget(); // Use member variable

    for (const QString& guildName : guildData) {
        guildsListWidget->addItem(guildName);
    }

    guildsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    guildLayout->addWidget(guildsListWidget);

    // Initialize all race-dependent UI elements (must be called after all widgets are created)
    updateRaceStats(0);
    
    QListWidgetItem *selectedItem = guildsListWidget->currentItem();
    if (selectedItem) {
        QString selectedGuild = selectedItem->text();
    }

    contentLayout->addLayout(guildLayout);
    
    mainLayout->addLayout(contentLayout);

    // ==========================================================
    // Bottom Buttons and Debug Info
    // ==========================================================
    
    // DEBUG INFO 
    QLabel *updateLabel = new QLabel("Current Name: "); 

    guildLayout->addWidget(updateLabel);

    QObject::connect(nameEdit, &QLineEdit::textChanged,
                     updateLabel, [updateLabel](const QString &newText) {
                         updateLabel->setText("Current Name: " + newText);
                     });

    QLabel *selectionStatusLabel = new QLabel("No Guild Selected");
    selectionStatusLabel->setStyleSheet("font-weight: bold; padding: 10px; background-color: #e0f7fa; border-radius: 5px;");

    guildLayout->addWidget(selectionStatusLabel);

    QObject::connect(guildsListWidget, &QListWidget::currentItemChanged,
                     selectionStatusLabel, [selectionStatusLabel](QListWidgetItem *current) {

                         if (current) {
                             if (current->flags() & Qt::ItemIsEnabled) {
                                 QString selectedGuild = current->text();
                                 selectionStatusLabel->setText("Selected Guild: " + selectedGuild);
                             } else {
                                 // If the user tries to select a disabled guild
                                 selectionStatusLabel->setText("Guild Not Allowed for Race");
                             }
                         } else {
                             selectionStatusLabel->setText("No Guild Selected");
                         }
                     });
                     
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
