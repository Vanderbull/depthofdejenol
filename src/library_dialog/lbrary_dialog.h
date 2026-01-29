#ifndef LIBRARY_DIALOG_H
#define LIBRARY_DIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>
// Forward declarations to avoid unnecessary includes in the header
class QListWidget;
class QTextEdit;
class QListWidgetItem;
// A simple struct to hold item or monster data
struct LibraryEntry 
{
    QString type;
    QString details;
};
// The main dialog class for the Library of Knowledge
class LibraryDialog : public QDialog {
    Q_OBJECT
public:
    // Constructor to set up the UI and data
    explicit LibraryDialog(QWidget *parent = nullptr);
private slots:
    // Slot to update the detailsTextEdit based on the selected item
    void updateDetails(QListWidgetItem *item);
private:
    // Private member widgets and data
    QListWidget *listWidget;
    QTextEdit *detailsTextEdit;
    QMap<QString, LibraryEntry> libraryData;
    // A function to populate the library with example data
    void populateLibrary();
};

#endif // LIBRARY_DIALOG_H
