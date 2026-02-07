#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadCsv();
    void saveCsv();
    void updateImage(); // Triggered when a row is selected

private:
    QTableWidget *tableWidget;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QLabel *imageLabel;      // Label to display the image
    QLabel *statusLabel;     // To show info about the current image
    int idColumnIndex = -1;  // Store where the ID column is
};

#endif

