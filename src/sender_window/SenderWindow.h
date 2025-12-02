#ifndef SENDERWINDOW_H
#define SENDERWINDOW_H

#include <QMainWindow>
#include <QPushButton>

class SenderWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SenderWindow(QWidget *parent = nullptr);

signals:
    /**
     * @brief Signal emitted when a message needs to be logged.
     * @param message The string to be logged by the MessagesWindow.
     */
    void messageTriggered(const QString &message);

private slots:
    // Slot connected to the button click event
    void handleButtonClick();
};

#endif // SENDERWINDOW_H
