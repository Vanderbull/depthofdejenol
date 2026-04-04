#ifndef CHARACTERDIALOG_H
#define CHARACTERDIALOG_H

#include <QDialog>
#include <QString>
#include <QVariant>
#include <QLabel>
#include "gameStateManager.h"

class CharacterDialog : public QDialog {
    Q_OBJECT

public:
    // Constructor signature now takes only the parent widget
    explicit CharacterDialog(QWidget *parent = nullptr);
    ~CharacterDialog() override = default;

private slots:
    // Slot to receive state change notifications from gameStateManager
    void updateGameStateValue(const QString& key, const QVariant& value);

private:
    // Pointer to the label displaying current gold, for dynamic updates
    QLabel *m_goldValueLabel = nullptr;
    void setupUi();
};

#endif // CHARACTERDIALOG_H
