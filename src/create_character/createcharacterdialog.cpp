#include "createcharacterdialog.h"
#include "src/race_data/RaceData.h" 

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
#include <QVariant> // Ensure QVariant is included for property handling

// Global constant for the property key used in the SpinBoxes
// FIX: Changed from const static QString to const char* to match QObject::setProperty/property signature in Qt 6
const static char* PROPERTY_PREVIOUS_VALUE = "previousValue";
const static QStringList STAT_NAMES = {"Strength", "Intelligence", "Wisdom", "Constitution", "Charisma", "Dexterity"};

// ... (Helper function to populateComboBox - REMOVED as per previous step)

// Helper function to update alignment options
void CreateCharacterDialog::updateAlignmentOptions(const RaceStats& race) {
// ... (Implementation unchanged)
    this->alignmentBox->clear();
    // Use a temporary list to add options before setting the index
    QStringList alignments;
    if (race.good == AS_Allowed) {
        alignments.append("Good");
    }
    if (race.neutral == AS_Allowed) {
        alignments.append("Neutral");
    }
    if (race.evil == AS_Allowed) {
        alignments.append("Evil");
    }
    this->alignmentBox->addItems(alignments);
    
    // Set a default if options exist
    if (this->alignmentBox->count() > 0) {
        this->alignmentBox->setCurrentIndex(0);
    }
}

// Helper to update guild list styling based on race eligibility
void CreateCharacterDialog::updateGuildListStyle(const RaceStats& race) {
// ... (Implementation unchanged)
    if (!this->guildsListWidget) return;

    for (int i = 0; i < this->guildsListWidget->count(); ++i) {
        QListWidgetItem *item = this->guildsListWidget->item(i);
        QString guildName = item->text();

        AlignmentStatus status = race.guildEligibility.value(guildName, AS_Allowed);

        if (status == AS_Allowed) {
            item->setForeground(QBrush(Qt::black));
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled); 
        } else {
            item->setForeground(QBrush(Qt::darkGray));
            item->setFlags(Qt::NoItemFlags); 
        }
    }
    QListWidgetItem *selectedItem = guildsListWidget->currentItem();
    if (selectedItem && !(selectedItem->flags() & Qt::ItemIsEnabled)) {
        guildsListWidget->setCurrentItem(nullptr);
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
        
        // FIX: PROPERTY_PREVIOUS_VALUE is now const char*
        spinBox->setProperty(PROPERTY_PREVIOUS_VALUE, raceStat.start); // Line 92 fixed
        
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

// DECISION: Updated constructor to accept data and initialize members
CreateCharacterDialog::CreateCharacterDialog(const QVector<RaceStats>& raceData, 
                                               const QVector<QString>& guildData, 
                                               QWidget *parent) 
    : QDialog(parent), 
      raceData(raceData), // Initialize raceData member
      guildData(guildData) // Initialize guildData member
{
    // Setup Window
    setWindowTitle("Create Character");

    // Load Stylesheet (Good candidate for QRC if not already used)
    QFile styleSheetFile("style.qss"); 
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }
    // ... (Window sizing code removed for brevity) ...

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
    nameEdit = new QLineEdit("");
    infoLayout->addWidget(nameEdit, 1, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Race"), 2, 0);
    raceBox = new QComboBox();
    // DECISION: Integrated populateComboBox logic directly
    QStringList raceNames;
    for (const RaceStats& race : raceData) {
        raceNames.append(race.raceName);
    }
    raceBox->addItems(raceNames);
    infoLayout->addWidget(raceBox, 3, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Sex"), 4, 0);
    sexBox = new QComboBox();
    sexBox->addItems({"Male", "Female"});
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
    
    // Use the first race for initial spin box setup
    const RaceStats& initialRace = raceData.at(0);

    for (int i = 0; i < STAT_NAMES.size(); ++i) {
        QString statName = STAT_NAMES.at(i);
        statsLayout->addWidget(new QLabel(statName), i + 1, 0);

        QSpinBox *spinBox = new QSpinBox();
        RaceStat initialRaceStat;

        // Map stat name to initial race stat data
        if (statName == "Strength") initialRaceStat = initialRace.strength;
        else if (statName == "Intelligence") initialRaceStat = initialRace.intelligence;
        else if (statName == "Wisdom") initialRaceStat = initialRace.wisdom;
        else if (statName == "Constitution") initialRaceStat = initialRace.constitution;
        else if (statName == "Charisma") initialRaceStat = initialRace.charisma;
        else if (statName == "Dexterity") initialRaceStat = initialRace.dexterity;
        else initialRaceStat = {1, 1, 18}; // Fallback 

        spinBox->setRange(initialRaceStat.min, initialRaceStat.max);
        spinBox->setValue(initialRaceStat.start); 
        
        // FIX: PROPERTY_PREVIOUS_VALUE is now const char*
        spinBox->setProperty(PROPERTY_PREVIOUS_VALUE, initialRaceStat.start); // Line 212 fixed
        
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

        // Connection logic to handle stat points
        QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                         [currentValueLabel, this, spinBox](int newValue) {

            // FIX: PROPERTY_PREVIOUS_VALUE is now const char*
            int oldValue = spinBox->property(PROPERTY_PREVIOUS_VALUE).toInt(); // Line 231 fixed
            int change = newValue - oldValue;

            if (change == 0) return;

            // Check limits and adjust points
            if (change > 0) {
                if (this->statPoints >= change) { 
                    this->statPoints -= change;
                } else {
                    // Not enough points: prevent the increase and revert the value
                    spinBox->blockSignals(true);
                    spinBox->setValue(oldValue); 
                    spinBox->blockSignals(false);
                    return; 
                }
            } else { // change < 0 (Decreasing stat)
                // Recovering points
                this->statPoints -= change; 
            }

            // Update UI and store the new value as the old value for the next call
            // FIX: PROPERTY_PREVIOUS_VALUE is now const char*
            spinBox->setProperty(PROPERTY_PREVIOUS_VALUE, newValue); // Line 253 fixed
            
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
    // ... (rest of constructor code, slots, and destructor are unchanged as the errors were only in property access) ...
    QVBoxLayout *guildLayout = new QVBoxLayout();
    guildLayout->setAlignment(Qt::AlignTop);

    QLabel *guildsTitle = new QLabel("Guilds Allowed");
    guildLayout->addWidget(guildsTitle);

    guildsListWidget = new QListWidget(); 
    guildsListWidget->addItems(guildData);

    guildsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    guildLayout->addWidget(guildsListWidget);

    // Initialize all race-dependent UI elements (uses the data passed to the constructor)
    updateRaceStats(0);

    // REMOVED TEMPORARY CODE: Removed debug labels for character name and guild selection status
    
    // Add a simple selection status label for current item
    QLabel *selectedGuildLabel = new QLabel("Select a Guild");
    selectedGuildLabel->setStyleSheet("font-weight: bold; padding: 10px; background-color: #e0f7fa; border-radius: 5px;");
    guildLayout->addWidget(selectedGuildLabel);

    QObject::connect(guildsListWidget, &QListWidget::currentItemChanged,
                     selectedGuildLabel, [selectedGuildLabel](QListWidgetItem *current) {
                         if (current && (current->flags() & Qt::ItemIsEnabled)) {
                             selectedGuildLabel->setText("Selected Guild: " + current->text());
                         } else {
                             selectedGuildLabel->setText("Select a Guild");
                         }
                     });


    contentLayout->addLayout(guildLayout);
    mainLayout->addLayout(contentLayout);

    // ==========================================================
    // Bottom Buttons 
    // ==========================================================

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
    connect(exitButton, &QPushButton::clicked, this, &CreateCharacterDialog::onExitClicked); 
    connect(raceStatsButton, &QPushButton::clicked, this, &CreateCharacterDialog::onRaceStatsClicked);
    connect(guildStatsButton, &QPushButton::clicked, this, &CreateCharacterDialog::onGuildStatsClicked);
    connect(saveButton, &QPushButton::clicked, this, &CreateCharacterDialog::onSaveCharacterClicked);
    connect(tutorialButton, &QPushButton::clicked, this, &CreateCharacterDialog::onTutorialClicked);
}

CreateCharacterDialog::~CreateCharacterDialog() {}

// Slot implementations (Removed QMessageBox, replaced with debug message)

void CreateCharacterDialog::onRaceStatsClicked() {
    qDebug() << "Placeholder: Display Race Stats Dialog.";
}

void CreateCharacterDialog::onGuildStatsClicked() {
    qDebug() << "Placeholder: Display Guild Stats Dialog.";
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
    
    // Check if an item is selected AND if it's enabled 
    if (selectedGuildItem && (selectedGuildItem->flags() & Qt::ItemIsEnabled)) {
        selectedGuild = selectedGuildItem->text();
    }
    out << "Guild: " << selectedGuild << "\n";

    file.close();

    QMessageBox::information(this, "Save Character", QString("Character **%1** saved successfully to **%2**.").arg(characterName).arg(filename));
}


void CreateCharacterDialog::onTutorialClicked() {
    qDebug() << "Placeholder: Opening the tutorial dialog.";
}

void CreateCharacterDialog::onExitClicked() {
    this->close();
}
