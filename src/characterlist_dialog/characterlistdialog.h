#ifndef CHARACTERLISTDIALOG_H
#define CHARACTERLISTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

#include "GameStateManager.h" // Include the GameStateManager header

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

    // Denna funktion används inte längre externt, men behålls som privat implementation.
    // void addCharacter(const QString &characterName); 

private slots:
    /**
     * @brief Slot called when the "Select" button is clicked.
     */
    void onSelectClicked();

    /**
     * @brief Slot called when the "Delete" button is clicked.
     */
    void onDeleteClicked();

private:
    /**
     * @brief Dynamically loads character names from .txt files in the "characters" folder.
     */
    void loadCharactersFromFiles();

    // UI elements
    QLabel *titleLabel;
    QListWidget *characterListWidget;
    QPushButton *selectButton;
    QPushButton *deleteButton;
    QPushButton *closeButton;
    QVBoxLayout *mainLayout;
};

#endif // CHARACTERLISTDIALOG_H
