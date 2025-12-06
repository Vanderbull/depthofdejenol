#ifndef LIBRARY_DIALOG_H
#define LIBRARY_DIALOG_H

#include <QDialog>
#include <QStringList>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QMap>
#include <QComboBox> // Added for category selection
#include <QStackedWidget> // Added to potentially switch list views (for future proofing)

#include "GameStateManager.h" // Include the GameStateManager header

/**
 * @brief The LibraryDialog class simulates the 'Library of Knowledge' from Mordor.
 *
 * It allows the user to browse different categories of information: 
 * Magic Books, Creatures of the Dark (Monsters), and Items.
 */
class LibraryDialog : public QDialog {
    Q_OBJECT

public:
    explicit LibraryDialog(QWidget *parent = nullptr);
    ~LibraryDialog() override = default;

private slots:
    /**
     * @brief Slot called when a list item is selected.
     */
    void onItemSelected(QListWidgetItem *item);

    /**
     * @brief Slot called when the 'Close' button is clicked.
     */
    void onCloseClicked();

    /**
     * @brief Slot called when the category combo box value changes.
     */
    void onCategoryChanged(const QString &categoryName);

private:
    // --- UI Components ---
    QLabel *headerLabel;
    QComboBox *categoryComboBox; // New component for switching categories
    QListWidget *bookList; // Renamed, but will now hold different categories
    QTextEdit *descriptionText;
    QPushButton *closeButton;

    // --- Data and Logic ---
    // A map to hold all possible data sets, keyed by category name.
    // The inner map stores the item name and its description/details.
    QMap<QString, QMap<QString, QString>> knowledgeBase;
    
    // Identifiers for the categories
    const QString CATEGORY_MAGIC = "Magic Books";
    const QString CATEGORY_MONSTERS = "Creatures";
    const QString CATEGORY_ITEMS = "Items";

    void setupUI();
    void loadKnowledge(); // Updated function name
    void updateList(const QString &category);
};

#endif // LIBRARY_DIALOG_H
