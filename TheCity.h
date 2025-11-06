#ifndef THECITY_H
#define THECITY_H

#include <QDialog>
#include <QToolButton> // <--- CHANGED: Use QToolButton for icon/text positioning
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TheCity : public QDialog
{
    Q_OBJECT

public:
    explicit TheCity(QWidget *parent = nullptr);
    ~TheCity();

private slots:
    void on_generalStoreButton_clicked();
    void on_morgueButton_clicked();
    void on_guildsButton_clicked();
    void on_dungeonButton_clicked();
    void on_confinementButton_clicked();
    void on_seerButton_clicked();
    void on_bankButton_clicked();
    void on_exitButton_clicked();

private:
    // Widgets (All now QToolButton)
    QLabel *titleLabel;
    
    QToolButton *generalStoreButton;
    QToolButton *morgueButton;
    QToolButton *guildsButton;
    QToolButton *dungeonButton;
    QToolButton *confinementButton;
    QToolButton *seerButton;
    QToolButton *bankButton;
    QToolButton *exitButton; // The large Exit icon button
    QToolButton *smallExitButton; // The small Exit button at the bottom right

    // Helper Functions
    void setupUi();
    void setupStyling();
    void loadButtonIcons();
};

#endif // THECITY_HIT Y
