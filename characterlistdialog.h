#ifndef CHARACTERLISTDIALOG_H
#define CHARACTERLISTDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

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

    /**
     * @brief Adds a character to the list.
     * @param characterName The name of the character.
     */
    void addCharacter(const QString &characterName);

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
    // UI elements
    QLabel *titleLabel;
    QListWidget *characterListWidget;
    QPushButton *selectButton;
    QPushButton *deleteButton;
    QPushButton *closeButton;
    QVBoxLayout *mainLayout;
};

#endif // CHARACTERLISTDIALOG_H
