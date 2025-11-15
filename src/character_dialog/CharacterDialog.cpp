#include "CharacterDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QFrame>
#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QProgressBar>

CharacterDialog::CharacterDialog(const QString &charName, QWidget *parent)
    : QDialog(parent)
{
    // Set the window title
    setWindowTitle(charName);
    // Fixed size to match the retro appearance
    setFixedSize(350, 550);

    // Apply global stylesheet
    setStyleSheet(
        "QDialog { background-color: lightgray; }"
        "QLabel { font-size: 10pt; }"
        "QLineEdit { "
        "background-color: #E0E0E0; " // Slightly darker gray for input fields
        "border: 1px inset #A0A0A0; " // Sunken border effect
        "padding: 2px; "
        "}"
        "QTabWidget::pane { border: 1px solid gray; }" // Border around the tab content
        "QTabBar::tab { "
        "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 #E1E1E1, stop: 0.4 #DDDDDD, "
        "stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3); "
        "border: 1px solid gray; "
        "border-bottom-color: #C2C2C2; " // Separator for selected tab
        "border-top-left-radius: 4px; "
        "border-top-right-radius: 4px; "
        "min-width: 8ex; "
        "padding: 2px; "
        "}"
        "QTabBar::tab:selected, QTabBar::tab:hover { "
        "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 #FAFAFA, stop: 0.4 #F4F4F4, "
        "stop: 0.5 #E7E7E7, stop: 1.0 #FAFAFA); "
        "}"
        "QTabBar::tab:selected { border-bottom-color: lightgray; }" // Hide border on selected tab
        "QLabel#headerTitle { " // Example of ID selector for the main title
        "font-size: 12pt; "
        "font-weight: bold; "
        "text-align: center; "
        "background-color: darkblue; "
        "color: white; "
        "padding: 5px; "
        "}"
        // Add a style for the retro progress bar
        "QProgressBar { "
        "border: 1px solid #808080; "
        "background-color: #E0E0E0; "
        "text-align: center; "
        "color: black; "
        "}"
        "QProgressBar::chunk { "
        "background-color: darkgreen; "
        "width: 1px; "
        "}"
        );

    setupUi();
}

void CharacterDialog::setupUi() {
    // --- Main Layout for the Dialog ---
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // --- Tab Widget ---
    QTabWidget *tabWidget = new QTabWidget();
    // Styling for a classic/retro look with border and background
    tabWidget->setStyleSheet(
        // General pane styling (the area under the tabs)
        "QTabWidget::pane { border: 1px solid #808080; background-color: #C0C0C0; }"
        // Tab bar styling
        "QTabBar::tab { background: #E0E0E0; border: 1px solid #808080; border-bottom-color: #C0C0C0; "
        "min-width: 60px; padding: 4px; }"
        // Selected tab styling
        "QTabBar::tab:selected { background: #C0C0C0; border-bottom-color: #C0C0C0; }"
    );
    // Dummy tabs for visual resemblance
    QWidget *lookTabContent = new QWidget();
    QWidget *buffersTabContent = new QWidget();
    QWidget *resistTabContent = new QWidget();
    QWidget *miscTabContent = new QWidget();
    QWidget *charTabContent = new QWidget();
    QWidget *guildTabContent = new QWidget();

    tabWidget->addTab(lookTabContent, "Look");
    tabWidget->addTab(buffersTabContent, "Buffers");
    tabWidget->addTab(resistTabContent, "Resist.");
    tabWidget->addTab(miscTabContent, "Misc");
    tabWidget->addTab(charTabContent, "Char");
    tabWidget->addTab(guildTabContent, "Guild");

    // Select the "Char" tab (index 4)
    tabWidget->setCurrentIndex(4);

    // --- Layouts for the tab contents ---
    QVBoxLayout *lookLayout = new QVBoxLayout(lookTabContent);
    lookLayout->setContentsMargins(5, 5, 5, 5);
    QVBoxLayout *buffersLayout = new QVBoxLayout(buffersTabContent);
    buffersLayout->setContentsMargins(5, 5, 5, 5);
    QVBoxLayout *resistLayout = new QVBoxLayout(resistTabContent);
    resistLayout->setContentsMargins(5, 5, 5, 5);
    QVBoxLayout *miscLayout = new QVBoxLayout(miscTabContent);
    miscLayout->setContentsMargins(5, 5, 5, 5);
    QVBoxLayout *charLayout = new QVBoxLayout(charTabContent);
    charLayout->setContentsMargins(5, 5, 5, 5);
    QVBoxLayout *guildLayout = new QVBoxLayout(guildTabContent);
    guildLayout->setContentsMargins(5, 5, 5, 5);

    // --- 1. Top Section: Name, Class, Status (Shared Frame) ---
    QFrame *topFrame = new QFrame();
    topFrame->setFrameShape(QFrame::StyledPanel);
    topFrame->setFrameShadow(QFrame::Sunken); // Sunken panel effect
    topFrame->setStyleSheet("background-color: #C0C0C0; border: 1px solid #808080;");
    QVBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setContentsMargins(10, 5, 10, 5);
    topLayout->setSpacing(2);

    topLayout->addWidget(new QLabel("<b>Female Elf (Good)</b>"));
    topLayout->addWidget(new QLabel("<b>Vagrant (1)</b>"));
    QLabel *aloneLabel = new QLabel("You are Alone");
    aloneLabel->setAlignment(Qt::AlignCenter);
    // Darker button-like look for the status
    aloneLabel->setStyleSheet("background-color: #A0A0A0; padding: 2px; border: 1px solid #808080;");
    topLayout->addWidget(aloneLabel);

    // Add the shared frame to all layouts
    lookLayout->addWidget(topFrame);
    buffersLayout->addWidget(topFrame);
    resistLayout->addWidget(topFrame);
    miscLayout->addWidget(topFrame);
    charLayout->addWidget(topFrame);
    guildLayout->addWidget(topFrame);


    // #####################################################################
    // --- 2. Tab Content Implementation ---
    // #####################################################################

    // --- Tab: Look (Inventory/Equipment) ---
    QTableWidget *inventoryTable = new QTableWidget(5, 2); // 5 rows, 2 columns (Slot, Item)
    inventoryTable->setHorizontalHeaderLabels(QStringList() << "Slot" << "Item");
    inventoryTable->verticalHeader()->setVisible(false);
    inventoryTable->horizontalHeader()->setStretchLastSection(true);
    inventoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    inventoryTable->setStyleSheet(
        "QTableWidget { gridline-color: #808080; background-color: #EFEFEF; }"
        "QHeaderView::section { background-color: #D0D0D0; border: 1px outset #A0A0A0; padding: 2px; }"
    );

    inventoryTable->setItem(0, 0, new QTableWidgetItem("Head"));
    inventoryTable->setItem(0, 1, new QTableWidgetItem("Iron Helm"));
    inventoryTable->setItem(1, 0, new QTableWidgetItem("Body"));
    inventoryTable->setItem(1, 1, new QTableWidgetItem("Leather Armor"));
    inventoryTable->setItem(2, 0, new QTableWidgetItem("Hand"));
    inventoryTable->setItem(2, 1, new QTableWidgetItem("Short Sword"));
    inventoryTable->setItem(3, 0, new QTableWidgetItem("Feet"));
    inventoryTable->setItem(3, 1, new QTableWidgetItem("Boots of Speed"));
    inventoryTable->setItem(4, 0, new QTableWidgetItem("Backpack"));
    inventoryTable->setItem(4, 1, new QTableWidgetItem("12/50 lbs"));

    lookLayout->addWidget(inventoryTable);

    // --- Tab: Buffers (Temporary Effects/Spells) ---
    QLabel *buffsHeader = new QLabel("<b>Active Buffs and Debuffs:</b>");
    buffersLayout->addWidget(buffsHeader);

    QFrame *buffsFrame = new QFrame();
    buffsFrame->setFrameShape(QFrame::Box);
    buffsFrame->setFrameShadow(QFrame::Sunken);
    buffsFrame->setStyleSheet("background-color: white; border: 1px inset #808080;");
    QVBoxLayout *buffsListLayout = new QVBoxLayout(buffsFrame);
    buffsListLayout->setContentsMargins(5, 5, 5, 5);
    buffsListLayout->setSpacing(2);

    buffsListLayout->addWidget(new QLabel("Might (15:00 remaining)"));
    buffsListLayout->addWidget(new QLabel("Speed (05:30 remaining)"));
    buffsListLayout->addWidget(new QLabel("<font color='red'>Wounded</font> (-2 Att)"));
    buffsListLayout->addStretch();

    buffersLayout->addWidget(buffsFrame);

    // --- Tab: Resist. (Resistances) ---
    QGridLayout *resistGrid = new QGridLayout();
    resistGrid->setHorizontalSpacing(30);
    resistGrid->setVerticalSpacing(5);

    auto addResistance = [&](int row, const QString& type, int value) {
        resistGrid->addWidget(new QLabel(type), row, 0);
        QProgressBar *bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setValue(value);
        bar->setFormat(QString("%1%").arg(value)); // Show value as text
        resistGrid->addWidget(bar, row, 1);
    };

    addResistance(0, "Fire", 30);
    addResistance(1, "Ice", 50);
    addResistance(2, "Poison", 75);
    addResistance(3, "Lightning", 10);
    addResistance(4, "Magic", 45);

    resistLayout->addLayout(resistGrid);

    // --- Tab: Misc (Miscellaneous Info) ---
    QGridLayout *miscGrid = new QGridLayout();
    miscGrid->setHorizontalSpacing(20);
    miscGrid->setVerticalSpacing(5);

    miscGrid->addWidget(new QLabel("Kills"), 0, 0);
    miscGrid->addWidget(new QLabel("42"), 0, 1, Qt::AlignRight);
    miscGrid->addWidget(new QLabel("Deaths"), 1, 0);
    miscGrid->addWidget(new QLabel("1"), 1, 1, Qt::AlignRight);
    miscGrid->addWidget(new QLabel("Play Time"), 2, 0);
    miscGrid->addWidget(new QLabel("00:45:12"), 2, 1, Qt::AlignRight);
    miscGrid->addWidget(new QLabel("Alignment"), 3, 0);
    miscGrid->addWidget(new QLabel("Good"), 3, 1, Qt::AlignRight);

    QFrame *notesFrame = new QFrame();
    notesFrame->setFrameShape(QFrame::Box);
    notesFrame->setFrameShadow(QFrame::Sunken);
    notesFrame->setStyleSheet("background-color: white; border: 1px inset #808080;");
    QVBoxLayout *notesLayout = new QVBoxLayout(notesFrame);
    notesLayout->setContentsMargins(5, 5, 5, 5);
    QTextEdit *notesEdit = new QTextEdit("Traveler of the Western Realms. Seeking adventure and lost artifacts.");
    notesEdit->setReadOnly(true);
    notesLayout->addWidget(notesEdit);
    QLabel *notesHeader = new QLabel("<b>Character Notes:</b>");
    miscLayout->addWidget(notesHeader);

    miscLayout->addLayout(miscGrid);
    miscLayout->addWidget(notesFrame);


    // --- Tab: Char (Stats - Existing Content) ---
    // --- 2. Middle Section: Stats ---
    QFrame *statsFrame = new QFrame();
    statsFrame->setFrameShape(QFrame::StyledPanel);
    statsFrame->setFrameShadow(QFrame::Sunken);
    statsFrame->setStyleSheet("background-color: #C0C0C0; border: 1px solid #808080;");
    QGridLayout *statsLayout = new QGridLayout(statsFrame);
    statsLayout->setContentsMargins(10, 5, 10, 5);
    statsLayout->setHorizontalSpacing(50);
    statsLayout->setVerticalSpacing(2);

    // Row setup: Label (left), Value (right)
    statsLayout->addWidget(new QLabel("Age"), 0, 0);
    statsLayout->addWidget(new QLabel("16"), 0, 1, Qt::AlignRight);
    statsLayout->addWidget(new QLabel("Spells"), 1, 0);
    statsLayout->addWidget(new QLabel("100/100"), 1, 1, Qt::AlignRight);
    statsLayout->addWidget(new QLabel("Spell Level"), 2, 0);
    statsLayout->addWidget(new QLabel("1"), 2, 1, Qt::AlignRight);
    statsLayout->addWidget(new QLabel("Hits"), 3, 0);
    statsLayout->addWidget(new QLabel("15/15"), 3, 1, Qt::AlignRight);
    statsLayout->addWidget(new QLabel("Att/Def"), 4, 0);
    statsLayout->addWidget(new QLabel("10/10"), 4, 1, Qt::AlignRight);
    statsLayout->addWidget(new QLabel("Experience"), 5, 0);
    statsLayout->addWidget(new QLabel("0"), 5, 1, Qt::AlignRight);
    statsLayout->addWidget(new QLabel("Gold"), 6, 0);
    statsLayout->addWidget(new QLabel("1,500"), 6, 1, Qt::AlignRight);
    charLayout->addWidget(statsFrame);

    // --- 3. Bottom Section: Attributes ---
    QFrame *attrFrame = new QFrame();
    attrFrame->setFrameShape(QFrame::StyledPanel);
    attrFrame->setFrameShadow(QFrame::Sunken);
    attrFrame->setStyleSheet("background-color: #C0C0C0; border: 1px solid #808080;");
    QGridLayout *attrLayout = new QGridLayout(attrFrame);
    attrLayout->setContentsMargins(10, 5, 10, 5);
    attrLayout->setHorizontalSpacing(50);
    attrLayout->setVerticalSpacing(2);

    attrLayout->addWidget(new QLabel("Strength"), 0, 0);
    attrLayout->addWidget(new QLabel("10"), 0, 1, Qt::AlignRight);
    attrLayout->addWidget(new QLabel("Intelligence"), 1, 0);
    attrLayout->addWidget(new QLabel("10"), 1, 1, Qt::AlignRight);
    attrLayout->addWidget(new QLabel("Wisdom"), 2, 0);
    attrLayout->addWidget(new QLabel("10"), 2, 1, Qt::AlignRight);
    attrLayout->addWidget(new QLabel("Constitution"), 3, 0);
    attrLayout->addWidget(new QLabel("16"), 3, 1, Qt::AlignRight);
    attrLayout->addWidget(new QLabel("Charisma"), 4, 0);
    attrLayout->addWidget(new QLabel("7"), 4, 1, Qt::AlignRight);
    attrLayout->addWidget(new QLabel("Dexterity"), 5, 0);
    attrLayout->addWidget(new QLabel("10"), 5, 1, Qt::AlignRight);
    charLayout->addWidget(attrFrame);


    // --- Tab: Guild (Guild/Faction Info) ---
    QFrame *guildFrame = new QFrame();
    guildFrame->setFrameShape(QFrame::StyledPanel);
    guildFrame->setFrameShadow(QFrame::Sunken);
    guildFrame->setStyleSheet("background-color: #C0C0C0; border: 1px solid #808080;");
    QGridLayout *guildGrid = new QGridLayout(guildFrame);
    guildGrid->setContentsMargins(10, 5, 10, 5);
    guildGrid->setHorizontalSpacing(20);
    guildGrid->setVerticalSpacing(5);

    guildGrid->addWidget(new QLabel("<b>Guild Name</b>"), 0, 0);
    guildGrid->addWidget(new QLabel("None"), 0, 1, Qt::AlignRight);
    guildGrid->addWidget(new QLabel("<b>Rank</b>"), 1, 0);
    guildGrid->addWidget(new QLabel("Wanderer"), 1, 1, Qt::AlignRight);
    guildGrid->addWidget(new QLabel("<b>Contribution</b>"), 2, 0);
    guildGrid->addWidget(new QLabel("0"), 2, 1, Qt::AlignRight);
    guildLayout->addWidget(guildFrame);

    QFrame *factionFrame = new QFrame();
    factionFrame->setFrameShape(QFrame::StyledPanel);
    factionFrame->setFrameShadow(QFrame::Sunken);
    factionFrame->setStyleSheet("background-color: #C0C0C0; border: 1px solid #808080;");
    QVBoxLayout *factionLayout = new QVBoxLayout(factionFrame);
    factionLayout->setContentsMargins(10, 5, 10, 5);
    factionLayout->setSpacing(2);

    factionLayout->addWidget(new QLabel("<b>Faction Standing:</b>"));
    QProgressBar *factionBar = new QProgressBar();
    factionBar->setRange(-100, 100);
    factionBar->setValue(15); // Slightly positive
    factionBar->setFormat("Friendly (%v)");
    factionLayout->addWidget(factionBar);
    guildLayout->addWidget(factionFrame);


    // Add stretch to push content sections to the top where appropriate
    lookLayout->addStretch();
    buffersLayout->addStretch();
    resistLayout->addStretch();
    miscLayout->addStretch();
    // Char tab content already fills the space well
    guildLayout->addStretch();

    // Finalize the main dialog layout
    mainLayout->addWidget(tabWidget);
}
