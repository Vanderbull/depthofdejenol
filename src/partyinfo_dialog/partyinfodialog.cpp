#include "partyinfodialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QGuiApplication>
#include <QFile>

PartyInfoDialog::PartyInfoDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Party Information");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Load external style sheet once
    QFile styleSheetFile("style.qss");
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }

    // Get screen geometry to size window once
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    int windowWidth = screenWidth * 0.40;
    int windowHeight = screenHeight * 0.35;
    setMinimumSize(400, 300);
    resize(windowWidth, windowHeight);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Create and add four member labels
    for (int i = 0; i < 4; ++i) {
        QLabel *rowLabel = new QLabel("(empty slot)", this);
        memberLabels.append(rowLabel);
        mainLayout->addWidget(rowLabel);
    }

    // Action buttons
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(new QPushButton("Switch To", this));
    controlsLayout->addWidget(new QPushButton("Options", this));
    controlsLayout->addWidget(new QPushButton("Leave", this));
    mainLayout->addLayout(controlsLayout);
}

PartyInfoDialog::~PartyInfoDialog() {}

void PartyInfoDialog::setPartyMembers(const QStringList& members) {
    partyMembers = members;
    qDebug() << "Party size:" << partyMembers.size() << partyMembers;
    for (int i = 0; i < 4; ++i) {
        memberLabels[i]->setText((i < partyMembers.size()) ? partyMembers.at(i) : "(empty slot)");
    }
}
