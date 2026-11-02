#pragma once

#include <QDialog>
#include <QVector>
#include <QString>
#include "../skill/SkillDatabase.h"

namespace Ui {
class SkillBookDialog;
}

class SkillBookDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SkillBookDialog(QWidget* parent = nullptr);
    ~SkillBookDialog();

    // Refresh the skill list (update levels from character)
    void refresh(const QVector<SkillDef>& defs, const QMap<QString, int>& levels);

private:
    QTableWidget* m_skillTable;
};

