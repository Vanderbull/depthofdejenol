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
    // Ensure instance existence before call
    if (auto* gsm = GameStateManager::instance()) {
        return gsm->getGameValue(QStringLiteral("GameVersion")).toString();
    }
    return QStringLiteral("Unknown");
}

void AboutDialog::setupUi()
{
    // 1. Main Layout
    auto *mainLayout = new QVBoxLayout(this);

    // 2. Scroll Area Setup
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto *scrollContent = new QWidget();
    auto *scrollLayout = new QVBoxLayout(scrollContent);

    // 3. Construct Text via Lua
    auto* gsm = GameStateManager::instance();
    const QString version = getGameVersionInfo();
    
    // Attempt to fetch the text from the Lua global variable "AboutText"
    QString aboutText = gsm->getLuaString("AboutText");

    // Fallback in case Lua variable is missing or script failed to load
    if (aboutText.isEmpty()) {
        aboutText = QString("<h2>DEPTH OF DEJENOL: BLACKLANDS</h2><p><b>Version: %1</b></p>").arg(version);
    } else {
        // Replace %1 placeholder in the Lua string with the actual version
        aboutText = aboutText.arg(version);
    }

    auto *infoText = new QLabel(aboutText);
    infoText->setWordWrap(true);
    infoText->setTextFormat(Qt::RichText);
    infoText->setAlignment(Qt::AlignTop);

    // 4. Assemble Scroll Content
    scrollLayout->addWidget(infoText);

    auto *checkUpdatesBtn = new QPushButton(tr("Check for Updates"), this);
    checkUpdatesBtn->setFixedWidth(180);
    scrollLayout->addWidget(checkUpdatesBtn, 0, Qt::AlignCenter);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);

    // 5. Bottom Button Row
    auto *closeButton = new QPushButton(tr("Ok"));
    closeButton->setDefault(true); // Allow 'Enter' key to close
    auto *buttonLayout = new QHBoxLayout();
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
        dlg.setManifestUrl(QUrl(QStringLiteral("https://raw.githubusercontent.com/Vanderbull/depthofdejenol/main/manifest.json")));
        dlg.exec();
    });
}

AboutDialog::~AboutDialog() = default;
