#include "partyinfodialog.h"
#include "GameStateManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>

PartyInfoDialog::PartyInfoDialog(QWidget *parent) 
    : QDialog(parent), activeMemberIndex(0) 
{
    setWindowTitle("Party Information");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(450, 350); // Increased size to accommodate extra text

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    for (int i = 0; i < 4; ++i) {
        QLabel *rowLabel = new QLabel("(empty slot)", this);
        rowLabel->setTextFormat(Qt::RichText); // Enable HTML formatting
        rowLabel->setStyleSheet("padding: 5px; border-bottom: 1px solid #ccc;");
        memberLabels.append(rowLabel);
        mainLayout->addWidget(rowLabel);
    }

    QHBoxLayout *controlsLayout = new QHBoxLayout();

    QPushButton *switchBtn = new QPushButton("Switch To", this);
    connect(switchBtn, &QPushButton::clicked, this, &PartyInfoDialog::onSwitchToClicked);
    controlsLayout->addWidget(switchBtn);

    QPushButton *optionsBtn = new QPushButton("Options", this);
    connect(optionsBtn, &QPushButton::clicked, this, &PartyInfoDialog::onOptionsClicked);
    controlsLayout->addWidget(optionsBtn);

    QPushButton *leaveBtn = new QPushButton("Leave", this);
    connect(leaveBtn, &QPushButton::clicked, this, &PartyInfoDialog::onLeaveClicked);
    controlsLayout->addWidget(leaveBtn);

    mainLayout->addLayout(controlsLayout);

    refreshFromGameState();

    connect(GameStateManager::instance(), &GameStateManager::gameValueChanged,
            this, &PartyInfoDialog::onGameStateChanged);
}

PartyInfoDialog::~PartyInfoDialog() {}

void PartyInfoDialog::refreshFromGameState() {
    activeMemberIndex = GameStateManager::instance()->getGameValue("ActiveCharacterIndex").toInt();
    updatePartyLabels();
}

void PartyInfoDialog::onGameStateChanged(const QString& key, const QVariant& value) {
    if (key == "Party" || key == "ActiveCharacterIndex") {
        refreshFromGameState();
    }
}

void PartyInfoDialog::updatePartyLabels() {
    partyMembers.clear();
    QVariantList partyData = GameStateManager::instance()->getGameValue("Party").toList();
    
    for (int i = 0; i < 4; ++i) {
        if (i < partyData.size()) {
            QVariantMap charMap = partyData[i].toMap();
            QString name = charMap.value("Name").toString();

            if (name.isEmpty() || name == "Empty Slot") {
                memberLabels[i]->setText("<font color='gray'>(empty slot)</font>");
                continue;
            }

            partyMembers.append(name);

            // Extract detailed stats
            QString race = charMap.value("Race", "Unknown").toString();
            int hp = charMap.value("HP").toInt();
            int maxHp = charMap.value("MaxHP").toInt();
            
            // Build Status String
            QStringList status;
            if (charMap.value("Dead").toBool()) status << "<font color='red'>DEAD</font>";
            else {
                if (charMap.value("Poisoned").toBool()) status << "Poisoned";
                if (charMap.value("Blinded").toBool()) status << "Blinded";
                if (charMap.value("Diseased").toBool()) status << "Diseased";
                if (status.isEmpty()) status << "Healthy";
            }

            QString activeTag = (i == activeMemberIndex) ? "<b>[ACTIVE]</b> " : "";
            
            // Final Display String
            memberLabels[i]->setText(QString(
                "%1<b>%2</b> (%3)<br/>"
                "HP: %4/%5 | Status: <i>%6</i>")
                .arg(activeTag)
                .arg(name)
                .arg(race)
                .arg(hp)
                .arg(maxHp)
                .arg(status.join(", ")));
        } else {
            memberLabels[i]->setText("<font color='gray'>(empty slot)</font>");
        }
    }
}

void PartyInfoDialog::onSwitchToClicked() {
    if (partyMembers.isEmpty()) return;

    bool ok;
    QString item = QInputDialog::getItem(this, "Switch Character",
                                         "Select active member:", 
                                         partyMembers, activeMemberIndex, false, &ok);
    if (ok && !item.isEmpty()) {
        int newIndex = partyMembers.indexOf(item);
        GameStateManager::instance()->setGameValue("ActiveCharacterIndex", newIndex);
    }
}

void PartyInfoDialog::onOptionsClicked() {
    QMessageBox::information(this, "Options", "Party management options will appear here.");
}

void PartyInfoDialog::onLeaveClicked() {
    if (partyMembers.isEmpty()) return;

    QString leaverName = partyMembers.at(activeMemberIndex);
    auto reply = QMessageBox::question(this, "Leave Party", 
                                       "Remove <b>" + leaverName + "</b> from the party?");

    if (reply == QMessageBox::Yes) {
        QVariantList partyData = GameStateManager::instance()->getGameValue("Party").toList();
        if (activeMemberIndex < partyData.size()) {
            partyData.removeAt(activeMemberIndex);
            QVariantMap emptySlot;
            emptySlot["Name"] = "Empty Slot";
            partyData.append(emptySlot);
            GameStateManager::instance()->setGameValue("Party", partyData);
        }
    }
}
