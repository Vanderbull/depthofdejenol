#ifndef Dungeondialog_H
#define Dungeondialog_H

#include <QDialog>
#include <QPixmap>

class DungeonDialog : public QDialog {
    Q_OBJECT

public:
    explicit DungeonDialog(QWidget *parent = nullptr);
    ~DungeonDialog();

private slots:
    void showPartyInfoDialog();

private:
    void setupUi();
};

#endif // Dungeondialog_H
