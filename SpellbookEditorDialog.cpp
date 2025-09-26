#include "SpellbookEditorDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QDebug>

SpellbookEditorDialog::SpellbookEditorDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Spellbook Editor");
    // Set a fixed size for consistency
    setFixedSize(500, 480);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Load external style sheet (assuming you have 'style.qss')
    QFile styleSheetFile("style.qss");
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }

    setupUi();
}

SpellbookEditorDialog::~SpellbookEditorDialog() {}

void SpellbookEditorDialog::setupUi() {
    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. Form Layout for Spell Properties
    QFormLayout *formLayout = new QFormLayout();

    nameInput = new QLineEdit();
    levelInput = new QSpinBox();
    manaCostInput = new QSpinBox();
    powerInput = new QSpinBox();
    targetTypeInput = new QComboBox();

    // Configure spin boxes (min/max values)
    levelInput->setRange(1, 10); // Spells Level 1 through 10
    manaCostInput->setRange(0, 999);
    powerInput->setRange(0, 9999); // Up to 9999 damage/heal

    // Configure Target Type ComboBox
    targetTypeInput->addItem("Single Target (Enemy)");
    targetTypeInput->addItem("Area of Effect (Enemies)");
    targetTypeInput->addItem("Single Target (Ally)");
    targetTypeInput->addItem("Self");

    // Add inputs to the form layout
    formLayout->addRow("Spell Name:", nameInput);
    formLayout->addRow("Spell Level (Tier):", levelInput);
    formLayout->addRow("Mana Cost:", manaCostInput);
    formLayout->addRow("Power (Damage/Heal):", powerInput);
    formLayout->addRow("Target Type:", targetTypeInput);

    mainLayout->addLayout(formLayout);

    // 2. Description Text Area
    QLabel *descriptionLabel = new QLabel("Description/Full Effect:");
    descriptionInput = new QTextEdit();
    descriptionInput->setMinimumHeight(100);

    mainLayout->addWidget(descriptionLabel);
    mainLayout->addWidget(descriptionInput);

    mainLayout->addStretch();

    // 3. Control Buttons (Save and Cancel)
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *saveButton = new QPushButton("Save Spell");
    QPushButton *cancelButton = new QPushButton("Cancel");

    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connect slots
    connect(saveButton, &QPushButton::clicked, this, &SpellbookEditorDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &SpellbookEditorDialog::onCancelClicked);

    // Set initial values for a default spell
    nameInput->setText("Fireball");
    levelInput->setValue(3);
    manaCostInput->setValue(35);
    powerInput->setValue(50);
    targetTypeInput->setCurrentIndex(1); // 'Area of Effect'
    descriptionInput->setText("Unleashes a massive ball of flame, dealing high damage to all enemies in the encounter. May inflict 'Burn' status.");
}

void SpellbookEditorDialog::onSaveClicked() {
    // Closes the dialog with QDialog::Accepted status
    accept();
}

void SpellbookEditorDialog::onCancelClicked() {
    // Closes the dialog with QDialog::Rejected status
    reject();
}

QString SpellbookEditorDialog::getSpellData() const {
    // Compiles all input data into a single string
    QString data = QString(
                       "Name: %1\n"
                       "Level: %2\n"
                       "Mana Cost: %3\n"
                       "Power: %4\n"
                       "Target: %5\n"
                       "Description: %6"
                       )
                       .arg(nameInput->text())
                       .arg(levelInput->value())
                       .arg(manaCostInput->value())
                       .arg(powerInput->value())
                       .arg(targetTypeInput->currentText())
                       .arg(descriptionInput->toPlainText().trimmed());

    return data;
}
