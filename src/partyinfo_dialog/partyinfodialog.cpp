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
    setMinimumSize(400, 350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    for (int i = 0; i < 4; ++i) {
        QLabel *rowLabel = new QLabel("(empty slot)", this);
        rowLabel->setTextFormat(Qt::RichText); 
        rowLabel->setStyleSheet("margin-bottom: 8px; border-bottom: 1px solid #444;");
        memberLabels.append(rowLabel);
        mainLayout->addWidget(rowLabel);
    }

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    QPushButton *switchBtn = new QPushButton("Switch To", this);
    QPushButton *optionsBtn = new QPushButton("Options", this);
    QPushButton *leaveBtn = new QPushButton("Leave", this);

    connect(switchBtn, &QPushButton::clicked, this, &PartyInfoDialog::onSwitchToClicked);
    connect(optionsBtn, &QPushButton::clicked, this, &PartyInfoDialog::onOptionsClicked);
    connect(leaveBtn, &QPushButton::clicked, this, &PartyInfoDialog::onLeaveClicked);

    controlsLayout->addWidget(switchBtn);
    controlsLayout->addWidget(optionsBtn);
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

            // Data Extraction
            QString race = charMap.value("Race", "Unknown").toString();
            int hp = charMap.value("HP").toInt();

            int maxHp = charMap.value("MaxHP").toInt();
            
            // Status Logic
            QStringList status;
            if (charMap.value("Dead").toBool()) status << "<font color='red'>DEAD</font>";
            else {
                if (charMap.value("Poisoned").toBool()) status << "Poison";
                if (charMap.value("Diseased").toBool()) status << "Disease";
                if (status.isEmpty()) status << "Healthy";
            }

            QString activeMarker = (i == activeMemberIndex) ? "<b>[ACTIVE] " : "";
            QString activeSuffix = (i == activeMemberIndex) ? "</b>" : "";

            // UI Rendering
            memberLabels[i]->setText(QString(
                "%1%2 (%3)%4<br/>"
                "&nbsp;&nbsp;HP: %5/%6 | Status: <i>%7</i>")
                .arg(activeMarker).arg(name).arg(race).arg(activeSuffix)
                .arg(hp).arg(maxHp).arg(status.join(", ")));

        } else {
            memberLabels[i]->setText("<font color='gray'>(empty slot)</font>");
        }
    }
}

void PartyInfoDialog::onSwitchToClicked() {
    if (partyMembers.isEmpty()) return;
    bool ok;
    QString item = QInputDialog::getItem(this, "Switch Active", "Select member:", partyMembers, activeMemberIndex, false, &ok);
    if (ok && !item.isEmpty()) {
        GameStateManager::instance()->setGameValue("ActiveCharacterIndex", partyMembers.indexOf(item));
    }
}

void PartyInfoDialog::onOptionsClicked() {
    QMessageBox::information(this, "Options", "Management options coming soon.");
}

void PartyInfoDialog::onLeaveClicked() {
    if (partyMembers.isEmpty()) return;
    if (QMessageBox::question(this, "Leave", "Remove " + partyMembers.at(activeMemberIndex) + "?") == QMessageBox::Yes) {
        QVariantList partyData = GameStateManager::instance()->getGameValue("Party").toList();
        partyData.removeAt(activeMemberIndex);
        QVariantMap empty; empty["Name"] = "Empty Slot";
        partyData.append(empty);
        GameStateManager::instance()->setGameValue("Party", partyData);
    }
}
