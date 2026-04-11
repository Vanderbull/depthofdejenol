#ifndef LIBRARY_DIALOG_H
#define LIBRARY_DIALOG_H

#include <QDialog>
#include <QMap>
#include <QListWidget>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>

class LibraryDialog : public QDialog {
    Q_OBJECT
public:
    explicit LibraryDialog(QWidget *parent = nullptr);

private:
    void setupUI();
    void loadKnowledge();
    void refreshDisplay(); // Centralized update logic

    QMap<QString, QMap<QString, QString>> knowledgeBase;
    QListWidget *bookList;
    QTextEdit *descriptionText;
    QComboBox *categoryBox;
    QLineEdit *searchField;
    QLabel *countLabel;
};

#endif
