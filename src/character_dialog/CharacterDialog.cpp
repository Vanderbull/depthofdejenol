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
}

void CharacterDialog::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    QTabWidget *tabWidget = new QTabWidget();
    QWidget *charTab = new QWidget();
    QWidget *buffsTab = new QWidget();
    QWidget *guildTab = new QWidget();

    tabWidget->addTab(charTab, "Char");
    tabWidget->addTab(buffsTab, "Buffers");
    tabWidget->addTab(guildTab, "Guild");

    // --- 1. Character Stats Tab ---
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
    charLayout->addWidget(headerFrame);

    // Attributes
    QGroupBox *attrGroup = new QGroupBox("Attributes");
    QGridLayout *attrLayout = new QGridLayout(attrGroup);
    QStringList attrs = {"Strength", "Intelligence", "Dexterity", "Wisdom", "Constitution"};
    for(int i = 0; i < attrs.size(); ++i) {
        attrLayout->addWidget(new QLabel(attrs[i] + ":"), i, 0);
        attrLayout->addWidget(new QLabel(GSM->getGameValue("CurrentCharacter" + attrs[i]).toString()), i, 1, Qt::AlignRight);
    }
    charLayout->addWidget(attrGroup);

    // Wealth
    QHBoxLayout *goldLayout = new QHBoxLayout();
    goldLayout->addWidget(new QLabel("Gold:"));
    goldLayout->addStretch();
    goldLayout->addWidget(new QLabel(QLocale().toString(GSM->getGameValue("PlayerGold").toULongLong())));
    charLayout->addLayout(goldLayout);

    charLayout->addStretch();

    // --- 2. Buffers Tab ---
    QVBoxLayout *buffsLayout = new QVBoxLayout(buffsTab);
    if (GSM->isCharacterPoisoned()) buffsLayout->addWidget(new QLabel("🤢 Poisoned"));
    if (GSM->isCharacterBlinded())  buffsLayout->addWidget(new QLabel("🕶️ Blinded"));
    if (!GSM->isCharacterPoisoned() && !GSM->isCharacterBlinded()) buffsLayout->addWidget(new QLabel("Status: Normal"));
    buffsLayout->addStretch();

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
