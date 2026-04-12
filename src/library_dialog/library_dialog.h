#ifndef LIBRARY_DIALOG_H
#define LIBRARY_DIALOG_H

#include "src/core/GameConstants.h"
#include <QDialog>
#include <QMap>
#include <QString>
#include <QListWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include "gameStateManager.h"

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
    void onAddItemClicked();
private:
    void setupUI();
    void loadKnowledge();
    void updateList(const QString &category);
    void updateCountLabel();
    QMap<QString, QMap<QString, QString>> knowledgeBase;
    QListWidget *bookList;
    QTextEdit *descriptionText;
    QComboBox *categoryComboBox;
    QLabel *headerLabel;
    QPushButton *addItemButton;
    QPushButton *closeButton;
    QLabel *countLabel;
    QLineEdit *searchField;
};

#endif // LIBRARY_DIALOG_H
