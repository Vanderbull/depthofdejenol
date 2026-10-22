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
        tr("Hjälp & Ledsningar\n"
           "\n"
           "• Press F5 to quicksave, F9 to quickload, F12 to autosave.\n"
           "• In the dungeon: WASD to move, Q/E to rotate, F to fight,\n"
           "  O to open, R to rest, H to eat, S to cast spell, U to teleport.\n"
           "• Press C to open the character sheet.\n"
           "• Press M to open the automap (manualen anger F8)."));
}

void InfoPanelDialog::showAreaLook() {
    m_titleLabel->setText(tr("F2 — Area Look"));
    m_body->setPlainText(
        tr("Area Look — entiteter i nuvarande zon\n"
           "\n"
           "Visar entiteter i nuvarande zonen (staden/dvärg). "
           "Enkel textrepresentation av vad som finns runt omkring.\n"
           "\n"
           "I praktiken visas en lista med förnemmelser, fientliga händelser,\n"
           "fastigheter och monstren i närheten."));
}

void InfoPanelDialog::showResistance() {
    m_titleLabel->setText(tr("F4 — Resistance Information"));
    m_body->setPlainText(
        tr("Resistance Information\n"
           "\n"
           "Entydiga resistor samt regler för hur skada reduceras.\n"
           "\n"
           "Resist-formler:\n"
           "  Resist Fire     → reducerad eldskada\n"
           "  Resist Cold     → reducerad kallskada\n"
           "  Resist Acid     → reducerad syrefördel\n"
           "  Resist Shock    → reducerad elskada\n"
           "  Resist Mind     → reducerad trolldomsskada\n"
           "  Resist Dodge    → reduced risk etta\n"
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
           "Visar statusen för ditt chars buffertar och statusars effekter.\n"
           "\n"
           "Tänk på buffertar som:\n"
           "  • Cure (hälsa)\n"
           "  • Shield (skydd)\n"
           "  • Protection (skydd)\n"
           "  • Evasion (undvikande)\n"
           "  • Resist (resistans)\n"
           "  • Stamina (utåt)\n"
           "  • Energy (energi)\n"
           "  • Combat Ready (klar för strid)\n"
           "  • ...}, etc."));
}
