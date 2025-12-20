#include "csvviewer.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QSplitter>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QStatusBar> // <--- ADD THIS LINE TO FIX THE ERROR

CSVViewer::CSVViewer(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Dejenol Item Browser");
    resize(1200, 700);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *controls = new QHBoxLayout();
    searchBar = new QLineEdit();
    searchBar->setPlaceholderText("Search item name...");
    
    QPushButton *loadButton = new QPushButton("Open CSV");
    QPushButton *exportButton = new QPushButton("Export to JSON");
    
    controls->addWidget(new QLabel("Search:"));
    controls->addWidget(searchBar);
    controls->addWidget(loadButton);
    controls->addWidget(exportButton);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    table = new QTableWidget();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setSortingEnabled(true);

    detailsDisplay = new QTextBrowser();
    detailsDisplay->setHtml("<i style='color:gray;'>Select an item from the list to see full statistics.</i>");

    splitter->addWidget(table);
    splitter->addWidget(detailsDisplay);
    splitter->setStretchFactor(0, 3); 
    splitter->setStretchFactor(1, 1);

    mainLayout->addLayout(controls);
    mainLayout->addWidget(splitter);
    setCentralWidget(centralWidget);

    connect(loadButton, &QPushButton::clicked, this, &CSVViewer::loadCSV);
    connect(exportButton, &QPushButton::clicked, this, &CSVViewer::exportToJSON);
    connect(searchBar, &QLineEdit::textChanged, this, &CSVViewer::filterTable);
    connect(table, &QTableWidget::itemSelectionChanged, this, &CSVViewer::updateDetails);
}

void CSVViewer::loadCSV() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Item CSV", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for reading.");
        return;
    }

    headers.clear();
    table->setRowCount(0);
    table->setSortingEnabled(false); 
    
    QTextStream in(&file);
    if (!in.atEnd()) {
        headers = in.readLine().split(",");
        table->setColumnCount(headers.size());
        table->setHorizontalHeaderLabels(headers);
    }

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        QStringList fields = line.split(","); 
        int row = table->rowCount();
        table->insertRow(row);
        for (int j = 0; j < fields.size(); ++j) {
            table->setItem(row, j, new QTableWidgetItem(fields[j].trimmed()));
        }
    }
    file.close();
    
    table->setSortingEnabled(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    // This line caused the error; it will now work because of the #include <QStatusBar>
    statusBar()->showMessage(tr("Loaded %1 items.").arg(table->rowCount()), 3000);
}

void CSVViewer::updateDetails() {
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.isEmpty()) return;

    int row = selected.first()->row();
    QString html = "<html><body style='font-family: sans-serif;'>";
    html += "<h2 style='color: #2c3e50;'>" + table->item(row, 0)->text() + "</h2>";
    html += "<hr><table width='100%' cellpadding='4' cellspacing='0'>";

    for (int i = 1; i < headers.size(); ++i) {
        QString bgColor = (i % 2 == 0) ? "#f9f9f9" : "#ffffff";
        html += QString("<tr bgcolor='%1'><td><b>%2</b></td><td>%3</td></tr>")
                .arg(bgColor)
                .arg(headers[i].trimmed())
                .arg(table->item(row, i)->text());
    }
    html += "</table></body></html>";
    detailsDisplay->setHtml(html);
}

void CSVViewer::filterTable(const QString &text) {
    for (int i = 0; i < table->rowCount(); ++i) {
        bool match = table->item(i, 0)->text().contains(text, Qt::CaseInsensitive);
        table->setRowHidden(i, !match);
    }
}

void CSVViewer::exportToJSON() {
    if (table->rowCount() == 0) return;

    QString fileName = QFileDialog::getSaveFileName(this, "Export to JSON", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;

    QJsonArray rootArray;
    for (int i = 0; i < table->rowCount(); ++i) {
        if (table->isRowHidden(i)) continue;
        QJsonObject itemObject;
        for (int j = 0; j < headers.size(); ++j) {
            QString key = headers[j].trimmed();
            QString value = table->item(i, j)->text();
            bool isInt;
            int intVal = value.toInt(&isInt);
            if (isInt) itemObject.insert(key, intVal);
            else itemObject.insert(key, value);
        }
        rootArray.append(itemObject);
    }

    QJsonDocument doc(rootArray);
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        statusBar()->showMessage("Exported to JSON successfully.", 3000);
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    CSVViewer viewer;
    viewer.show();
    return app.exec();
}
