#include "common_csv_editor.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QGroupBox>
#include <QFont>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

CommonCsvEditor::CommonCsvEditor(const QString& defaultFileName,
                                   const QString& windowTitle,
                                   QWidget* parent)
    : QMainWindow(parent)
{
    m_lastFile = defaultFileName;
    setupUi(windowTitle);
    connectSignals();
}

void CommonCsvEditor::setupUi(const QString& windowTitle)
{
    setWindowTitle(windowTitle);
    resize(1200, 700);

    setStyleSheet(
        "QMainWindow { background: #1e1e1e; }"
        "QGroupBox { background: #252526; border: 1px solid #3a3a3c; "
        "border-radius: 6px; margin-top: 10px; font-weight: bold; color: #d4d4d4; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; color: #569cd6; }"
        "QLabel { color: #d4d4d4; font-size: 13px; }"
        "QTableWidget { background: #252526; color: #d4d4d4; gridline-color: #3a3a3c; "
        "font-size: 13px; }"
        "QTableWidget::item { padding: 2px 6px; }"
        "QTableWidget::item:selected { background: #007acc; color: white; }"
        "QHeaderView::section { background: #3a3a3c; color: #d4d4d4; "
        "border: none; padding: 3px; font-weight: bold; font-size: 12px; }"
        "QPushButton { background: #0e639c; color: white; border: none; "
        "padding: 7px 16px; border-radius: 3px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background: #1177bb; }"
        "QPushButton#secondary { background: #4a4a4a; }"
        "QPushButton#secondary:hover { background: #5a5a5a; }"
        "QLineEdit { background: #3c3c3c; color: #d4d4d4; border: 1px solid #555; "
        "border-radius: 3px; padding: 3px; font-size: 13px; }"
        "QStatusBar { background: #252526; color: #d4d4d4; }"
    );

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);

    table = new QTableWidget(this);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSortingEnabled(true);
    mainLayout->addWidget(table, 1);

    QWidget* sidePanel = new QWidget();
    sidePanel->setMaximumWidth(320);
    QVBoxLayout* sideLayout = new QVBoxLayout(sidePanel);

    QGroupBox* searchGroup = new QGroupBox("Search");
    QVBoxLayout* searchLayout = new QVBoxLayout(searchGroup);
    searchBar = new QLineEdit();
    searchBar->setPlaceholderText("Filter rows...");
    searchLayout->addWidget(searchBar);
    sideLayout->addWidget(searchGroup);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    loadButton = new QPushButton("Load CSV");
    saveButton = new QPushButton("Save CSV");
    exportButton = new QPushButton("Export JSON");
    loadButton->setObjectName("secondary");
    btnLayout->addWidget(loadButton);
    btnLayout->addWidget(saveButton);
    btnLayout->addWidget(exportButton);
    btnLayout->addStretch();
    sideLayout->addLayout(btnLayout);

    QGroupBox* detailsGroup = new QGroupBox("Row Details");
    detailsDisplay = new QTextBrowser();
    detailsDisplay->setOpenExternalLinks(false);
    detailsGroup->setLayout(new QVBoxLayout(detailsGroup));
    detailsGroup->layout()->addWidget(detailsDisplay);
    sideLayout->addWidget(detailsGroup);

    sideLayout->addStretch();
    mainLayout->addWidget(sidePanel);

    statusBar()->showMessage("Ready");
}

void CommonCsvEditor::connectSignals()
{
    connect(loadButton, &QPushButton::clicked, this, [this]() {
        QString fn = QFileDialog::getOpenFileName(this, tr("Open CSV"),
                                     m_lastFile, tr("CSV Files (*.csv)"));
        if (!fn.isEmpty()) loadCsv(fn);
    });
    connect(saveButton, &QPushButton::clicked, this, &CommonCsvEditor::saveCsv);
    connect(exportButton, &QPushButton::clicked, this, &CommonCsvEditor::exportToJson);
    connect(searchBar, &QLineEdit::textChanged, this, &CommonCsvEditor::filterTable);
    connect(table, &QTableWidget::itemSelectionChanged, this, &CommonCsvEditor::updateDetails);
    connect(table, &QTableWidget::cellChanged, this, [this](int, int) {
        emit dataChanged();
    });
}

bool CommonCsvEditor::loadCsv(const QString& fileName)
{
    QString fn = fileName;
    if (fn.isEmpty())
        fn = QFileDialog::getOpenFileName(this, tr("Open CSV"),
                                           m_lastFile, tr("CSV Files (*.csv)"));
    if (fn.isEmpty()) return false;

    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open: %1").arg(fn));
        return false;
    }

    QTextStream in(&file);
    QStringList allLines;
    while (!in.atEnd())
        allLines << in.readLine().trimmed();
    file.close();

    if (allLines.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Empty file."));
        return false;
    }

    // First line = headers
    QStringList rawHeaders = allLines[0].split(",");
    headers.clear();
    for (QString& h : rawHeaders) {
        h.remove("\\uFEFF");
        h = h.trimmed();
        headers << h;
    }

    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount(0);
    table->setSortingEnabled(false);

    for (int i = 1; i < allLines.size(); ++i) {
        if (allLines[i].isEmpty()) continue;
        QStringList fields = allLines[i].split(",");
        for (QString& f : fields) f = f.trimmed();
        while (fields.size() < headers.size())
            fields << "";

        int row = table->rowCount();
        table->insertRow(row);
        for (int col = 0; col < headers.size(); ++col) {
            QString val = col < fields.size() ? fields[col] : "";
            QTableWidgetItem* item = new QTableWidgetItem(val);
            item->setData(Qt::UserRole, val);
            table->setItem(row, col, item);
        }
    }

    table->setSortingEnabled(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_lastFile = fn;
    statusBar()->showMessage(
        tr("Loaded %1 rows from %2").arg(table->rowCount()).arg(fn), 3000);
    updateDetails();
    return true;
}

bool CommonCsvEditor::saveCsv()
{
    if (headers.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("No data to save."));
        return false;
    }
    QString fn = m_lastFile;
    if (fn.isEmpty())
        fn = QFileDialog::getSaveFileName(this, tr("Save CSV"), "",
                                           tr("CSV Files (*.csv)"));
    if (fn.isEmpty()) return false;

    QFile file(fn);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot write: %1").arg(fn));
        return false;
    }

    QTextStream out(&file);
    for (int i = 0; i < headers.size(); ++i) {
        out << csvEscape(headers[i])
            << (i == headers.size() - 1 ? "\n" : ",");
    }
    for (int row = 0; row < table->rowCount(); ++row) {
        for (int col = 0; col < headers.size(); ++col) {
            QString val = table->item(row, col) ? table->item(row, col)->text() : "";
            out << csvEscape(val)
                << (col == headers.size() - 1 ? "\n" : ",");
        }
    }
    file.close();
    statusBar()->showMessage(tr("Saved %1 rows.").arg(table->rowCount()), 3000);
    return true;
}

bool CommonCsvEditor::exportToJson()
{
    if (table->rowCount() == 0) {
        QMessageBox::information(this, tr("Export"), tr("No data to export."));
        return false;
    }

    QString fn = QFileDialog::getSaveFileName(this, tr("Export to JSON"),
                                              m_lastFile, tr("JSON Files (*.json)"));
    if (fn.isEmpty()) return false;

    QJsonArray root;
    for (int i = 0; i < table->rowCount(); ++i) {
        if (table->isRowHidden(i)) continue;
        QJsonObject obj;
        for (int j = 0; j < headers.size(); ++j) {
            QString key = headers[j];
            QString val = table->item(i, j) ? table->item(i, j)->text() : "";
            bool isInt, isFloat;
            int ival = val.toInt(&isInt);
            float fval = val.toFloat(&isFloat);
            if (isInt && !val.contains("."))
                obj.insert(key, ival);
            else if (isFloat && val.contains("."))
                obj.insert(key, fval);
            else
                obj.insert(key, val);
        }
        root.append(obj);
    }

    QJsonDocument doc(root);
    QFile file(fn);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        statusBar()->showMessage(
            tr("Exported %1 rows to JSON.").arg(table->rowCount()), 3000);
        return true;
    }
    return false;
}

void CommonCsvEditor::updateDetails()
{
    QList<QTableWidgetItem*> sel = table->selectedItems();
    if (sel.isEmpty()) {
        detailsDisplay->setHtml("<i style='color:gray;'>No row selected.</i>");
        return;
    }
    int row = sel.first()->row();
    detailsDisplay->setHtml(detailHtml(row));
}

void CommonCsvEditor::filterTable(const QString& text)
{
    for (int i = 0; i < table->rowCount(); ++i)
        table->setRowHidden(i, !rowMatchesFilter(i, text));
}

bool CommonCsvEditor::rowMatchesFilter(int row, const QString& text) const
{
    if (text.isEmpty()) return true;
    for (int col = 0; col < headers.size(); ++col) {
        QString val = table->item(row, col) ? table->item(row, col)->text() : "";
        if (val.contains(text, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

QList<QStringList> CommonCsvEditor::rowData() const
{
    QList<QStringList> result;
    for (int row = 0; row < table->rowCount(); ++row) {
        QStringList r;
        for (int col = 0; col < headers.size(); ++col)
            r << (table->item(row, col) ? table->item(row, col)->text() : "");
        result << r;
    }
    return result;
}

void CommonCsvEditor::setData(const QStringList& h,
                               const QList<QStringList>& rows)
{
    headers = h;
    table->setRowCount(0);
    table->setSortingEnabled(false);
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    for (const QStringList& row : rows) {
        int r = table->rowCount();
        table->insertRow(r);
        for (int col = 0; col < headers.size(); ++col) {
            QString val = col < row.size() ? row[col] : "";
            QTableWidgetItem* item = new QTableWidgetItem(val);
            item->setData(Qt::UserRole, val);
            table->setItem(r, col, item);
        }
    }
    table->setSortingEnabled(true);
    updateDetails();
}

QString CommonCsvEditor::detailHtml(int row) const
{
    QString html =
        "<html><body style='font-family: Consolas, monospace; font-size: 13px;'>";
    html += "<h2 style='color: #569cd6; margin: 0 0 8px 0;'>"
            + (table->item(row, 0) ? table->item(row, 0)->text() : "") + "</h2>";
    html +=
        "<table width='100%' cellpadding='3' cellspacing='0' border='1' "
        "style='border-collapse: collapse; border-color: #3a3a3c;'>";
    for (int i = 1; i < headers.size(); ++i) {
        QString bg = (i % 2 == 0) ? "#2d2d2d" : "#252526";
        html += QString("<tr bgcolor='%1'><td style='color:#9cdcfe;'><b>%2</b></td>"
                        "<td style='color:#d4d4d4;'>%3</td></tr>")
                    .arg(bg)
                    .arg(headers[i])
                    .arg(table->item(row, i) ? table->item(row, i)->text() : "");
    }
    html += "</table></body></html>";
    return html;
}

QString CommonCsvEditor::csvEscape(const QString& s) const
{
    QString copy = s;
    if (copy.contains(",") || copy.contains("\"") || copy.contains("\n")) {
        return "\"" + copy.replace("\"", "\"\"") + "\"";
    }
    return copy;
}
