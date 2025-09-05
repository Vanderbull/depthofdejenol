#include "hallofrecordsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScreen>
#include <QGuiApplication>
#include <QFile>

HallOfRecordsDialog::HallOfRecordsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Hall of Records");
    
    // Load external style sheet
    QFile styleSheetFile("style.qss");
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }

    // Get screen geometry to make the window scale with screen resolution
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int windowWidth = screenGeometry.width() * 0.75;
    int windowHeight = screenGeometry.height() * 0.70;
    
    // Set a minimum size to prevent the window from becoming too small
    setMinimumSize(750, 600);
    resize(windowWidth, windowHeight);

    // Main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    // Left column for Hall of Records
    QGridLayout *recordsLayout = new QGridLayout();

    QLabel *recordsTitle = new QLabel("Hall of Records");
    recordsTitle->setStyleSheet("font-size: 18px; font-weight: bold;");
    recordsLayout->addWidget(recordsTitle, 0, 0, 1, 2, Qt::AlignCenter);

    // Headers for records
    QLabel *rankHeader = new QLabel("Rank");
    recordsLayout->addWidget(rankHeader, 1, 0, Qt::AlignCenter);
    
    QLabel *nameHeader = new QLabel("Name");
    recordsLayout->addWidget(nameHeader, 1, 1, Qt::AlignCenter);

    // Sample records data
    for (int i = 0; i < 10; ++i) {
        QLabel *rankLabel = new QLabel(QString::number(i + 1));
        recordsLayout->addWidget(rankLabel, i + 2, 0, Qt::AlignCenter);

        QLabel *nameLabel = new QLabel(QString("Player %1").arg(i + 1));
        recordsLayout->addWidget(nameLabel, i + 2, 1, Qt::AlignCenter);
    }

    // Right column for Masters of Dejenol
    QGridLayout *mastersLayout = new QGridLayout();

    QLabel *mastersTitle = new QLabel("Masters of Dejenol");
    mastersTitle->setStyleSheet("font-size: 18px; font-weight: bold;");
    mastersLayout->addWidget(mastersTitle, 0, 0, 1, 2, Qt::AlignCenter);

    // Headers for masters
    QLabel *masterNameHeader = new QLabel("Name");
    mastersLayout->addWidget(masterNameHeader, 1, 0, Qt::AlignCenter);

    QLabel *masterDungeonHeader = new QLabel("Dungeon");
    mastersLayout->addWidget(masterDungeonHeader, 1, 1, Qt::AlignCenter);

    // Sample masters data
    for (int i = 0; i < 10; ++i) {
        QLabel *nameLabel = new QLabel(QString("Master %1").arg(i + 1));
        mastersLayout->addWidget(nameLabel, i + 2, 0, Qt::AlignCenter);

        QLabel *dungeonLabel = new QLabel("Dungeon 10");
        mastersLayout->addWidget(dungeonLabel, i + 2, 1, Qt::AlignCenter);
    }

    // Add layouts to the main layout
    mainLayout->addLayout(recordsLayout);
    mainLayout->addLayout(mastersLayout);
    
    // Add close button at the bottom center
    QVBoxLayout *bottomLayout = new QVBoxLayout();
    QPushButton *closeButton = new QPushButton("Close");
    closeButton->setFixedSize(150, 40);
    bottomLayout->addWidget(closeButton, 0, Qt::AlignCenter);

    mainLayout->addLayout(bottomLayout);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
}

HallOfRecordsDialog::~HallOfRecordsDialog() {}
