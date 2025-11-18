#include "marlith_dialog.h"
#include <QVBoxLayout>
#include <QMessageBox>

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
    // When a button is clicked, a QMessageBox is displayed
    connect(storeButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Location", "The Store\n\nThis is where all of the trading, buying and selling of things of worth takes place. Carrying a nice line of Bronze weapons. Leather armor. and goods that can be used for a rough day in the dungeon, the store is an ideal place for any adventurer to find what they need. Services such as Item Identification, Item Realignment, and Cursed Item Separation are also offered.\n\nBe aware that the store clerks are very well known for charging hefty fees for item-related services. They're also notorious for buying items at half their value and selling the same items for full price when you need to buy it back. Adventurers who pay close attention can quite often tell the true nature of an unidentified item by the price that a clerk may offer to pay for the item.");
    });
    connect(confinementButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Location", "Confinement & Holding\n\nOne of the most recent additions to the city is the Confinement & Holding center for the various creatures that are found in the depths. Here, an explorer can sell companions who joined the adventurer by choice, or were bound by a spell. Depending on the abilities and type of creature, keepers will pay a very gracious price for the right monster. The center also deals in the selling of creatures that are currently in confinement, allowing an explorer to choose a new companion. And for those adventurers who have companions that are close to breaking their binding (or are just plain aggravated), there's always the option of increasing the companions bind level, which reinforces the creatures loyalty to the character by substantial amounts. Be warned that this feature can be very expensive if the monster is hard to control, and isn't always successful!");
    });
    connect(guildButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Location", "The Guilds\n\nHeadquarters to the local schools of training, an adventurer can learn any type of skill that they are need here. Whether it's magical abilities, thieving abilities, or fighting abilities, there's always a guild ready to teach an eager adventurer. The guilds also keep a common library of the items and creatures encountered in the dungeon. Each guild also keeps a log of it's members' progress and achievements. From this log, the guild masters are able to periodically assign quests to check the skill of their students.");
    });
    connect(bankButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Location", "The Bank\n\nA large storehouse that's located in the center of the city, the Bank is constantly guarded by the elite of the Warriors guild. Every individual who opens an account with the bank is guaranteed full monetary and item protection. It's strongly recommended that adventurers off-load their valuable items and gold before heading into the dungeon, where it might be stolen.");
    });
    connect(morgueButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Location", "The Morgue\n\nHeaded by the master of the Healer's guild, this is the place where people look when they've 'lost' a comrade. Trained in the arts of search, rescue, and resurrection, the Morgue workers are very good at what they do. Able to raise the dead and even bring a stoned adventurer back to life, the workers are well known for their abilities, and the amount of gold they charge for their services.\n\nThe search and rescue team of the Morgue is fairly skilled at bringing back dead characters from the dungeon - as long as they're told the general location of the body. However, even with their powerful magical abilities, a rescue team quite often is never heard from again - much like the gold that is charged for initiating such a search. The only kind of death that the Morgue workers are unable to deal with is those who have materialized in rock. Only a master in the ways of the Mages guild can remove characters from rock.");
    });
    connect(theSeerButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Location", "The Seer\n\nPossessing the power to \"see beyond\", the city Seer has the ability to locate other characters that may be lost in the dungeon, creatures that one may be quested for, and items that creatures may carry.\n\nHis powers are useful to find anybody who has \"disappeared\" recently so that a search & rescue party may be sent to the general area which the seeker senses. Quite often, his powers are strong enough to give the exact location of the character, and even the character's status.\n\nWhen locating a creature or item, the Seer needs to know the exact name of the object of interest before he can proceed. Unfortunately, unlike a lost character, the energy emitting from an unknown creature or item is quite often harder to pinpoint. This makes it harder and more expensive. The same is true for locating items or creatures on the lower levels.");
    });
}

// Destructor to clean up allocated memory
MarlithDialog::~MarlithDialog()
{
}
