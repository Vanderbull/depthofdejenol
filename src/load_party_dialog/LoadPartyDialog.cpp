#include "LoadPartyDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QHeaderView>

LoadPartyDialog::LoadPartyDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Load Party"));
    setMinimumSize(500, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Title
    QLabel *titleLabel = new QLabel(tr("Select a Party to Load"));
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Party list
    m_partyList = new QListWidget(this);
    m_partyList->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(m_partyList);

    // Preview
    m_previewLabel = new QLabel(tr("Select a party to preview"));
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_previewLabel->setMinimumHeight(60);
    mainLayout->addWidget(m_previewLabel);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_loadButton = new QPushButton(tr("Load"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_loadButton);
    buttonLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(m_partyList, &QListWidget::itemSelectionChanged, this, &LoadPartyDialog::onPartySelected);
    connect(m_loadButton, &QPushButton::clicked, this, &LoadPartyDialog::onLoadClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &LoadPartyDialog::onCancelClicked);

    loadPartyList();
}

LoadPartyDialog::~LoadPartyDialog() = default;

void LoadPartyDialog::loadPartyList()
{
    m_partyList->clear();
    m_selectedPath.clear();

    // List saved game states from data/saves/ only.
    // Individual character files in data/characters/ are not game states —
    // they are managed by Assemble Party / Create Character.
    QStringList filters;
    filters << "*.json";
    QDir savesDir("data/saves");

    if (savesDir.exists()) {
        QFileInfoList saves = savesDir.entryInfoList(filters, QDir::Files, QDir::Time);
        for (const QFileInfo &fi : saves) {
            QListWidgetItem *item = new QListWidgetItem(fi.completeBaseName());
            item->setData(Qt::UserRole, fi.absoluteFilePath());
            m_partyList->addItem(item);
        }
    }
}

void LoadPartyDialog::onPartySelected()
{
    QListWidgetItem *item = m_partyList->currentItem();
    if (!item) {
        m_previewLabel->setText(tr("Select a party to preview"));
        return;
    }

    m_selectedPath = item->data(Qt::UserRole).toString();

    // Try to read party info from the file
    QFile file(m_selectedPath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_previewLabel->setText(tr("Cannot read file"));
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || doc.isEmpty()) {
        m_previewLabel->setText(tr("Invalid save file"));
        return;
    }

    QJsonObject root = doc.object();

    // Extract party info
    QStringList memberNames;
    int totalLevel = 0;
    int memberCount = 0;

    if (root.contains("Party") && root["Party"].isObject()) {
        QJsonObject party = root["Party"].toObject();
        QJsonArray members = party.value("Members").toArray();
        for (const QJsonValue &v : members) {
            QJsonObject member = v.toObject();
            QString name = member.value("Name").toString();
            int level = member.value("Level").toInt(1);
            bool alive = member.value("isAlive").toBool(true);
            if (name != "Empty Slot" && !name.isEmpty()) {
                memberNames.append(name + (alive ? "" : " (deceased)"));
                totalLevel += level;
                memberCount++;
            }
        }
    }

    if (memberCount == 0) {
        m_previewLabel->setText(tr("No party members found"));
        return;
    }

    QString preview = tr("Party: %1\nMembers: %2\nAvg Level: %3")
        .arg(memberNames.join(", "))
        .arg(memberCount)
        .arg(totalLevel / memberCount);

    m_previewLabel->setText(preview);
}

void LoadPartyDialog::onLoadClicked()
{
    if (m_selectedPath.isEmpty()) {
        return;
    }
    accept();
}

void LoadPartyDialog::onCancelClicked()
{
    reject();
}

QString LoadPartyDialog::selectedPartyPath() const
{
    return m_selectedPath;
}
