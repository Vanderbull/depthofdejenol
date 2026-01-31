#ifndef LIBRARY_DIALOG_H
#define LIBRARY_DIALOG_H

#include "GameConstants.h"
#include <QDialog>
#include <QMap>
#include <QString>
#include <QListWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include "GameStateManager.h"
// Define Category Constants
//const QString CATEGORY_MAGIC = "Magic Books";
//const QString CATEGORY_MONSTERS = "Creatures";
//const QString CATEGORY_ITEMS = "Items";

class LibraryDialog : public QDialog 
{
    Q_OBJECT
public:
    explicit LibraryDialog(QWidget *parent = nullptr);
private slots:
    void onItemSelected(QListWidgetItem *item);
    void onCloseClicked();
    void onCategoryChanged(const QString &categoryName);
    void onSearchTextChanged(const QString &searchText);
    // Missing slot declarations added here
    void onAddItemClicked(); 
private:
    void setupUI();
    void loadKnowledge();
    void updateList(const QString &category);
    // Missing function declaration added here
    void updateCountLabel();
private:
    // QMap<CategoryName, QMap<ItemName, ItemDescription>>
    QMap<QString, QMap<QString, QString>> knowledgeBase;
    // UI elements (Missing members added here)
    QListWidget *bookList;
    QTextEdit *descriptionText;
    QComboBox *categoryComboBox;
    QLabel *headerLabel;
    // Missing member variables added here
    QPushButton *addItemButton; 
    QPushButton *closeButton;
    QLabel *countLabel;
    QLineEdit *searchField;
};

#endif // LIBRARY_DIALOG_H
