#ifndef LOADPARTYDIALOG_H
#define LOADPARTYDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

class LoadPartyDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoadPartyDialog(QWidget *parent = nullptr);
    ~LoadPartyDialog();

    QString selectedPartyPath() const;

private slots:
    void onPartySelected();
    void onLoadClicked();
    void onCancelClicked();

private:
    void loadPartyList();

    QListWidget *m_partyList = nullptr;
    QLabel *m_previewLabel = nullptr;
    QPushButton *m_loadButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QString m_selectedPath;
};

#endif // LOADPARTYDIALOG_H
