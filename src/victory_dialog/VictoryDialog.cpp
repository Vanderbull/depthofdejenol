#include "VictoryDialog.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>

VictoryDialog::VictoryDialog(const QString& title, const QStringList& paragraphs,
                            bool isFinalVictory, QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setMinimumSize(600, 400);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(20);
    layout->setContentsMargins(40, 40, 40, 40);

    // Title
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("Georgia", 28, QFont::Bold);
    m_titleLabel->setFont(titleFont);
    QPalette titlePal = m_titleLabel->palette();
    titlePal.setColor(QPalette::WindowText, QColor("#FFD700"));  // Gold
    m_titleLabel->setPalette(titlePal);
    layout->addWidget(m_titleLabel);

    // Body
    m_bodyLabel = new QLabel(paragraphs.join("\n\n"), this);
    m_bodyLabel->setAlignment(Qt::AlignCenter);
    m_bodyLabel->setWordWrap(true);
    QFont bodyFont("Georgia", 16);
    m_bodyLabel->setFont(bodyFont);
    QPalette bodyPal = m_bodyLabel->palette();
    bodyPal.setColor(QPalette::WindowText, Qt::white);
    m_bodyLabel->setPalette(bodyPal);
    layout->addWidget(m_bodyLabel);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    if (isFinalVictory) {
        m_ngPlusBtn = new QPushButton("New Game Plus", this);
        m_ngPlusBtn->setMinimumSize(150, 40);
        connect(m_ngPlusBtn, &QPushButton::clicked, this, &VictoryDialog::startNewGamePlus);
        btnLayout->addWidget(m_ngPlusBtn);
    }

    m_quitBtn = new QPushButton("Return to Menu", this);
    m_quitBtn->setMinimumSize(150, 40);
    if (m_ngPlusBtn) {
        connect(m_ngPlusBtn, &QPushButton::clicked, this, &VictoryDialog::startNewGamePlus);
    }
    connect(m_quitBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_quitBtn);

    layout->addLayout(btnLayout);

    // Fade-in animation
    QGraphicsOpacityEffect* eff = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(eff);
    eff->setProperty("opacity", 0.0);
    QPropertyAnimation* fadeIn = new QPropertyAnimation(eff, "opacity");
    fadeIn->setDuration(1000);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    group->addAnimation(fadeIn);
    group->start();

    // Dark background
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(10, 10, 30));
    setPalette(pal);
}
