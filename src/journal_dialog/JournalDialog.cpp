#include "JournalDialog.h"
#include "gameStateManager.h"
#include <QtWidgets>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>

JournalDialog::JournalDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    loadEntries();
    refreshList();
}

JournalDialog::~JournalDialog() = default;

void JournalDialog::setupUi() {
    setWindowTitle(tr("Journal"));
    setMinimumSize(500, 400);

    auto *mainLayout = new QVBoxLayout(this);

    // Filter
    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(tr("Category:")));
    m_filterCombo = new QComboBox;
    m_filterCombo->addItem(tr("All"));
    m_filterCombo->addItem(tr("Quest"));
    m_filterCombo->addItem(tr("Combat"));
    m_filterCombo->addItem(tr("Exploration"));
    m_filterCombo->addItem(tr("Notes"));
    filterLayout->addWidget(m_filterCombo);
    mainLayout->addLayout(filterLayout);

    // Entry list
    m_entryList = new QListWidget;
    m_entryList->setMaximumWidth(200);
    mainLayout->addWidget(m_entryList);

    // Entry text
    m_entryText = new QTextEdit;
    m_entryText->setReadOnly(true);
    mainLayout->addWidget(m_entryText);

    // Buttons
    auto *btnLayout = new QHBoxLayout;
    m_addNoteBtn = new QPushButton(tr("Add Note"));
    m_clearBtn = new QPushButton(tr("Clear All"));
    m_exitBtn = new QPushButton(tr("Close"));
    btnLayout->addWidget(m_addNoteBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_exitBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_entryList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *previous) {
                Q_UNUSED(previous);
                onEntrySelected(current);
            });
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &JournalDialog::onFilterChanged);
    connect(m_addNoteBtn, &QPushButton::clicked, this, &JournalDialog::onAddNoteClicked);
    connect(m_clearBtn, &QPushButton::clicked, this, &JournalDialog::onClearClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &JournalDialog::onExitClicked);
}

void JournalDialog::loadEntries() {
    m_entries.clear();

    QString path = "data/journal.json";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) return;

    QJsonArray arr = doc.array();
    for (const auto& val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        JournalEntry e;
        e.timestamp = QDateTime::fromString(obj.value("timestamp").toString(), Qt::ISODate);
        e.category = obj.value("category").toString();
        e.text = obj.value("text").toString();
        m_entries.append(e);
    }
}

void JournalDialog::refreshList() {
    m_entryList->clear();

    QString filter = m_filterCombo->currentText();
    if (filter == tr("All")) filter = QString();

    for (int i = 0; i < m_entries.size(); ++i) {
        const JournalEntry& e = m_entries[i];
        if (!filter.isEmpty() && e.category != filter) continue;

        QString label = tr("[%1] %2")
            .arg(e.timestamp.toString("yyyy-MM-dd"))
            .arg(e.text.left(40));
        QListWidgetItem *item = new QListWidgetItem(label, m_entryList);
        item->setData(Qt::UserRole, i);
    }

    if (m_entryList->count() > 0) {
        m_entryList->setCurrentRow(0);
    }
}

void JournalDialog::onEntrySelected(QListWidgetItem *item) {
    if (!item) return;
    int idx = item->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= m_entries.size()) return;

    const JournalEntry& e = m_entries[idx];
    m_entryText->setHtml(tr("<b>%1</b> — %2<br><br>%3")
        .arg(e.timestamp.toString("yyyy-MM-dd hh:mm"))
        .arg(e.category)
        .arg(e.text));
}

void JournalDialog::onFilterChanged(int index) {
    Q_UNUSED(index)
    refreshList();
}

void JournalDialog::onAddNoteClicked() {
    bool ok;
    QString text = QInputDialog::getMultiLineText(this, tr("Add Note"),
        tr("Enter your note:"), QString(), &ok);
    if (!ok || text.isEmpty()) return;

    addEntry(tr("Notes"), text);
    loadEntries();
    refreshList();
}

void JournalDialog::onClearClicked() {
    if (QMessageBox::Yes != QMessageBox::question(this, tr("Clear Journal"),
        tr("Delete all journal entries?"))) return;

    m_entries.clear();
    QFile::remove("data/journal.json");
    refreshList();
    m_entryText->clear();
}

void JournalDialog::onExitClicked() {
    accept();
}

void JournalDialog::addEntry(const QString& category, const QString& text) {
    JournalEntry e;
    e.timestamp = QDateTime::currentDateTime();
    e.category = category;
    e.text = text;

    // Load existing
    QList<JournalEntry> entries;
    QFile file("data/journal.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (doc.isArray()) {
            for (const auto& val : doc.array()) {
                if (!val.isObject()) continue;
                QJsonObject obj = val.toObject();
                JournalEntry existing;
                existing.timestamp = QDateTime::fromString(obj.value("timestamp").toString(), Qt::ISODate);
                existing.category = obj.value("category").toString();
                existing.text = obj.value("text").toString();
                entries.append(existing);
            }
        }
    }

    entries.append(e);

    // Save
    QJsonArray arr;
    for (const auto& entry : entries) {
        QJsonObject obj;
        obj["timestamp"] = entry.timestamp.toString(Qt::ISODate);
        obj["category"] = entry.category;
        obj["text"] = entry.text;
        arr.append(obj);
    }

    QDir().mkpath("data");
    QFile out("data/journal.json");
    if (out.open(QIODevice::WriteOnly)) {
        out.write(QJsonDocument(arr).toJson());
        out.close();
    }
}

QList<JournalEntry> JournalDialog::allEntries() {
    QList<JournalEntry> entries;

    QFile file("data/journal.json");
    if (!file.open(QIODevice::ReadOnly)) return entries;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isArray()) return entries;

    for (const auto& val : doc.array()) {
        if (!val.isObject()) continue;
        const QJsonObject obj = val.toObject();
        JournalEntry e;
        e.timestamp = QDateTime::fromString(obj.value("timestamp").toString(), Qt::ISODate);
        e.category = obj.value("category").toString();
        e.text = obj.value("text").toString();
        entries.append(e);
    }
    return entries;
}

void JournalDialog::clearAll() {
    QFile::remove("data/journal.json");
}
