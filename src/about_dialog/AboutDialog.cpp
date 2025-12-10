#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QString>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Mordor Ordering Information");
    // Make the dialog non-resizeable
    setFixedSize(650, 500);
    // Remove the context help button (common for simple dialogs)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi();
}

AboutDialog::~AboutDialog() {}

QString AboutDialog::getGameVersionInfo() const {
    GameStateManager* gsm = GameStateManager::instance();
    QString version = gsm->getGameValue("GameVersion").toString();
    
    return QString(
        "Current Game Version: **%1**\n"
        ).arg(version);
}

void AboutDialog::setupUi() {
    GameStateManager::instance()->printAllGameState();
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QString gameStateInfo = getGameVersionInfo();
    QLabel *gameStateLabel = new QLabel(gameStateInfo);
    gameStateLabel->setWordWrap(true);
    gameStateLabel->setStyleSheet("font-size: 14px; font-weight: normal;");
    
    QString orderingText =
        "Whether you're ordering MORDOR for yourself or as a gift, this first of a kind Windows FRP\n"
        "game designed to be played for months and even longer is guaranteed to put a smile on most any avid\n"
        "FRP'er's face!! The full commercial version of MORDOR has so many more Monsters, Items and\n"
        "Dungeon Levels than the PUBLIC version that we can't even begin to get into detail.\n"
        "Find out for yourself!\n\n"
        "You can now pre-order the full version of MORDOR at any time for only $39.95 + $4!! Feel free to\n"
        "use your VISA, MASTERCARD, Check, or Money Order to have MORDOR: the Depths of Dejenol,\n\n"
        "To order, call TDA! at (800) 624-2101 between 9AM and 4PM PST Monday through Friday and ask to\n"
        "order MORDOR! Be sure to specify either 3.5'' disks or CD-ROM.\n\n"
        "You can also contact the company at the following EMail addresses:\n"
        "Internet: 102033,242@CompuServe.com\n"
        "CompuServe: 102033,242\n"
        "(Remember to include the proper Address & Credit Card information if you order via EMail!)\n\n"
        "Street address:\n"
        "TDA!\n"
        "720 132nd St SW #202\n"
        "Everett, WA 98204\n"
        "(206) 742-4145\n"
        "Created by [Vanderbull]";
        
    QLabel *infoText = new QLabel(orderingText);
    infoText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    infoText->setWordWrap(true);

    QPushButton *closeButton = new QPushButton("Ok");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(gameStateLabel);
    mainLayout->addWidget(infoText);
    mainLayout->addStretch();
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
}
