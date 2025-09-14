#include "marlith_dialog.h"
#include <QVBoxLayout>
#include <QDebug>

// Constructor for the MarlithDialog class
MarlithDialog::MarlithDialog(QWidget *parent) :
    QDialog(parent),
    ui(nullptr) // Initialize the ui pointer to null
{
    // Set the window title
    setWindowTitle("City of Marlith");

    // Create the buttons for each location
    storeButton = new QPushButton("Store", this);
    confinementButton = new QPushButton("Confinement & Holding", this);
    guildButton = new QPushButton("Guild", this);
    bankButton = new QPushButton("Bank", this);
    morgueButton = new QPushButton("Morgue", this);
    theSeerButton = new QPushButton("The Seer", this);

    // Create a vertical layout to arrange the buttons
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(storeButton);
    layout->addWidget(confinementButton);
    layout->addWidget(guildButton);
    layout->addWidget(bankButton);
    layout->addWidget(morgueButton);
    layout->addWidget(theSeerButton);

    // Connect the buttons to slots using C++11 lambda functions
    // When a button is clicked, a message is printed to the console
    connect(storeButton, &QPushButton::clicked, [this]() {
        qDebug() << "Store button clicked!";
    });
    connect(confinementButton, &QPushButton::clicked, [this]() {
        qDebug() << "Confinement & Holding button clicked!";
    });
    connect(guildButton, &QPushButton::clicked, [this]() {
        qDebug() << "Guild button clicked!";
    });
    connect(bankButton, &QPushButton::clicked, [this]() {
        qDebug() << "Bank button clicked!";
    });
    connect(morgueButton, &QPushButton::clicked, [this]() {
        qDebug() << "Morgue button clicked!";
    });
    connect(theSeerButton, &QPushButton::clicked, [this]() {
        qDebug() << "The Seer button clicked!";
    });
}

// Destructor to clean up allocated memory
MarlithDialog::~MarlithDialog()
{
    // No need to delete ui as it's not used in this simplified example
}
