// SeerDialog.cpp
#include "SeerDialog.h"
#include <QMessageBox> // For demonstration of button clicks

SeerDialog::SeerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("The Seer");
    setFixedSize(350, 200); // Adjust size as needed

    // --- Widgets ---
    welcomeLabel = new QLabel("<b><font color='blue' size='5'>Welcome to the City Seer!</font></b>", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);

    optionsLabel = new QLabel("<font size='4'>Seer Options</font>", this);
    optionsLabel->setContentsMargins(10, 20, 0, 10); // Adjust margins as needed

    characterButton = new QPushButton("Character", this);
    monsterButton = new QPushButton("Monster", this);
    itemButton = new QPushButton("Item", this);
    exitButton = new QPushButton("Exit", this);

    // --- Layouts ---
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(characterButton);
    buttonLayout->addWidget(monsterButton);
    buttonLayout->addWidget(itemButton);

    mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addWidget(optionsLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(); // Pushes buttons to the top
    mainLayout->addWidget(exitButton, 0, Qt::AlignRight); // Align exit button to the bottom right

    // --- Connections ---
    connect(characterButton, &QPushButton::clicked, this, &SeerDialog::on_characterButton_clicked);
    connect(monsterButton, &QPushButton::clicked, this, &SeerDialog::on_monsterButton_clicked);
    connect(itemButton, &QPushButton::clicked, this, &SeerDialog::on_itemButton_clicked);
    connect(exitButton, &QPushButton::clicked, this, &SeerDialog::on_exitButton_clicked);

    // Set layout for the dialog
    setLayout(mainLayout);
}

SeerDialog::~SeerDialog()
{
    // No explicit deletion of child widgets created with 'new' and parented to 'this'
    // Qt's object tree handles their deletion.
}

// --- Slot implementations ---
void SeerDialog::on_characterButton_clicked()
{
    QMessageBox::information(this, "Seer Option", "Character button clicked!");
}

void SeerDialog::on_monsterButton_clicked()
{
    QMessageBox::information(this, "Seer Option", "Monster button clicked!");
}

void SeerDialog::on_itemButton_clicked()
{
    QMessageBox::information(this, "Seer Option", "Item button clicked!");
}

void SeerDialog::on_exitButton_clicked()
{
    QMessageBox::information(this, "Exit", "Exiting the Seer!");
    accept(); // Closes the dialog with QDialog::Accepted result
}
