#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QStringList>
#include <QPropertyAnimation>    // Added for the animation system
#include <QGraphicsOpacityEffect> // Added to allow opacity manipulation

class LoadingScreen : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingScreen(QWidget *parent = nullptr);
    ~LoadingScreen();

private slots:
    void closeDialogAutomatically(); 
    void updateLoadingMessage(); 
    void checkSettingsFile(); 

private:
    QLabel *m_gameTitleLabel;
    QLabel *m_versionLabel;
    QLabel *m_imageLabel;
    QLabel *m_copyrightLabel;

    QLabel *m_loadingMessageLabel; 
    QTimer *m_closeTimer;
    QTimer *m_messageTimer; 
    
    QStringList m_loadingFiles; 
    int m_currentFileIndex;     

    // Animation System Members
    QGraphicsOpacityEffect *m_titleOpacityEffect; // The "bridge" between the widget and animation
    QPropertyAnimation *m_titleFadeAnimation;     // The animation controller
};

#endif // LOADINGSCREEN_H
