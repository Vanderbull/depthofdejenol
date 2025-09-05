#include "createcharacterdialog.h"

#include <QScreen>
#include <QGuiApplication>
#include <QMessageBox>
#include <QFile>

CreateCharacterDialog::CreateCharacterDialog(QWidget *parent) : QDialog(parent) {
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
    raceBox->addItems({"Human", "Elf", "Dwarf"});
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
        statsLayout->addWidget(new QLabel("(4-18)"), i + 1, 2);
    }
    
    QLabel *statPointsLeft = new QLabel("3 Stat Points Left");
    statsLayout->addWidget(statPointsLeft, statNames.size() + 2, 0, 1, 3);
    
    contentLayout->addLayout(statsLayout);

    // Right Column: Guilds
    QVBoxLayout *guildLayout = new QVBoxLayout();
    guildLayout->setAlignment(Qt::AlignTop);
    
    QLabel *guildsTitle = new QLabel("Guilds Allowed");
    guildLayout->addWidget(guildsTitle);
    
    QLabel *guildsList = new QLabel("Nomad\nWarrior\nPaladin\nNinja\nVillain\nSeeker\nThief\nScavenger\nMage\nSorcerer\nWizard\nHealer");
    guildLayout->addWidget(guildsList);
    
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
