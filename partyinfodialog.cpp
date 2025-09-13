#include "partyinfodialog.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QGuiApplication>
#include <QFile>

PartyInfoDialog::PartyInfoDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Party Information - Test");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi();
}

PartyInfoDialog::~PartyInfoDialog() {}

void PartyInfoDialog::setupUi() {
    // Load external style sheet
    QFile styleSheetFile("style.qss");
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }

    // Get screen geometry to make the window scale with screen resolution
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    
    // Calculate window size as a percentage of screen size
    int windowWidth = screenWidth * 0.40;
    int windowHeight = screenHeight * 0.35;
    
    setMinimumSize(400, 300);
    resize(windowWidth, windowHeight);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Party list view
    QLabel *partyListLabel = new QLabel("Party Information Placeholder");
    partyListLabel->setStyleSheet("background-color: #2b2b2b; color: #878787; border: 2px solid #5a5a5a; padding: 10px;");
    mainLayout->addWidget(partyListLabel);
    
    // Action buttons
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    QPushButton *switchButton = new QPushButton("Switch To");
    QPushButton *optionsButton = new QPushButton("Options");
    QPushButton *leaveButton = new QPushButton("Leave");
    
    controlsLayout->addWidget(switchButton);
    controlsLayout->addWidget(optionsButton);
    controlsLayout->addWidget(leaveButton);
    
    mainLayout->addLayout(controlsLayout);
}
