#include "MorgueDialog.h"
#include <QMessageBox>

MorgueDialog::MorgueDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("Morgue");

    // Welcome Label
    welcomeLabel = new QLabel("Welcome to the Morgue!");
    welcomeLabel->setStyleSheet("color: blue; font-weight: bold; font-size: 16px;");
    welcomeLabel->setAlignment(Qt::AlignCenter);

    // Buttons
    raiseCharacterButton = new QPushButton("Raise Character");
    hireRescuersButton = new QPushButton("Hire Rescuers");
    grabBodyButton = new QPushButton("Grab Body");
    exitButton = new QPushButton("Exit");

    // Connect signals to slots
    connect(raiseCharacterButton, &QPushButton::clicked, this, &MorgueDialog::onRaiseCharacterClicked);
    connect(hireRescuersButton, &QPushButton::clicked, this, &MorgueDialog::onHireRescuersClicked);
    connect(grabBodyButton, &QPushButton::clicked, this, &MorgueDialog::onGrabBodyClicked);
    connect(exitButton, &QPushButton::clicked, this, &MorgueDialog::onExitClicked);

    // Layouts
    QHBoxLayout *topButtonLayout = new QHBoxLayout();
    topButtonLayout->addWidget(raiseCharacterButton);
    topButtonLayout->addWidget(hireRescuersButton);

    QHBoxLayout *middleButtonLayout = new QHBoxLayout();
    middleButtonLayout->addWidget(grabBodyButton);
    middleButtonLayout->addStretch(); // Pushes "Grab Body" to the left

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addLayout(topButtonLayout);
    mainLayout->addLayout(middleButtonLayout);
    mainLayout->addStretch(); // Pushes everything upwards
    mainLayout->addWidget(exitButton, 0, Qt::AlignRight); // Align Exit button to the right

    setLayout(mainLayout);

    // Set a fixed size to mimic the image's appearance more closely
    setFixedSize(300, 200);
}

MorgueDialog::~MorgueDialog()
{
}

void MorgueDialog::onRaiseCharacterClicked()
{
    QMessageBox::information(this, "Morgue Action", "Raise Character button clicked!");
}

void MorgueDialog::onHireRescuersClicked()
{
    QMessageBox::information(this, "Morgue Action", "Hire Rescuers button clicked!");
}

void MorgueDialog::onGrabBodyClicked()
{
    QMessageBox::information(this, "Morgue Action", "Grab Body button clicked!");
}

void MorgueDialog::onExitClicked()
{
    close(); // Close the dialog
}
