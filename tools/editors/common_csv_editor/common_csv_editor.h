#ifndef COMMON_CSV_EDITOR_H
#define COMMON_CSV_EDITOR_H

#include <QMainWindow>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QSplitter>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QStringList>
#include <QList>

class CommonCsvEditor : public QMainWindow
{
    Q_OBJECT
public:
    explicit CommonCsvEditor(const QString& defaultFileName = QString(),
                             const QString& windowTitle = "CSV Editor",
                             QWidget* parent = nullptr);
    virtual ~CommonCsvEditor() = default;

    bool loadCsv(const QString& fileName = QString());
    bool saveCsv();
    bool exportToJson();
    QList<QStringList> rowData() const;
    void setData(const QStringList& headers, const QList<QStringList>& rows);

signals:
    void dataChanged();

protected:
    virtual QString detailHtml(int row) const;
    virtual bool rowMatchesFilter(int row, const QString& filterText) const;

    QTableWidget* table = nullptr;
    QLineEdit* searchBar = nullptr;
    QTextBrowser* detailsDisplay = nullptr;
    QStringList headers;
    QString m_lastFile;
    QPushButton* loadButton = nullptr;
    QPushButton* saveButton = nullptr;
    QPushButton* exportButton = nullptr;

    void setupUi(const QString& windowTitle);
    void connectSignals();
    void updateDetails();
    void filterTable(const QString& text);

    QString csvEscape(const QString& s) const;
};

#endif
