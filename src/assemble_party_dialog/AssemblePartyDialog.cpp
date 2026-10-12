#include "AssemblePartyDialog.h"
#include "gameStateManager.h"
#include "src/core/SoundEffects.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QInputDialog>

AssemblePartyDialog::AssemblePartyDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Assemble Party"));
    setMinimumSize(700, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Title
    QLabel *title = new QLabel(tr("Assemble Your Party"));
    QFont titleFont = title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    // Two-column layout: current party | available characters
    QHBoxLayout *columns = new QHBoxLayout();

    // Left: current party
    QVBoxLayout *partyBox = new QVBoxLayout();
    QLabel *partyTitle = new QLabel(tr("Current Party"));
    partyBox->addWidget(partyTitle);
    m_partyList = new QListWidget(this);
    partyBox->addWidget(m_partyList);
    columns->addLayout(partyBox, 1);

    // Right: available characters
    QVBoxLayout *availBox = new QVBoxLayout();
    QLabel *availTitle = new QLabel(tr("Available Characters"));
    availBox->addWidget(availTitle);
    m_availableList = new QListWidget(this);
    availBox->addWidget(m_availableList);
    columns->addLayout(availBox, 1);

    mainLayout->addLayout(columns);

    // Action buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_addButton = new QPushButton(tr("Add →"), this);
    m_removeButton = new QPushButton(tr("← Remove"), this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_addButton);
    btnLayout->addWidget(m_removeButton);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // Preview
    m_previewLabel = new QLabel(tr("No party members"));
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    m_previewLabel->setMinimumHeight(50);
    mainLayout->addWidget(m_previewLabel);

    // Bottom buttons
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    m_saveButton = new QPushButton(tr("Save Party"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_saveButton);
    bottomLayout->addWidget(m_cancelButton);
    mainLayout->addLayout(bottomLayout);

    connect(m_addButton, &QPushButton::clicked, this, &AssemblePartyDialog::onAddClicked);
    connect(m_removeButton, &QPushButton::clicked, this, &AssemblePartyDialog::onRemoveClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &AssemblePartyDialog::onSaveClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &AssemblePartyDialog::onCancelClicked);

    refreshLists();
}

AssemblePartyDialog::~AssemblePartyDialog() = default;

void AssemblePartyDialog::refreshLists()
{
    m_partyList->clear();
    m_availableList->clear();

    QList<Character> &party = gameStateManager::instance()->getPartyMembers();

    // Current party
    if (party.isEmpty()) {
        QListWidgetItem *empty = new QListWidgetItem(tr("(empty)"));
        empty->setFlags(Qt::NoItemFlags);
        m_partyList->addItem(empty);
    } else {
        for (const Character &c : party) {
            QString label = QString("%1 — %2 (Lv %3)")
                .arg(c.name, c.race)
                .arg(c.level);
            if (!c.isAlive) label += tr(" [DEAD]");
            m_partyList->addItem(label);
        }
    }

    // Available characters on disk
    loadAvailableCharacters();
    updatePreview();
}

void AssemblePartyDialog::loadAvailableCharacters()
{
    QDir dir("data/characters/");
    if (!dir.exists()) return;

    QStringList filters;
    filters << "*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);

    for (const QFileInfo &fi : files) {
        QFile f(fi.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly)) continue;
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        if (doc.isNull() || !doc.isObject()) continue;
        QJsonObject obj = doc.object();

        QString name = obj.value("Name").toString();
        QString race = obj.value("Race").toString();
        int level = obj.value("Level").toInt(1);
        bool alive = obj.value("isAlive").toBool(true);

        if (name.isEmpty()) continue;

        QString label = QString("%1 — %2 (Lv %3)").arg(name, race).arg(level);
        if (!alive) label += tr(" [DEAD]");

        QListWidgetItem *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, fi.absoluteFilePath());
        m_availableList->addItem(item);
    }
}

void AssemblePartyDialog::updatePreview()
{
    QList<Character> &party = gameStateManager::instance()->getPartyMembers();

    if (party.isEmpty()) {
        m_previewLabel->setText(tr("No party members"));
        return;
    }

    QStringList lines;
    int totalLevel = 0;
    int aliveCount = 0;
    for (const Character &c : party) {
        lines << c.name;
        totalLevel += c.level;
        if (c.isAlive) aliveCount++;
    }

    m_previewLabel->setText(tr("Party: %1\nMembers: %2 | Alive: %3 | Avg Level: %4")
        .arg(lines.join(", "))
        .arg(party.size())
        .arg(aliveCount)
        .arg(totalLevel / party.size()));
}

void AssemblePartyDialog::onAddClicked()
{
    QListWidgetItem *item = m_availableList->currentItem();
    if (!item) return;

    QString path = item->data(Qt::UserRole).toString();
    if (path.isEmpty()) return;

    // Read the character from disk
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot read character file."));
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (doc.isNull() || !doc.isObject()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid character file."));
        return;
    }
    QJsonObject obj = doc.object();

    Character c;
    c.loadFromMap(obj.toVariantMap());

    // Check if already in party
    for (const Character &existing : gameStateManager::instance()->getPartyMembers()) {
        if (existing.name == c.name) {
            QMessageBox::information(this, tr("Already in Party"),
                tr("%1 is already in the party.").arg(c.name));
            return;
        }
    }

    gameStateManager::instance()->addCharacterToParty(c);
    SoundEffects::instance()->play(SoundEffects::Type::LevelUp);
    refreshLists();
}

void AssemblePartyDialog::onRemoveClicked()
{
    int row = m_partyList->currentRow();
    if (row < 0) return;

    QList<Character> &party = gameStateManager::instance()->getPartyMembers();
    if (row >= party.size()) return;

    QString name = party[row].name;
    party.removeAt(row);
    refreshLists();
}

void AssemblePartyDialog::onSaveClicked()
{
    QList<Character> &party = gameStateManager::instance()->getPartyMembers();

    if (party.isEmpty()) {
        QMessageBox::warning(this, tr("Empty Party"),
            tr("You need at least one party member to save."));
        return;
    }

    bool hasAlive = false;
    for (const Character &c : party) {
        if (c.isAlive) { hasAlive = true; break; }
    }

    if (!hasAlive) {
        QMessageBox::warning(this, tr("All Dead"),
            tr("Your party has no living members. Add at least one living character."));
        return;
    }

    // Save the full game state with the modified party
    if (gameStateManager::instance()->saveFullGameState("autosave")) {
        QMessageBox::information(this, tr("Party Saved"),
            tr("Party saved successfully."));
        accept();
    } else {
        QMessageBox::critical(this, tr("Save Error"),
            tr("Failed to save the party."));
    }
}

void AssemblePartyDialog::onCancelClicked()
{
    reject();
}
