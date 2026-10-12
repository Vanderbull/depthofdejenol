#ifndef LOADPARTYDIALOG_H
#define LOADPARTYDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QObject>
#include <QPushButton>

class LoadPartyDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoadPartyDialog(QWidget *parent = nullptr);
    ~LoadPartyDialog();

    QString selectedPartyPath() const;

public slots:
    void onPartySelected();

private slots:
    void onLoadClicked();
    void onCancelClicked();

public:
    QListWidget *m_partyList = nullptr;
    QLabel *m_previewLabel = nullptr;

private:
    void loadPartyList();

    QPushButton *m_loadButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QString m_selectedPath;
};

#endif // LOADPARTYDIALOG_H
