#ifndef VICTORYDIALOG_H
#define VICTORYDIALOG_H

#include <QDialog>
#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStringList>

class VictoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit VictoryDialog(const QString& title, const QStringList& paragraphs,
                          bool isFinalVictory, QWidget *parent = nullptr);

signals:


private:
    QLabel* m_titleLabel;
    QLabel* m_bodyLabel;
    QPushButton* m_ngPlusBtn;
    QPushButton* m_quitBtn;
};

#endif
