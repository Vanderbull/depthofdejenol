#include "SenderWindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QDateTime>

SenderWindow::SenderWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Sender Window");    
    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    // Create the button
    QPushButton *button = new QPushButton("Trigger Log Message", centralWidget);
    layout->addWidget(button);
    // Connect the button's clicked signal to the local slot
    connect(button, &QPushButton::clicked, this, &SenderWindow::handleButtonClick);
}

void SenderWindow::handleButtonClick()
{
    // Create a dynamic message
    QString message = QString("Button clicked at %1.").arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"));
    // Emit the signal, which is connected to MessagesWindow::logMessage
    emit messageTriggered(message);
}
