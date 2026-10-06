#include "ShortcutHelp.h"
#include <QtWidgets>
#include <QHeaderView>

ShortcutHelp::ShortcutHelp(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Keyboard Shortcuts"));
    setMinimumSize(500, 400);

    auto *mainLayout = new QVBoxLayout(this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(tr("Context:")));
    auto *filterCombo = new QComboBox;
    filterCombo->addItem(tr("All"));
    for (const QString& ctx : contexts()) {
        filterCombo->addItem(ctx);
    }
    filterLayout->addWidget(filterCombo);
    mainLayout->addLayout(filterLayout);

    auto *table = new QTableWidget(0, 3);
    table->setHorizontalHeaderLabels({tr("Key"), tr("Action"), tr("Context")});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(table);

    auto *closeBtn = new QPushButton(tr("Close"));
    mainLayout->addWidget(closeBtn, 0, Qt::AlignRight);

    auto refresh = [table, filterCombo]() {
        QString filter = filterCombo->currentText();
        if (filter == tr("All")) filter = QString();

        QList<ShortcutEntry> entries = allShortcuts();
        table->setRowCount(0);
        for (const ShortcutEntry& e : entries) {
            if (!filter.isEmpty() && e.context != filter) continue;
            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(e.key));
            table->setItem(row, 1, new QTableWidgetItem(e.action));
            table->setItem(row, 2, new QTableWidgetItem(e.context));
        }
    };

    connect(filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [refresh](int) { refresh(); });
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    refresh();
}

ShortcutHelp::~ShortcutHelp() = default;

QList<ShortcutEntry> ShortcutHelp::allShortcuts() {
    QList<ShortcutEntry> list;

    auto add = [&list](const QString& key, const QString& action, const QString& ctx) {
        ShortcutEntry e;
        e.key = key;
        e.action = action;
        e.context = ctx;
        list.append(e);
    };

    // --- Dungeon movement ---
    add("W / Up",    "Move forward",       "Dungeon");
    add("S / Down",  "Move backward",      "Dungeon");
    add("A / Left",  "Step left",          "Dungeon");
    add("D / Right", "Step right",         "Dungeon");

    // --- Dungeon rotation ---
    add("Q",         "Rotate left",        "Dungeon");
    add("E",         "Rotate right",       "Dungeon");

    // --- Dungeon actions ---
    add("T",         "Use stairs",          "Dungeon");
    add("F",         "Fight",              "Dungeon");
    add("S",         "Cast spell",         "Dungeon");
    add("R",         "Rest",               "Dungeon");
    add("O",         "Open door/chest",    "Dungeon");
    add("I",         "Inventory",          "Dungeon");
    add("C",         "Character sheet",    "Dungeon");
    add("1",         "Character dialog",   "Dungeon");
    add("U",         "Teleport",           "Dungeon");
    add("M",         "Map",                "Dungeon");
    add("Z",         "Search",             "Dungeon");
    add("P",         "Pick up item",       "Dungeon");
    add("D",         "Defend / Drop",      "Dungeon");

    // --- Global ---
    add("F1",        "Help",               "Global");
    add("F5",        "Quick save",         "Global");
    add("F9",        "Quick load",         "Global");
    add("Esc",       "Close dialog",       "Global");

    return list;
}

QList<ShortcutEntry> ShortcutHelp::shortcutsFor(const QString& context) {
    QList<ShortcutEntry> result;
    for (const ShortcutEntry& e : allShortcuts()) {
        if (e.context == context) result.append(e);
    }
    return result;
}

QStringList ShortcutHelp::contexts() {
    return {"Dungeon", "Global"};
}

ShortcutEntry ShortcutHelp::find(const QString& key, const QString& context) {
    for (const ShortcutEntry& e : allShortcuts()) {
        if (e.key == key && e.context == context) return e;
    }
    ShortcutEntry invalid;
    invalid.key = QString();
    return invalid;
}

QString ShortcutHelp::format(const ShortcutEntry& e) {
    return QString("%1 — %2").arg(e.key, e.action);
}
