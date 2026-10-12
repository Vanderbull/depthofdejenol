#ifndef CHARACTERLISTDIALOG_H
#define CHARACTERLISTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

#include "gameStateManager.h"
#include <QObject>
/**
 * @brief The dialog window for displaying and managing the list of characters.
 */
class CharacterListDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for CharacterListDialog.
     * @param parent The parent widget.
     */
    explicit CharacterListDialog(QWidget *parent = nullptr);
    ~CharacterListDialog();
    // void addCharacter(const QString &characterName); 
private slots:
    /**
     * @brief Slot called when the "Select" button is clicked.
     */
    void onSelectClicked();
    void onSavePartyClicked();
    void onLoadPartyClicked();
    /**
     * @brief Slot called when the "Delete" button is clicked.
     */
    void onDeleteClicked();
    void onClearSelectionClicked();
    void updateSelectionPreview();
private:
    /**
     * @brief Dynamically loads character names from .txt files in the "characters" folder.
     */
    void loadCharactersFromFiles();
    // UI elements
    QLabel *titleLabel;
public:
    QListWidget *characterListWidget;
private:
    QPushButton *selectButton;
    QPushButton *deleteButton;
    QPushButton *closeButton;
    QPushButton *savePartyButton;
    QPushButton *loadPartyButton;
    QPushButton *clearSelectionButton;
    QLabel *selectionPreviewLabel;
    QLabel *partySizeLabel;
    QVBoxLayout *mainLayout;
};

#endif // CHARACTERLISTDIALOG_H
