#ifndef HELPLESSON_H
#define HELPLESSON_H

#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QLabel>

class HelpLessonDialog : public QDialog
{
    Q_OBJECT

public:
    // Konstruktor: Parent-widgetet är QWidget* parent
    explicit HelpLessonDialog(QWidget *parent = nullptr);
    ~HelpLessonDialog();

private slots:
    // Slot för att hantera klick på länkar i QTextEdit
    void handleAnchorClicked(const QUrl &link);
    // Slot för att hantera klick på Toolbar-knappar (t.ex. Back, History)
    void handleToolbarAction(QAction *action);

private:
    // Hjälpfunktion för att bygga verktygsfältet
    QToolBar* createToolBar();
    // Hjälpfunktion för att bygga innehållsytan (med dvärg-bilden och texten)
    QWidget* createContentArea();
    // Innehåller all HTML-data
    QString createHelpContentHtml();

    QTextEdit *m_textEdit;
    QLabel *m_dwarfImageLabel; // För bilden till vänster

};

#endif // HELPLESSON_H
