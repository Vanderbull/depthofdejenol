#include "AboutDialog.h"
#include "GameStateManager.h"
#include "../update/UpdateDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QVariant>
#include <QUrl>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) 
{
    setWindowTitle("Depth of Dejenol: Blacklands"); 
    setFixedSize(650, 500);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi();
}

// Function retained for internal logic but removed from UI per request
QString AboutDialog::getGameVersionInfo() const
{
    GameStateManager* gsm = GameStateManager::instance();
    return gsm->getGameValue("GameVersion").toString();
}

void AboutDialog::setupUi()
{
    // 1. Main Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 2. Scroll Area Setup
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);

    // 3. Construct Text (Integrated Version Info)
    QString version = getGameVersionInfo();
    QString aboutText = QString(
        "<h2>DEPTH OF DEJENOL: BLACKLANDS</h2>"
        "<p><b>Version: %1</b></p>"
        "<i>A Grand-Scale Fantasy Role Playing Adventure</i><br><br>"
        "<b>THE LEGEND</b><br>"
        "Welcome to the world of the Mines of Marlith, now known as the Depth of Dejenol. "
        "In this massive dungeon crawler, you must lead a party of adventurers into a mysterious "
        "labyrinth on a nearly endless mission of exploration, mapping, and looting.<br><br>"
        "<b>GAME FEATURES</b><br>"
        "• <b>Deep Character Progression:</b> Create heroes from multiple races and join specialized guilds.<br>"
        "• <b>Tactical Combat:</b> A unique blend of real-time action and strategic party management.<br>"
        "• <b>Vast Labyrinth:</b> unknown number of procedurally generated dungeon filled with hundreds of monsters and items.<br><br>"
        "<b>CREDITS</b><br>"
        "<b>Game Design & Programming:</b> Vanderbull<br><br>"
        "<b>HOMAGE</b><br>"
        "This project is a tribute to the legendary work of <b>David Allen</b>. We honor his "
        "original vision, game design, and code for the classic 1995 release of 'Mordor: The Depths of Dejenol,' "
        "which paved the way for this adventure."
    ).arg(version);


    QLabel *infoText = new QLabel(aboutText);
    infoText->setWordWrap(true);
    infoText->setTextFormat(Qt::RichText);
    infoText->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    // 4. Assemble Scroll Content
    scrollLayout->addWidget(infoText);

    QPushButton *checkUpdatesBtn = new QPushButton(tr("Check for Updates"), this);
    checkUpdatesBtn->setFixedWidth(150);
    scrollLayout->addWidget(checkUpdatesBtn);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);

    // 5. Bottom Button Row
    QPushButton *closeButton = new QPushButton(tr("Ok"));
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    // 6. Final Assembly
    mainLayout->addWidget(scrollArea);
    mainLayout->addLayout(buttonLayout);

    // 7. Connections
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    connect(checkUpdatesBtn, &QPushButton::clicked, this, [this]() {
        UpdateDialog dlg(this);
        // Manifest placed at the repository root -> raw.githubusercontent.com path
        dlg.setManifestUrl(QUrl(QStringLiteral("https://raw.githubusercontent.com/Vanderbull/depthofdejenol/main/manifest.json")));
        dlg.exec();
    });
}

AboutDialog::~AboutDialog() {}
