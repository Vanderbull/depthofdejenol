#include "mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QHeaderView>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    loadButton = new QPushButton("Load MDATA2.csv", this);
    saveButton = new QPushButton("Save Changes", this);
    tableWidget = new QTableWidget(this);
    
    // Allow editing by double-clicking (enabled by default in QTableWidget)
    tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    layout->addWidget(loadButton);
    layout->addWidget(tableWidget);
    layout->addWidget(saveButton); // Add save button to UI
    
    setCentralWidget(centralWidget);
    setWindowTitle("CSV Editor");
    resize(1000, 600);

    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadCsv);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveCsv);
}

MainWindow::~MainWindow() {}

void MainWindow::loadCsv() {
    QString fileName = "MDATA2.csv";
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for reading.");
        return;
    }

    QTextStream in(&file);
    bool isHeader = true;
    tableWidget->setRowCount(0);

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");

        if (isHeader) {
            tableWidget->setColumnCount(fields.size());
            QStringList headers;
            for (const QString &field : fields) headers << field.trimmed();
            tableWidget->setHorizontalHeaderLabels(headers);
            isHeader = false;
        } else {
            int row = tableWidget->rowCount();
            tableWidget->insertRow(row);
            for (int i = 0; i < fields.size(); ++i) {
                tableWidget->setItem(row, i, new QTableWidgetItem(fields[i].trimmed()));
            }
        }
    }
    file.close();
}

void MainWindow::saveCsv() {
    QString fileName = "MDATA2.csv";
    QFile file(fileName);

    // Open in WriteOnly mode which truncates the file (clears it)
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);
    int rowCount = tableWidget->rowCount();
    int colCount = tableWidget->columnCount();

    // 1. Write the Headers
    QStringList headers;
    for (int i = 0; i < colCount; ++i) {
        headers << tableWidget->horizontalHeaderItem(i)->text();
    }
    out << headers.join(",") << "\n";

    // 2. Write the Data Rows
    for (int i = 0; i < rowCount; ++i) {
        QStringList rowData;
        for (int j = 0; j < colCount; ++j) {
            QTableWidgetItem *item = tableWidget->item(i, j);
            if (item) {
                rowData << item->text();
            } else {
                rowData << ""; // Handle empty cells
            }
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, "Success", "Changes saved to MDATA2.csv");
}
