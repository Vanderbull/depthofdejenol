#ifndef GENERALSTORE_H
#define GENERALSTORE_H

#include <QDialog>
#include <QTableWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

#include "gameStateManager.h"

class GeneralStore : public QDialog
{
    Q_OBJECT

public:
    explicit GeneralStore(QWidget *parent = nullptr);
    ~GeneralStore() override = default;

private slots:
    void onShopSelectionChanged();
    void onPlayerInventorySelectionChanged();
    void buySelectedItem();
    void sellSelectedItem();
    void identifySelectedItem();
    void uncurseSelectedItem();
    void combineSelectedItems();

private:
    void setupUi();
    void setupStyling();
    void populateShopItems();
    void populatePlayerInventory();
    void updateCharacterHeader();
    void loadItemsFromCsv(const QString& filePath);

    // Dynamic UI Elements
    QLabel *m_charInfoLabel = nullptr;
    QLabel *m_goldLabel = nullptr;

    QTableWidget *m_shopTable = nullptr;
    QListWidget *m_playerInventoryList = nullptr;

    QTextEdit *m_itemDetailsText = nullptr;

    QPushButton *m_buyButton = nullptr;
    QPushButton *m_sellButton = nullptr;
    QPushButton *m_identifyButton = nullptr;
    QPushButton *m_uncurseButton = nullptr;
    QPushButton *m_combineButton = nullptr;

    QList<QVariantMap> m_availableShopItems;
};

#endif // GENERALSTORE_H
