#ifndef GAME_DATA_EDITOR_H
#define GAME_DATA_EDITOR_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QTextBrowser>
#include <QSplitter>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QStatusBar>

class GameDataEditor : public QMainWindow
{
    Q_OBJECT
public:
    explicit GameDataEditor(QWidget* parent = nullptr);

private slots:
    void loadFile();
    void saveFile();
    void loadExample();
    void refreshAllTabs();

private:
    // Tabs for each data block
    QTabWidget* m_tabWidget;
    // Each tab: table on left, JSON preview on right
    struct TabData {
        QString name;
        QTableWidget* table;
        QTextBrowser* preview;
    };
    QList<TabData> m_tabs;

    // Currently loaded data (as JSON)
    QJsonObject m_gameData;
    QString m_currentFile;

    void setupUi();
    void createRaceTab();
    void createGuildTab();
    void createItemTypeTab();
    void createMonsterTypeTab();
    void updateRaceTable(const QJsonArray& races);
    void updateGuildTable(const QJsonArray& guilds);
    void updateItemTypeTable(const QJsonArray& items);
    void updateMonsterTypeTable(const QJsonArray& monsters);
    void updateAllPreviews();
    void exportToFile(const QString& path);
    QJsonObject readFromFile(const QString& path);
};

#endif
