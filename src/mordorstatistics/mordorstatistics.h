#ifndef MORDORSTATISTICS_H
#define MORDORSTATISTICS_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>

class MordorStatistics : public QDialog
{
    Q_OBJECT

public:
    explicit MordorStatistics(QWidget *parent = nullptr);
    ~MordorStatistics();

private:
    QTableWidget *guildTable;
    QLabel *descriptionLabel;

    void setupUi();
    void populateTable();
};

#endif // MORDORSTATISTICS_H
