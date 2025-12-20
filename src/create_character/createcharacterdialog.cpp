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
const static QStringList STAT_NAMES = {"Strength", "Intelligence", "Wisdom", "Constitution", "Charisma", "Dexterity"};

// Helper function to update alignment options
void CreateCharacterDialog::updateAlignmentOptions(const RaceStats& race) {
    this->alignmentBox->clear();
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

    for (auto it = statSpinBoxes.constBegin(); it != statSpinBoxes.constEnd(); ++it) {
        QString statName = it.key();
        QSpinBox* spinBox = it.value();
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

    this->statPoints = 5;
    this->statPointsLeftLabel->setText(QString("%1 Stat Points Left").arg(this->statPoints));
}

CreateCharacterDialog::CreateCharacterDialog(const QVector<RaceStats>& raceData, 
                                             const QVector<QString>& guildData, 
                                             QWidget *parent) 
    : QDialog(parent), 
      raceData(raceData), 
      guildData(guildData) 
{
    setWindowTitle("Create Character");

    // Fix: Ensure proper path separator for QSS loading 
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
    sexBox->addItems({"Male", "Female"});
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

    for (int i = 0; i < STAT_NAMES.size(); ++i) {
        QString statName = STAT_NAMES.at(i);
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
                          [currentValueLabel, this, spinBox](int newValue) {

            int oldValue = spinBox->property(PROPERTY_PREVIOUS_VALUE).toInt();
            int change = newValue - oldValue;

            if (change == 0) return;

            if (change > 0) {
                if (this->statPoints >= change) { 
                    this->statPoints -= change;
                } else {
                    QMessageBox::warning(this, "Stat Points", "You have no remaining stat points to spend.");
                    spinBox->blockSignals(true);
                    spinBox->setValue(oldValue); 
                    spinBox->blockSignals(false);
                    return; 
                }
            } else { 
                this->statPoints -= change; 
            }

            spinBox->setProperty(PROPERTY_PREVIOUS_VALUE, newValue);
            
            for (const QString& name : STAT_NAMES) {
                QSpinBox* otherSpinBox = this->statSpinBoxes.value(name);
                if (otherSpinBox) {
                    otherSpinBox->blockSignals(true);
                    otherSpinBox->setMaximum(otherSpinBox->value() + this->statPoints);
                    otherSpinBox->blockSignals(false);
                }
            }
            
            currentValueLabel->setText(QString::number(newValue));
            this->statPointsLeftLabel->setText(QString("%1 Stat Points Left").arg(this->statPoints));
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
    
    if (this->statPoints > 0) {
        QMessageBox::warning(this, "Stat Points Remaining", QString("You must spend the remaining **%1** stat point(s).").arg(this->statPoints));
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
    
    for (auto it = statSpinBoxes.constBegin(); it != statSpinBoxes.constEnd(); ++it) {
        gsm->setGameValue(QString("CurrentCharacter%1").arg(it.key()), it.value()->value());
    }
    gsm->setGameValue("CurrentCharacterStatPointsLeft", this->statPoints);

    // Write to file 
    QString filename = characterName + ".txt";
    QFile file(filename);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "CHARACTER_FILE_VERSION: 1.0\n";
        out << "Name: " << characterName << "\n";
        out << "Race: " << raceBox->currentText() << "\n";
        out << "Sex: " << sexBox->currentText() << "\n";
        out << "Alignment: " << alignmentBox->currentText() << "\n\n--- STATS ---\n";
        for (auto it = statSpinBoxes.constBegin(); it != statSpinBoxes.constEnd(); ++it) {
            out << it.key() << ": " << it.value()->value() << "\n";
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
