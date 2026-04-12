#include "AboutDialog.h"
#include "GameStateManager.h"
#include "../update/UpdateDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QUrl>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) 
{
    setWindowTitle("Depth of Dejenol: Blacklands"); 
    setFixedSize(650, 500);
    setWindowFlags(windowFlags().setFlag(Qt::WindowContextHelpButtonHint, false));
    setupUi();
}

QString AboutDialog::getGameVersionInfo()
{
    // Retrieve the string pushed to Lua in GameStateManager's constructor
    if (auto* gsm = GameStateManager::instance()) {
        QString version = gsm->getLuaString("GameVersion");
        if (!version.isEmpty()) {
            return version;
        }
    }
    return QStringLiteral("v0-unknown");
}

void AboutDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollContent = new QWidget();
    auto *scrollLayout = new QVBoxLayout(scrollContent);

    auto* gsm = GameStateManager::instance();
    const QString version = getGameVersionInfo();
    
    // Fetch HTML body from Lua script "AboutText"
    QString aboutText = gsm->getLuaString("AboutText");

    if (aboutText.isEmpty()) {
        aboutText = QString("<h2>DEPTH OF DEJENOL: BLACKLANDS</h2>"
                            "<p><b>Version: %1</b></p>").arg(version);
    } else {
        aboutText = aboutText.arg(version);
    }

    auto *infoText = new QLabel(aboutText);
    infoText->setWordWrap(true);
    infoText->setTextFormat(Qt::RichText);
    infoText->setAlignment(Qt::AlignTop);

    scrollLayout->addWidget(infoText);

    auto *checkUpdatesBtn = new QPushButton(tr("Check for Updates"), this);
    checkUpdatesBtn->setFixedWidth(180);
    scrollLayout->addWidget(checkUpdatesBtn, 0, Qt::AlignCenter);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);

    auto *closeButton = new QPushButton(tr("Ok"));
    closeButton->setDefault(true); 
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    mainLayout->addWidget(scrollArea);
    mainLayout->addLayout(buttonLayout);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    connect(checkUpdatesBtn, &QPushButton::clicked, this, [this]() {
        UpdateDialog dlg(this);
        dlg.setManifestUrl(QUrl(QStringLiteral("https://raw.githubusercontent.com/Vanderbull/depthofdejenol/main/manifest.json")));
        dlg.exec();
    });
}

AboutDialog::~AboutDialog() = default;
