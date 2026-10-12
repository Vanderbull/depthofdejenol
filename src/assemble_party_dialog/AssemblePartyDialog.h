#ifndef ASSEMBLEPARTYDIALOG_H
#define ASSEMBLEPARTYDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>

#include "character.h"

/**
 * @brief Assemble or modify the party from the current game state.
 *
 * Left side:  the current party members (with remove buttons).
 * Right side: available characters on disk (with add buttons).
 * Bottom:    party preview + Save / Cancel.
 *
 * On Save the dialog calls gameStateManager::packStateForSaving() +
 * saveFullGameState("autosave") so the modified party is persisted.
 */
class AssemblePartyDialog : public QDialog {
    Q_OBJECT

public:
    explicit AssemblePartyDialog(QWidget *parent = nullptr);
    ~AssemblePartyDialog();

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onSaveClicked();
    void onCancelClicked();
    void refreshLists();

private:
    void loadAvailableCharacters();
    void updatePreview();

    QListWidget *m_partyList = nullptr;
    QListWidget *m_availableList = nullptr;
    QLabel *m_previewLabel = nullptr;
    QPushButton *m_addButton = nullptr;
    QPushButton *m_removeButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
};

#endif // ASSEMBLEPARTYDIALOG_H
