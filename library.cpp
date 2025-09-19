#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QMap>
#include <QString>

// A simple struct to hold item or monster data
struct LibraryEntry {
    QString type;
    QString details;
};

// The main dialog class for the Library of Knowledge
class LibraryDialog : public QDialog {
    Q_OBJECT

public:
    // Constructor to set up the UI and data
    LibraryDialog(QWidget *parent = nullptr) : QDialog(parent) {
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

private slots:
    // Slot to update the detailsTextEdit based on the selected item
    void updateDetails(QListWidgetItem *item) {
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

private:
    // Private members
    QListWidget *listWidget;
    QTextEdit *detailsTextEdit;
    QMap<QString, LibraryEntry> libraryData;

    // A function to populate the library with example data
    void populateLibrary() {
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
};

#include "library_dialog.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    LibraryDialog dialog;
    dialog.show();
    return app.exec();
}
