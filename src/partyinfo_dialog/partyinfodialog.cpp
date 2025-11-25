#include "partyinfodialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QFile>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

PartyInfoDialog::PartyInfoDialog(QWidget *parent) : QDialog(parent), activeMemberIndex(0) {
    setWindowTitle("Party Information");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // [StyleSheet and Geometry logic remains the same as previous code...]
    setMinimumSize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Create labels
    for (int i = 0; i < 4; ++i) {
        QLabel *rowLabel = new QLabel("(empty slot)", this);
        // Set text format to RichText so <b> tags work
        rowLabel->setTextFormat(Qt::RichText); 
        memberLabels.append(rowLabel);
        mainLayout->addWidget(rowLabel);
    }

    // Buttons
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
}

PartyInfoDialog::~PartyInfoDialog() {}

void PartyInfoDialog::setPartyMembers(const QStringList& members) {
    partyMembers = members;
    // Reset active index to 0 (leader) when loading new list, or keep logic to remember
    if (activeMemberIndex >= partyMembers.size()) {
        activeMemberIndex = 0;
    }
    updatePartyLabels();
}

void PartyInfoDialog::updatePartyLabels() {
    for (int i = 0; i < 4; ++i) {
        if (i < partyMembers.size()) {
            QString name = partyMembers.at(i);
            
            // If this is the active member, wrap in HTML bold tags
            if (i == activeMemberIndex) {
                memberLabels[i]->setText("<b>" + name + " (Active)</b>");
            } else {
                memberLabels[i]->setText(name);
            }
        } else {
            memberLabels[i]->setText("(empty slot)");
        }
    }
}

// --- Slots ---

void PartyInfoDialog::onSwitchToClicked() {
    if (partyMembers.isEmpty()) return;

    bool ok;
    QString item = QInputDialog::getItem(this, "Switch Character",
                                         "Select active member:", 
                                         partyMembers, activeMemberIndex, false, &ok);
    if (ok && !item.isEmpty()) {
        // Update the active index based on selection
        activeMemberIndex = partyMembers.indexOf(item);
        updatePartyLabels(); // Refresh to show the bold change
    }
}

void PartyInfoDialog::onOptionsClicked() {
    QMessageBox::information(this, "Options", "Options clicked");
}

void PartyInfoDialog::onLeaveClicked() {
    if (partyMembers.isEmpty()) return;

    // Get the name of the person leaving
    QString leaverName = partyMembers.at(activeMemberIndex);

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Leave Party", 
                                  "Remove <b>" + leaverName + "</b> from the party?",
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Remove the member at the active index
        partyMembers.removeAt(activeMemberIndex);

        // Adjust index if we removed the last person in the list
        if (activeMemberIndex >= partyMembers.size() && activeMemberIndex > 0) {
            activeMemberIndex--;
        }

        // Refresh the UI
        updatePartyLabels();
        
        // Optional: Close dialog if party is empty?
        if (partyMembers.isEmpty()) {
            QMessageBox::information(this, "Party Empty", "The party is now empty.");
        }
    }
}
