#include "SkillBookDialog.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLabel>
#include <QGroupBox>
#include <QFont>

SkillBookDialog::SkillBookDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Skill Book"));
    setMinimumWidth(700);
    setMinimumHeight(500);

    auto *mainLayout = new QVBoxLayout(this);

    auto *skillGroup = new QGroupBox(tr("Skill Book"));
    auto *skillLayout = new QVBoxLayout(skillGroup);

    m_skillTable = new QTableWidget(0, 5);
    m_skillTable->setHorizontalHeaderLabels({tr("Skill"), tr("Level"), tr("Prerequisite"), tr("Cost"), tr("Description")});
    m_skillTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_skillTable->setSelectionMode(QAbstractItemView::NoSelection);
    skillLayout->addWidget(m_skillTable);

    mainLayout->addWidget(skillGroup);
}

SkillBookDialog::~SkillBookDialog()
{
    delete m_skillTable;
}

void SkillBookDialog::refresh(const QVector<SkillDef>& defs, const QMap<QString, int>& levels)
{
    m_skillTable->setRowCount(defs.size());
    for (int i = 0; i < defs.size(); ++i) {
        const SkillDef& def = defs[i];

        // Skill name
        QTableWidgetItem* nameItem = new QTableWidgetItem(def.name);
        if (def.type == QStringLiteral("Combat")) {
            nameItem->setBackground(Qt::gray);
        } else if (def.type == QStringLiteral("Craft")) {
            nameItem->setBackground(Qt::yellow);
        } else if (def.type == QStringLiteral("Social")) {
            nameItem->setBackground(Qt::green);
        } else if (def.type == QStringLiteral("Survival")) {
            nameItem->setBackground(Qt::blue);
        }
        m_skillTable->setItem(i, 0, nameItem);

        // Level
        QTableWidgetItem* levelItem = new QTableWidgetItem(QString::number(levels.value(def.name, 0)));
        m_skillTable->setItem(i, 1, levelItem);

        // Prerequisite
        QTableWidgetItem* prereqItem = new QTableWidgetItem(def.prerequisite.isEmpty() ? QStringLiteral("None") : def.prerequisite);
        m_skillTable->setItem(i, 2, prereqItem);

        // Cost
        QTableWidgetItem* costItem = new QTableWidgetItem(QString::number(def.cost));
        m_skillTable->setItem(i, 3, costItem);

        // Description
        QTableWidgetItem* descItem = new QTableWidgetItem(def.description);
        m_skillTable->setItem(i, 4, descItem);
    }

    // Set column widths
    m_skillTable->setColumnWidth(0, 150);
    m_skillTable->setColumnWidth(1, 60);
    m_skillTable->setColumnWidth(2, 120);
    m_skillTable->setColumnWidth(3, 60);
    m_skillTable->setColumnWidth(4, 400);
}
