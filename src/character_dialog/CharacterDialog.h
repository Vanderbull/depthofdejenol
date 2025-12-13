// CharacterDialog.h
#ifndef CHARACTERDIALOG_H
#define CHARACTERDIALOG_H

#include <QDialog>
#include <QString>
#include <QVariant> // Added for the slot's QVariant argument
#include <QLabel> // Added for private member m_goldValueLabel
#include "GameStateManager.h"

class CharacterDialog : public QDialog {
    Q_OBJECT

public:
    // CORRECTED: Constructor signature now takes only the parent widget
    explicit CharacterDialog(QWidget *parent = nullptr);
    ~CharacterDialog() override = default;

private slots:
    // NEW: Slot to receive state change notifications from GameStateManager
    void updateGameStateValue(const QString& key, const QVariant& value);

private:
    // NEW: Pointer to the label displaying current gold, for dynamic updates
    QLabel *m_goldValueLabel = nullptr;

    void setupUi();
};

#endif // CHARACTERDIALOG_H
