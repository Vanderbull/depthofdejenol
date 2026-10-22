#include "InfoPanelDialog.h"
#include <QtWidgets>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include "src/core/gameStateManager.h"

InfoPanelDialog::InfoPanelDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

InfoPanelDialog::~InfoPanelDialog() = default;

void InfoPanelDialog::setupUi() {
    setWindowTitle(tr("Info Panels"));
    setMinimumSize(500, 400);

    auto *mainLayout = new QVBoxLayout(this);
    m_titleLabel = new QLabel(tr("Select a panel from the dropdown below."));
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(m_titleLabel);

    m_body = new QTextEdit;
    m_body->setReadOnly(true);
    m_body->setMinimumHeight(250);
    mainLayout->addWidget(m_body);

    auto *btnLayout = new QHBoxLayout;
    m_exitBtn = new QPushButton(tr("Close"));
    btnLayout->addWidget(m_exitBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_exitBtn, &QPushButton::clicked, this, &InfoPanelDialog::onExitClicked);

    // Default panel
    showHelp();
}

void InfoPanelDialog::onExitClicked() {
    accept();
}

void InfoPanelDialog::showHelp() {
    m_titleLabel->setText(tr("F1 — Help & Tips"));
    m_body->setPlainText(
        tr("Help & Tips\n"
           "\n"
           "• Press F5 to quicksave, F9 to quickload, F12 to autosave.\n"
           "• In the dungeon: WASD to move, Q/E to rotate, F to fight,\n"
           "  O to open, R to rest, H to eat, S to cast spell, U to teleport.\n"
           "• Press C to open the character sheet.\n"
           "• Press M to open the automap (the manual says F8)."));
}

void InfoPanelDialog::showAreaLook() {
    m_titleLabel->setText(tr("F2 — Area Look"));
    m_body->setPlainText(
        tr("Area Look — entities in current zone\n"
           "\n"
           "Shows entities in the current zone (city/dwarf). Simple text"
           "representation of what is around you.\n"
           "\n"
           "In practice a list of feats, hostile events,\n"
           "and nearby monsters is shown."));
}

void InfoPanelDialog::showResistance() {
    m_titleLabel->setText(tr("F4 — Resistance Information"));
    m_body->setPlainText(
        tr("Resistance Information\n"
           "\n"
           "Resistances and the rules for how damage is reduced.\n"
           "\n"
           "Resistance formulas:\n"
           "  Resist Fire     → reduced fire damage\n"
           "  Resist Cold     → reduced cold damage\n"
           "  Resist Acid     → reduced acid damage\n"
           "  Resist Shock    → reduced electric damage\n"
           "  Resist Mind     → reduced spell damage\n"
           "  Resist Dodge    → reduced one-handed risk\n"
           "  Resist Parry    → reduced risk melee\n"
           "  Resist Reflex   → increased dodge chance\n"
           "  Resist Mend     → faster healing\n"
           "  Resist Ailment  → resistant to status effects"));
}

void InfoPanelDialog::showBuffer() {
    m_titleLabel->setText(tr("F8 — Buffer Information"));
    m_body->setPlainText(
        tr("Buffer Information\n"
           "\n"
           "Your character's buff states and status-effect effects.\n"
           "\n"
           "Your buffs include:\n"
           "  • Cure (health)\n"
           "  • Shield (protection)\n"
           "  • Protection (guard)\n"
           "  • Evasion (evasion)\n"
           "  • Resist (resistance)\n"
           "  • Stamina (visible bar)\n"
           "  • Energy (energy)\n"
           "  • Combat Ready (ready for combat)\n"
           "  • ...}, etc."));
}
