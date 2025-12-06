#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QStringList> // Include QStringList

class LoadingScreen : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingScreen(QWidget *parent = nullptr);
    ~LoadingScreen();

private slots:
    void closeDialogAutomatically(); 
    void updateLoadingMessage(); // Slot to cycle the message
    void checkSettingsFile(); // Declaration of the new private slot

private:
    QLabel *m_gameTitleLabel;
    QLabel *m_versionLabel;
    QLabel *m_imageLabel;
    QLabel *m_copyrightLabel;

    // NEW MEMBERS for loading messages
    QLabel *m_loadingMessageLabel; // Label to show the "loading file" text
    QTimer *m_closeTimer;
    QTimer *m_messageTimer; // Timer for message cycling
    
    QStringList m_loadingFiles; // List of fake filenames
    int m_currentFileIndex;     // Index to track which file is currently displayed

    // Removed unused setupUi()
};

#endif // LOADINGSCREEN.H
