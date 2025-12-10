#ifndef HELPLESSON_H
#define HELPLESSON_H

#include <QDialog>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QLabel>
#include <QUrl>
#include <QAction>
#include "GameStateManager.h"

class HelpLessonDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpLessonDialog(QWidget *parent = nullptr);
    ~HelpLessonDialog();

private slots:
    void handleAnchorClicked(const QUrl &link);
    void handleToolbarAction(QAction *action);

private:
    QToolBar* createToolBar();
    QWidget* createContentArea();
    QString createHelpContentHtml();

    QTextBrowser *m_textEdit;
    QLabel *m_dwarfImageLabel;
};

#endif // HELPLESSON_H
