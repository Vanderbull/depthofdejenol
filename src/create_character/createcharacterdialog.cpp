#include "createcharacterdialog.h"
#include "src/race_data/RaceData.h" // Include the new data header

#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>
#include <QFile>
#include <QTextStream> 
#include <QtWidgets/QListWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QDebug> 
#include <QBrush> 
#include <QColor> 

// NOTE: loadGuildData() and loadRaceData() implementations are in racedata.cpp.

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
        spinBox->setProperty("previousValue", raceStat.start); // IMPORTANT: Reset previous value
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
    // Load data using the moved functions
    raceData = loadRaceData();
    guildData = loadGuildData();

    // Setup Window
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
    // Use member variable nameEdit
    nameEdit = new QLineEdit("");
    infoLayout->addWidget(nameEdit, 1, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Race"), 2, 0);
    // Use member variable raceBox
    raceBox = new QComboBox();
    populateComboBox(raceBox, raceData);
    infoLayout->addWidget(raceBox, 3, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Sex"), 4, 0);
    // Use member variable sexBox
    sexBox = new QComboBox();
    sexBox->addItems({"Male", "Female"});
    infoLayout->addWidget(sexBox, 5, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Alignment"), 6, 0);
    // Use member variable alignmentBox
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
        spinBox->setProperty("previousValue", initialRaceStat.start); // Initialize previous value
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

        // MODIFIED: Connection logic to handle stat points
        QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                         [currentValueLabel, this, spinBox](int newValue) {

            // 1. Get the old value stored in the property
            int oldValue = spinBox->property("previousValue").toInt();
            int change = newValue - oldValue;

            if (change == 0) {
                return;
            }

            // 2. Check limits and adjust points
            if (change > 0) {
                // Attempting to increase stat
                if (this->statPoints >= change) { // Check if we have enough points
                    this->statPoints -= change;
                } else {
                    // Not enough points: prevent the increase and revert the value
                    spinBox->blockSignals(true);
                    spinBox->setValue(oldValue); // Revert the value
                    spinBox->blockSignals(false);
                    return; // Stop execution
                }
            } else { // change < 0 (Decreasing stat)
                // Recovering points
                this->statPoints -= change; // -= change because change is negative
            }

            // 3. Update UI and store the new value as the old value for the next call
            spinBox->setProperty("previousValue", newValue); 
            currentValueLabel->setText(QString::number(newValue));
            this->statPointsLeftLabel->setText(QString("%1 Stat Points Left").arg(this->statPoints));
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
    // 1. Get Character Name and sanitize
    QString characterName = nameEdit->text().trimmed();

    if (characterName.isEmpty()) {
        QMessageBox::warning(this, "Save Error", "Please enter a **Character Name** before saving.");
        return;
    }

    // 2. Define the file path (e.g., "JohnDoe.txt")
    QString filename = characterName + ".txt";
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Save Error", QString("Could not open file for writing: %1").arg(filename));
        return;
    }

    // 3. Write data to the file
    QTextStream out(&file);

    // --- Basic Info ---
    out << "CHARACTER_FILE_VERSION: 1.0\n";
    out << "Name: " << characterName << "\n";
    out << "Race: " << raceBox->currentText() << "\n";
    out << "Sex: " << sexBox->currentText() << "\n";
    out << "Alignment: " << alignmentBox->currentText() << "\n";

    // --- Stats ---
    out << "\n--- STATS ---\n";
    for (auto it = statSpinBoxes.constBegin(); it != statSpinBoxes.constEnd(); ++it) {
        out << it.key() << ": " << it.value()->value() << "\n";
    }

    // --- Guild ---
    out << "\n--- GUILD ---\n";
    QListWidgetItem* selectedGuildItem = guildsListWidget->currentItem();
    QString selectedGuild = "None";
    if (selectedGuildItem && (selectedGuildItem->flags() & Qt::ItemIsEnabled)) {
        selectedGuild = selectedGuildItem->text();
    }
    out << "Guild: " << selectedGuild << "\n";

    file.close();

    QMessageBox::information(this, "Save Character", QString("Character **%1** saved successfully to **%2**.").arg(characterName).arg(filename));
}


void CreateCharacterDialog::onTutorialClicked() {
    QMessageBox::information(this, "Tutorial", "Opening the tutorial...");
}

void CreateCharacterDialog::onExitClicked() {
    this->close();
}
