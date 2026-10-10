#ifndef HALLOFRECORDSDIALOG_H
#define HALLOFRECORDSDIALOG_H

#include <QDialog>
#include <QObject>
#include <QVBoxLayout>
#include "src/core/Endgame.h"

class QLabel;
class QPushButton;
class QScrollArea;

class HallOfRecordsDialog : public QDialog {
    Q_OBJECT
public:
    explicit HallOfRecordsDialog(QWidget* parent = nullptr);
    ~HallOfRecordsDialog() override;

private slots:
    void refresh();

private:
    void buildRankedSection(QVBoxLayout* parentLayout, const QString& header,
                            Endgame::Category category);

    QLabel* m_guildRecordsLabel = nullptr;
};

#endif // HALLOFRECORDSDIALOG_H
