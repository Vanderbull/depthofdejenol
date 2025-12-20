#ifndef CSVVIEWER_H
#define CSVVIEWER_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QTextBrowser>
#include <QStringList>

class CSVViewer : public QMainWindow {
    Q_OBJECT

public:
    CSVViewer(QWidget *parent = nullptr);

private slots:
    void loadCSV();
    void updateDetails();
    void filterTable(const QString &text);
    void exportToJSON(); // New slot for JSON export

private:
    QTableWidget *table;
    QLineEdit *searchBar;
    QTextBrowser *detailsDisplay;
    QStringList headers;
};

#endif
