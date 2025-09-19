#include "library_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>

// Constructor implementation
LibraryDialog::LibraryDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Library of Knowledge");
    setMinimumSize(600, 400);

    // Main vertical layout for the entire dialog
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Title label
    QLabel *titleLabel = new QLabel("Library of Knowledge", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);

    // Horizontal layout for the content (list and details)
    QHBoxLayout *contentLayout = new QHBoxLayout();
    mainLayout->addLayout(contentLayout);

    // List widget on the left to show entries
    listWidget = new QListWidget(this);
    contentLayout->addWidget(listWidget);

    // Text edit on the right to show details
    detailsTextEdit = new QTextEdit(this);
    detailsTextEdit->setReadOnly(true);
    detailsTextEdit->setPlaceholderText("Select an item or monster from the list to view its details.");
    detailsTextEdit->setStyleSheet("background-color: #333; color: #fff; border: 1px solid #555;");
    contentLayout->addWidget(detailsTextEdit);

    // Close button
    QPushButton *closeButton = new QPushButton("Close", this);
    mainLayout->addWidget(closeButton, 0, Qt::AlignCenter);

    // --- Connect signals and slots ---
    // Close the dialog when the button is clicked
    connect(closeButton, &QPushButton::clicked, this, &LibraryDialog::accept);

    // Update the details when an item is clicked in the list
    connect(listWidget, &QListWidget::itemClicked, this, &LibraryDialog::updateDetails);

    // Populate the library with placeholder data
    populateLibrary();
}

// Slot implementation
void LibraryDialog::updateDetails(QListWidgetItem *item) {
    QString entryName = item->text();
    if (libraryData.contains(entryName)) {
        const LibraryEntry &entry = libraryData.value(entryName);
        QString details = QString("<b>Name:</b> %1<br>")
                          .arg(entryName);
        details += QString("<b>Type:</b> %1<br><br>")
                   .arg(entry.type);
        details += entry.details;

        detailsTextEdit->setHtml(details);
    }
}

// Populate library implementation
void LibraryDialog::populateLibrary() {
    // Sample monsters
    libraryData["Skeleton Warrior"] = {"Monster", "A reanimated skeleton wielding a rusty sword. Vulnerable to blunt damage and fire spells."};
    libraryData["Giant Rat"] = {"Monster", "A common and weak monster found on the upper levels. Prone to disease and poison."};
    libraryData["Blood Bat"] = {"Monster", "A fast-flying creature that drains blood. Resistant to physical attacks. Weak to ice and light magic."};
    
    // Sample items
    libraryData["Rusty Sword"] = {"Weapon", "A basic, low-level sword. It deals minimal damage and can be sold for a small amount of gold."};
    libraryData["Potion of Healing"] = {"Consumable", "Restores a small amount of health. Can be found in chests or purchased from vendors."};
    libraryData["Amulet of Protection"] = {"Armor", "An enchanted amulet that provides a minor bonus to magical and physical resistance."};
    
    // Add all keys from the map to the list widget
    for (const QString &key : libraryData.keys()) {
        listWidget->addItem(key);
    }
}
