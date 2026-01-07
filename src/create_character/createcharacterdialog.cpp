#include "createcharacterdialog.h"
#include "src/race_data/RaceData.h"
#include "../../GameStateManager.h"
#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>
#include <QBrush>
#include <QColor>
#include <QVariant>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>

// Global constant for the property key used in the SpinBoxes
const static char* PROPERTY_PREVIOUS_VALUE = "previousValue";

// Helper function to update alignment
void CreateCharacterDialog::updateAlignmentOptions(const RaceStats& race) {
    // RAII approach to blocking signals: automatically unblocks when 'blocker' goes out of scope
    QSignalBlocker blocker(this->alignmentBox);
    
    this->alignmentBox->clear();

    const QStringList allAlignments = GameStateManager::alignmentNames();
    if (allAlignments.size() < 3) return; // Guard against uninitialized data

    // 1. Filter alignments using a mapping strategy
    QStringList allowed;
    if (race.good == AS_Allowed)    allowed << allAlignments.at(0);
    if (race.neutral == AS_Allowed) allowed << allAlignments.at(1);
    if (race.evil == AS_Allowed)    allowed << allAlignments.at(2);

    if (allowed.isEmpty()) return;

    this->alignmentBox->addItems(allowed);

    // 2. Determine the best default
    // We prefer the system default if available, otherwise fallback to the first allowed item
    const QString systemDefault = allAlignments.value(GameStateManager::defaultAlignmentIndex());
    int defaultIndex = this->alignmentBox->findText(systemDefault);
    
    this->alignmentBox->setCurrentIndex(defaultIndex != -1 ? defaultIndex : 0);
}

/*
void CreateCharacterDialog::updateAlignmentOptions(const RaceStats& race) {
    this->alignmentBox->blockSignals(true);
    this->alignmentBox->clear();
    // Retrieve the master list of strings from the State Manager
    QStringList allAlignments = GameStateManager::alignmentNames();
    QStringList allowedAlignments;
    // Filter based on RaceData permissions
    if (race.good == AS_Allowed) { 
        allowedAlignments.append(allAlignments.at(0)); // Good
    }
    if (race.neutral == AS_Allowed) {
        allowedAlignments.append(allAlignments.at(1)); // Neutral
    }
    if (race.evil == AS_Allowed) {
        allowedAlignments.append(allAlignments.at(2)); // Evil
    }
    this->alignmentBox->addItems(allowedAlignments);
    // Logic to select the best default for the race
    if (this->alignmentBox->count() > 0) {
        QString defaultName = allAlignments.at(GameStateManager::defaultAlignmentIndex());
        int index = this->alignmentBox->findText(defaultName);
        // If "Neutral" is allowed, select it; otherwise, select the first available option
        if (index != -1) {
            this->alignmentBox->setCurrentIndex(index);
        } else {
            this->alignmentBox->setCurrentIndex(0);
        }
    }
    this->alignmentBox->blockSignals(false);
}
*/

// Helper to update guild list styling based on race eligibility
void CreateCharacterDialog::updateGuildListStyle(const RaceStats& race) {
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

// Slot implementation to update all race-specific UI elements and reset stat points
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

    // Use GameStateManager to iterate through stat names
    for (const QString& statName : GameStateManager::statNames()) {
        QSpinBox* spinBox = statSpinBoxes.value(statName);
        if (!spinBox) continue;

        RaceStat raceStat = statMap.value(statName);

        spinBox->blockSignals(true);
        spinBox->setRange(raceStat.min, raceStat.min + 5);
        spinBox->setValue(raceStat.start);
        spinBox->setProperty(PROPERTY_PREVIOUS_VALUE, raceStat.start);
        spinBox->blockSignals(false);

        QLabel* rangeLabel = statRangeLabels.value(statName);
        if (rangeLabel) {
            rangeLabel->setText(QString("(%1-%2)").arg(raceStat.min).arg(raceStat.max));
        }

        QLabel* valueLabel = statValueLabels.value(statName);
        if (valueLabel) {
             valueLabel->setText(QString::number(raceStat.start));
        }
    }

    updateAlignmentOptions(selectedRace);
    updateGuildListStyle(selectedRace);

//    this->statPoints = 5;
//    this->statPointsLeftLabel->setText(QString("%1 Stat Points Left").arg(this->statPoints));

    GameStateManager::instance()->setGameValue("CurrentCharacterStatPointsLeft", GameStateManager::defaultStatPoints());
    this->statPointsLeftLabel->setText(QString("%1 Stat Points Left").arg(GameStateManager::defaultStatPoints()));
}

CreateCharacterDialog::CreateCharacterDialog(const QVector<RaceStats>& raceData, 
                                             const QVector<QString>& guildData, 
                                             QWidget *parent) 
    : QDialog(parent), 
      raceData(raceData), 
      guildData(guildData)
{
    setWindowTitle("Create Character");

    QFileInfo fileInfo(__FILE__);
    QString dir = fileInfo.absolutePath();
    QString qssPath = QDir(dir).filePath("createcharacterdialog.qss");
    
    QFile styleFile(qssPath);
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&styleFile);
        this->setStyleSheet(ts.readAll());
        styleFile.close();
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
    contentLayout->setSpacing(40);

    // Left Column: Character Info
    QGridLayout *infoLayout = new QGridLayout();
    infoLayout->setSpacing(10);
    infoLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    infoLayout->addWidget(new QLabel("Character Name"), 0, 0);
    nameEdit = new QLineEdit("");
    nameEdit->setPlaceholderText("Zuman");
    infoLayout->addWidget(nameEdit, 1, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Race"), 2, 0);
    raceBox = new QComboBox();
    QStringList raceNames;
    for (const RaceStats& race : raceData) {
        raceNames.append(race.raceName);
    }
    raceBox->addItems(raceNames);
    infoLayout->addWidget(raceBox, 3, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Sex"), 4, 0);
    sexBox = new QComboBox();
    sexBox->addItems(GameStateManager::sexOptions());
    infoLayout->addWidget(sexBox, 5, 0, 1, 2);

    infoLayout->addWidget(new QLabel("Alignment"), 6, 0);
    alignmentBox = new QComboBox();
    infoLayout->addWidget(alignmentBox, 7, 0, 1, 2);

    contentLayout->addLayout(infoLayout);
    contentLayout->addStretch(1);

    // Middle Column: Stats
    QGridLayout *statsLayout = new QGridLayout();
    statsLayout->setSpacing(10);
    statsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    QLabel *statsTitle = new QLabel("Stats");
    statsTitle->setStyleSheet("font-size: 14pt; font-weight: bold;");
    statsLayout->addWidget(statsTitle, 0, 0, 1, 4, Qt::AlignCenter);

    this->statPointsLeftLabel = new QLabel(QString("%1 Stat Points Left").arg(this->statPoints));
    this->statPointsLeftLabel->setStyleSheet("color: red; font-weight: bold; padding-top: 5px;");
    statsLayout->addWidget(this->statPointsLeftLabel, 7, 0, 1, 4, Qt::AlignCenter);
    
    const RaceStats& initialRace = raceData.at(0);

    // Refactored to use GameStateManager::statNames()
    const QStringList statList = GameStateManager::statNames();
    for (int i = 0; i < statList.size(); ++i) {
        QString statName = statList.at(i);
        statsLayout->addWidget(new QLabel(statName), i + 1, 0);

        QSpinBox *spinBox = new QSpinBox();
        RaceStat initialRaceStat;

        if (statName == "Strength") initialRaceStat = initialRace.strength;
        else if (statName == "Intelligence") initialRaceStat = initialRace.intelligence;
        else if (statName == "Wisdom") initialRaceStat = initialRace.wisdom;
        else if (statName == "Constitution") initialRaceStat = initialRace.constitution;
        else if (statName == "Charisma") initialRaceStat = initialRace.charisma;
        else if (statName == "Dexterity") initialRaceStat = initialRace.dexterity;
        else initialRaceStat = {1, 1, 18};

        spinBox->setRange(initialRaceStat.min, initialRaceStat.min + this->statPoints);
        spinBox->setValue(initialRaceStat.start);
        spinBox->setProperty(PROPERTY_PREVIOUS_VALUE, initialRaceStat.start);
        
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

	connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
	                  [currentValueLabel, this, spinBox](int newValue)
        {
	    GameStateManager* gsm = GameStateManager::instance();
	    int currentPoints = gsm->getGameValue("CurrentCharacterStatPointsLeft").toInt();
	    int oldValue = spinBox->property(PROPERTY_PREVIOUS_VALUE).toInt();
	    int change = newValue - oldValue;

	    if (change == 0) return;

	    if (change > 0) {
	        if (currentPoints >= change) { 
	            currentPoints -= change;
	        } else {
	            // ... (warning code)
	            return; 
	        }
	    } else { 
	        currentPoints -= change;
	    }

	    gsm->setGameValue("CurrentCharacterStatPointsLeft", currentPoints);
	    spinBox->setProperty(PROPERTY_PREVIOUS_VALUE, newValue);
            updateSpinBoxLimits();
            // Sync all spinboxes using the central stat list
//           for (const QString& name : GameStateManager::statNames()) {
//               QSpinBox* otherSpinBox = this->statSpinBoxes.value(name);
//               if (otherSpinBox) {
//                   otherSpinBox->blockSignals(true);
//                   otherSpinBox->setMaximum(otherSpinBox->value() + this->statPoints);
//                   otherSpinBox->blockSignals(false);
//               }
//            }
            
            currentValueLabel->setText(QString::number(newValue));
            this->statPointsLeftLabel->setText(QString("%1 Stat Points Left").arg(currentPoints));
        });
    }

    connect(raceBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     this, &CreateCharacterDialog::updateRaceStats);

    contentLayout->addLayout(statsLayout);
    contentLayout->addStretch(1);

    // Right Column: Guilds
    QVBoxLayout *guildLayout = new QVBoxLayout();
    guildLayout->setAlignment(Qt::AlignTop);

    QLabel *guildsTitle = new QLabel("Guilds Allowed");
    guildsTitle->setStyleSheet("font-size: 14pt; font-weight: bold;");
    guildLayout->addWidget(guildsTitle);

    guildsListWidget = new QListWidget();
    guildsListWidget->addItems(guildData);
    guildsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    guildLayout->addWidget(guildsListWidget);

    updateRaceStats(0);
    
    QLabel *selectedGuildLabel = new QLabel("Select a Guild");
    selectedGuildLabel->setStyleSheet("font-weight: bold; padding: 10px; background-color: #e0f7fa; border-radius: 5px; color: #333;");
    guildLayout->addWidget(selectedGuildLabel);

    connect(guildsListWidget, &QListWidget::currentItemChanged,
                     selectedGuildLabel, [selectedGuildLabel](QListWidgetItem *current) {
                         if (current && (current->flags() & Qt::ItemIsEnabled)) {
                             selectedGuildLabel->setText("Selected Guild: " + current->text());
                         } else {
                             selectedGuildLabel->setText("Select a Guild");
                         }
                     });

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

    mainLayout->addLayout(bottomLayout);

    connect(exitButton, &QPushButton::clicked, this, &CreateCharacterDialog::onExitClicked);
    connect(raceStatsButton, &QPushButton::clicked, this, &CreateCharacterDialog::onRaceStatsClicked);
    connect(guildStatsButton, &QPushButton::clicked, this, &CreateCharacterDialog::onGuildStatsClicked);
    connect(saveButton, &QPushButton::clicked, this, &CreateCharacterDialog::onSaveCharacterClicked);
    connect(tutorialButton, &QPushButton::clicked, this, &CreateCharacterDialog::onTutorialClicked);
}

void CreateCharacterDialog::updateSpinBoxLimits() {
    int pointsLeft = GameStateManager::instance()->getGameValue("CurrentCharacterStatPointsLeft").toInt();
    
    // Get the data for the currently selected race
    const RaceStats& selectedRace = raceData.at(raceBox->currentIndex());

    // Create a local mapping for easy lookup during the loop
    QMap<QString, RaceStat> statMap;
    statMap["Strength"]     = selectedRace.strength;
    statMap["Intelligence"] = selectedRace.intelligence;
    statMap["Wisdom"]       = selectedRace.wisdom;
    statMap["Constitution"] = selectedRace.constitution;
    statMap["Charisma"]     = selectedRace.charisma;
    statMap["Dexterity"]    = selectedRace.dexterity;

    for (const QString& statName : GameStateManager::statNames()) {
        QSpinBox* sb = statSpinBoxes.value(statName);
        if (!sb) continue;

        RaceStat raceStat = statMap.value(statName);

        sb->blockSignals(true);
        // The maximum is the lower of:
        // 1. The absolute racial maximum (e.g., 18)
        // 2. The current value + whatever points are remaining in the pool
        int allowedMax = qMin(raceStat.max, sb->value() + pointsLeft);
        sb->setMaximum(allowedMax);
        sb->blockSignals(false);
    }
}

CreateCharacterDialog::~CreateCharacterDialog() {}

void CreateCharacterDialog::onRaceStatsClicked() {
    QMessageBox::information(this, "Race Stats", "Displaying details for: " + raceBox->currentText());
}

void CreateCharacterDialog::onGuildStatsClicked() {
    QMessageBox::information(this, "Guild Stats", "Displaying details for available guilds.");
}

void CreateCharacterDialog::onSaveCharacterClicked() {
    QString characterName = nameEdit->text().trimmed();

    if (characterName.isEmpty()) {
        QMessageBox::warning(this, "Save Error", "Please enter a **Character Name** before saving.");
        return;
    }

    int remainingPoints = GameStateManager::instance()->getGameValue("CurrentCharacterStatPointsLeft").toInt();
    if (remainingPoints > 0) {
        QMessageBox::warning(this, "Stat Points Remaining", QString("You must spend the remaining **%1** stat point(s).").arg(remainingPoints));
        return;
    }

    QListWidgetItem* selectedGuildItem = guildsListWidget->currentItem();
    QString selectedGuild = (selectedGuildItem && (selectedGuildItem->flags() & Qt::ItemIsEnabled)) ? selectedGuildItem->text() : "None";
    
    // Update Game State Manager 
    GameStateManager* gsm = GameStateManager::instance();
    gsm->setGameValue("CurrentCharacterName", characterName);
    gsm->setGameValue("CurrentCharacterRace", raceBox->currentText());
    gsm->setGameValue("CurrentCharacterSex", sexBox->currentText());
    gsm->setGameValue("CurrentCharacterAlignment", alignmentBox->currentText());
    gsm->setGameValue("CurrentCharacterGuild", selectedGuild);
    
    // Use the central list to save values
    for (const QString& statName : GameStateManager::statNames()) {
        QSpinBox* spin = statSpinBoxes.value(statName);
        if (spin) {
            gsm->setGameValue(QString("CurrentCharacter%1").arg(statName), spin->value());
        }
    }
    gsm->setGameValue("CurrentCharacterStatPointsLeft", this->statPoints);

// Ensure the data/characters directory exists
    QString saveDirPath = QCoreApplication::applicationDirPath() + "/data/characters";
    QDir dir(saveDirPath);
    if (!dir.exists()) {
        dir.mkpath("."); // Creates the full path including parent folders if necessary
    }

    // Combine path and filename
    QString filename = dir.filePath(characterName + ".txt");
    QFile file(filename);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "CHARACTER_FILE_VERSION: 1.0\n";
        out << "Name: " << characterName << "\n";
        out << "Race: " << raceBox->currentText() << "\n";
        out << "Sex: " << sexBox->currentText() << "\n";
        out << "Alignment: " << alignmentBox->currentText() << "\n\n--- STATS ---\n";
        
        for (const QString& statName : GameStateManager::statNames()) {
            QSpinBox* spin = statSpinBoxes.value(statName);
            if (spin) {
                out << statName << ": " << spin->value() << "\n";
            }
        }
        out << "\n--- GUILD ---\nGuild: " << selectedGuild << "\n";
        file.close();
        
        QMessageBox::information(this, "Save Character", QString("Character **%1** saved successfully.").arg(characterName));
        emit characterCreated(characterName);
        QDialog::accept();
    }
}

void CreateCharacterDialog::onTutorialClicked() {
    QMessageBox::information(this, "Tutorial", "Instructions for character creation...");
}

void CreateCharacterDialog::onExitClicked() {
    this->reject();
}
