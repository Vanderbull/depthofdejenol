#include "MessageWindow.h"
#include <QDateTime> 
#include <QApplication>

MessagesWindow::MessagesWindow(QWidget *parent)
    : QWidget(parent)
{
    // Set the window title
    setWindowTitle("Messages Log");
    // Set a fixed minimum size for usability
    setMinimumSize(400, 300);
    // Initialize the QTextEdit widget
    messageDisplay = new QTextEdit(this);
    messageDisplay->setReadOnly(true); // Users shouldn't edit the log
    messageDisplay->setFontPointSize(9); // Smaller font for log data
    // Set up the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(messageDisplay);
    // Set the layout on the widget
    setLayout(layout);
    // Initial log message
    logMessage("Message Window Initialized.");
}

void MessagesWindow::logMessage(const QString &message)
{
    // Get current time for a timestamp
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");

    // Format the final log entry (bold the timestamp for visibility)
    QString logEntry = QString("<b>%1</b>%2").arg(timestamp, message);

    // Append the message to the QTextEdit using HTML (allows the bold tag)
    messageDisplay->append(logEntry);

    // Scroll to the bottom to ensure the newest message is visible
    messageDisplay->ensureCursorVisible();
}
