// CharacterDialog.cpp
#include "CharacterDialog.h"

// Qt Includes
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
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QProgressBar>
#include <QGroupBox>
#include <QLocale>

// Shorthand for the singleton
#define GSM GameStateManager::instance()

CharacterDialog::CharacterDialog(QWidget *parent)
    : QDialog(parent)
{
    // --- Set the dialog title to the character name from GameStateManager ---
    QString charName = GSM->getGameValue("CurrentCharacterName").toString();
    if (charName.isEmpty()) charName = "Unknown Traveler";
    setWindowTitle(charName);

    setFixedSize(360, 580);

    // Retro UI Styling
    setStyleSheet(
        "QDialog { background-color: #C0C0C0; }"
        "QLabel { font-size: 10pt; color: black; }"
        "QGroupBox { font-weight: bold; border: 2px groove #808080; margin-top: 10px; padding-top: 5px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 3px; }"
        "QTabWidget::pane { border: 1px solid #808080; }"
        "QTabBar::tab { background: #D0D0D0; border: 1px solid #808080; padding: 5px; min-width: 50px; }"
        "QTabBar::tab:selected { background: #C0C0C0; border-bottom-color: #C0C0C0; }"
    );

    setupUi();

    // NEW: Connect the GameStateManager signal to the dialog's update slot
    connect(GSM, &GameStateManager::gameValueChanged, 
            this, &CharacterDialog::updateGameStateValue);
}

// NEW: Implementation of the slot to update UI based on state changes
void CharacterDialog::updateGameStateValue(const QString& key, const QVariant& value)
{
    // Check if the key that changed is the gold key
    if (key == "CurrentCharacterGold" && m_goldValueLabel) 
    {
        // Update the label's text with the new gold value, formatted with locale
        QString newGoldStr = QLocale().toString(value.toULongLong());
        m_goldValueLabel->setText(newGoldStr);
    }
}

void CharacterDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    QTabWidget *tabWidget = new QTabWidget();
    QWidget *lookTab = new QWidget();
    QWidget *buffsTab = new QWidget();
    QWidget *miscTab = new QWidget();
    QWidget *statsTab = new QWidget();
    QWidget *resistanceTab = new QWidget();
    QWidget *charTab = new QWidget();
    QWidget *guildTab = new QWidget();

    tabWidget->addTab(lookTab, "Look");
    tabWidget->addTab(buffsTab, "Buffers");
    tabWidget->addTab(miscTab, "Misc");
    tabWidget->addTab(statsTab, "Stats");
    tabWidget->addTab(resistanceTab, "Resist");
    tabWidget->addTab(charTab, "Char");
    tabWidget->addTab(guildTab, "Guild");

    QVBoxLayout *lookLayout = new QVBoxLayout(lookTab);
    QVBoxLayout *miscLayout = new QVBoxLayout(miscTab);
    QVBoxLayout *statsLayout = new QVBoxLayout(statsTab);
    //QVBoxLayout *resistanceLayout = new QVBoxLayout(resistanceTab);
    QVBoxLayout *charLayout = new QVBoxLayout(charTab);

    // Identity Header (Class and Level)
    QFrame *headerFrame = new QFrame();
    headerFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    headerFrame->setStyleSheet("background-color: #D0D0D0;");
    QVBoxLayout *headerLayout = new QVBoxLayout(headerFrame);

    headerLayout->addWidget(new QLabel(QString("<b>%1 %2</b>")
        .arg(GSM->getGameValue("CurrentCharacterSex").toString())
        .arg(GSM->getGameValue("CurrentCharacterRace").toString())));
    
    headerLayout->addWidget(new QLabel(QString("Level %1 Vagrant (%2)")
        .arg(GSM->getGameValue("CurrentCharacterLevel").toInt())
        .arg(GSM->getGameValue("CurrentCharacterAlignment").toString())));
    statsLayout->addWidget(headerFrame);

    // Attributes
    QGroupBox *attrGroup = new QGroupBox("Attributes");
    QGridLayout *attrLayout = new QGridLayout(attrGroup);
    QStringList attrs = {"Strength", "Intelligence", "Dexterity", "Wisdom", "Constitution"};
    for(int i = 0; i < attrs.size(); ++i) {
        attrLayout->addWidget(new QLabel(attrs[i] + ":"), i, 0);
        attrLayout->addWidget(new QLabel(GSM->getGameValue("CurrentCharacter" + attrs[i]).toString()), i, 1, Qt::AlignRight);
    }
    statsLayout->addWidget(attrGroup);

    // Wealth
    QHBoxLayout *goldLayout = new QHBoxLayout();
    goldLayout->addWidget(new QLabel("Gold:"));
    goldLayout->addStretch();
    
    // CHANGED: Create and store the gold label pointer in the member variable
    m_goldValueLabel = new QLabel(QLocale().toString(
                                    GSM->getGameValue("CurrentCharacterGold").toULongLong()
                                  ));
    goldLayout->addWidget(m_goldValueLabel);
    statsLayout->addLayout(goldLayout);

    statsLayout->addStretch();

    // --- 2. Buffers Tab ---
    QVBoxLayout *buffsLayout = new QVBoxLayout(buffsTab);
    if (GSM->isCharacterPoisoned()) buffsLayout->addWidget(new QLabel("Poisoned"));
    if (GSM->isCharacterBlinded())  buffsLayout->addWidget(new QLabel("Blinded"));
    if (!GSM->isCharacterPoisoned() && !GSM->isCharacterBlinded()) buffsLayout->addWidget(new QLabel("Status: Normal"));
    buffsLayout->addStretch();

    // --- Resistance Tab ---
    QVBoxLayout *resistanceLayout = new QVBoxLayout(resistanceTab);
    resistanceLayout->addWidget(new QLabel("Current Resistance"));
    resistanceLayout->addWidget(new QLabel("Fire             -"));
    resistanceLayout->addWidget(new QLabel("Cold             -"));
    resistanceLayout->addWidget(new QLabel("Electrical       -"));
    resistanceLayout->addWidget(new QLabel("Mind             -"));
    resistanceLayout->addWidget(new QLabel("Disease          -"));
    resistanceLayout->addWidget(new QLabel("Poison           -"));
    resistanceLayout->addWidget(new QLabel("Magic            -"));
    resistanceLayout->addWidget(new QLabel("Stone            -"));
    resistanceLayout->addWidget(new QLabel("Paralysis        -"));
    resistanceLayout->addWidget(new QLabel("Drain            -"));
    resistanceLayout->addWidget(new QLabel("Acid             -"));

    // --- 3. Guild Tab ---
    QVBoxLayout *guildLayout = new QVBoxLayout(guildTab);
    QTextEdit *logView = new QTextEdit();
    logView->setReadOnly(true);
    QVariantList actions = GSM->getGameValue("GuildActionLog").toList();
    for (const QVariant& action : actions) logView->append(action.toString());
    guildLayout->addWidget(new QLabel("Action Log:"));
    guildLayout->addWidget(logView);

    mainLayout->addWidget(tabWidget);

    QPushButton *btnOk = new QPushButton("Close");
    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(btnOk, 0, Qt::AlignRight);
}
