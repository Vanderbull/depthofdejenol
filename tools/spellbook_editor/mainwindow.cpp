#include "mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QHeaderView>
#include <QMessageBox>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *contentLayout = new QHBoxLayout(); // Split table and image

    // Left Side: Table
    tableWidget = new QTableWidget(this);
    tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Right Side: Image Preview Panel
    QWidget *previewPanel = new QWidget();
    QVBoxLayout *previewLayout = new QVBoxLayout(previewPanel);
    
    imageLabel = new QLabel("No Image", this);
    imageLabel->setFixedSize(256, 256); // Set a standard size for icons
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setFrameStyle(QFrame::StyledPanel);
    
    statusLabel = new QLabel("Select an item", this);
    statusLabel->setAlignment(Qt::AlignCenter);

    previewLayout->addWidget(new QLabel("<b>Item Preview:</b>"));
    previewLayout->addWidget(imageLabel);
    previewLayout->addWidget(statusLabel);
    previewLayout->addStretch();

    contentLayout->addWidget(tableWidget, 1);
    contentLayout->addWidget(previewPanel, 0);

    // Buttons
    loadButton = new QPushButton("Load MDATA2.csv", this);
    saveButton = new QPushButton("Save Changes", this);

    mainLayout->addWidget(loadButton);
    mainLayout->addLayout(contentLayout);
    mainLayout->addWidget(saveButton);

    setCentralWidget(centralWidget);
    setWindowTitle("CSV Editor with Image Preview");
    resize(1200, 700);

    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadCsv);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveCsv);
    
    // Connect selection change to update the image
    connect(tableWidget, &QTableWidget::itemSelectionChanged, this, &MainWindow::updateImage);
}

MainWindow::~MainWindow() {}

void MainWindow::loadCsv() {
    QString fileName = "MDATA2.csv";
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    bool isHeader = true;
    tableWidget->setRowCount(0);

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");

        if (isHeader) {
            tableWidget->setColumnCount(fields.size());
            QStringList headers;
            for (int i = 0; i < fields.size(); ++i) {
                QString h = fields[i].trimmed();
                headers << h;
                // Identify the ID column (the CSV has " ID" with a space)
                if (h.toLower() == "id" || h.toLower() == "picid") {
                    idColumnIndex = i;
                }
            }
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
    tableWidget->resizeColumnsToContents();
}

void MainWindow::updateImage() {
    int row = tableWidget->currentRow();
    if (row < 0 || idColumnIndex == -1) return;

    // Get ID from the correct column
    QTableWidgetItem *idItem = tableWidget->item(row, idColumnIndex);
    if (!idItem) return;

    QString picID = idItem->text();
    // Path: resources/images/MON<ID>.png
    QString imagePath = QDir::currentPath() + "/../../resources/images/MON" + picID + ".jpg";

    QPixmap pix(imagePath);
    if (pix.isNull()) {
        imageLabel->setText("Image Missing\n" + picID);
        statusLabel->setText("Path: " + imagePath);
    } else {
        imageLabel->setPixmap(pix.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("Loaded: MON" + picID + ".png");
    }
}

void MainWindow::saveCsv() {
    QFile file("MDATA2.csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    // Write Headers
    for (int i = 0; i < tableWidget->columnCount(); ++i) {
        out << tableWidget->horizontalHeaderItem(i)->text() << (i == tableWidget->columnCount() - 1 ? "" : ",");
    }
    out << "\n";

    // Write Data
    for (int i = 0; i < tableWidget->rowCount(); ++i) {
        for (int j = 0; j < tableWidget->columnCount(); ++j) {
            QTableWidgetItem *item = tableWidget->item(i, j);
            out << (item ? item->text() : "") << (j == tableWidget->columnCount() - 1 ? "" : ",");
        }
        out << "\n";
    }
    file.close();
    QMessageBox::information(this, "Saved", "CSV Updated successfully.");
}
