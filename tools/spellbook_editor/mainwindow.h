#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadCsv();
    void saveCsv(); // New slot for saving

private:
    QTableWidget *tableWidget;
    QPushButton *loadButton;
    QPushButton *saveButton; // New button
};

#endif
