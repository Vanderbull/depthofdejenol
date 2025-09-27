#include "CharacterDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QFrame>
#include <QWidget>

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
    tabWidget->addTab(new QWidget(), "Look");
    tabWidget->addTab(new QWidget(), "Buffers");
    tabWidget->addTab(new QWidget(), "Resist.");
    tabWidget->addTab(new QWidget(), "Misc");
    tabWidget->addTab(new QWidget(), "Char");
    tabWidget->addTab(new QWidget(), "Guild");

    // Select the "Char" tab (index 4)
    tabWidget->setCurrentIndex(4);

    // --- Content for the "Char" tab ---
    QWidget *charTabContent = new QWidget();
    QVBoxLayout *charLayout = new QVBoxLayout(charTabContent);
    charLayout->setContentsMargins(0, 0, 0, 0);
    charLayout->setSpacing(5);

    // --- 1. Top Section: Name, Class, Status ---
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
    charLayout->addWidget(topFrame);

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

    // Add stretch to push content sections to the top
    charLayout->addStretch();

    // Finalize the "Char" tab layout
    tabWidget->widget(4)->setLayout(charLayout);

    // Finalize the main dialog layout
    mainLayout->addWidget(tabWidget);
}
