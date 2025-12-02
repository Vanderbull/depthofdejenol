#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>

class MessagesWindow : public QWidget
{
    Q_OBJECT

public:
    // Constructor
    explicit MessagesWindow(QWidget *parent = nullptr);

public slots:
    /**
     * @brief Slot to receive and display a new message with a timestamp.
     * @param message The string message to append to the log.
     */
    void logMessage(const QString &message);

private:
    // Widget to display the log messages
    QTextEdit *messageDisplay;
};

#endif // MESSAGEWINDOW_H
