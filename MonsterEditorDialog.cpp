#include "MonsterEditorDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFile>

MonsterEditorDialog::MonsterEditorDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Monster Editor");
    setFixedSize(500, 450);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Attempt to load the external style sheet
    QFile styleSheetFile("style.qss");
    if (styleSheetFile.open(QFile::ReadOnly | QFile::Text)) {
        setStyleSheet(styleSheetFile.readAll());
        styleSheetFile.close();
    }

    setupUi();
}

MonsterEditorDialog::~MonsterEditorDialog() {}

void MonsterEditorDialog::setupUi() {
    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. Form Layout for Inputs (Name, Level, HP, Damage)
    QFormLayout *formLayout = new QFormLayout();

    nameInput = new QLineEdit();
    levelInput = new QSpinBox();
    hitPointsInput = new QSpinBox();
    damageInput = new QSpinBox();

    // Configure spin boxes (set reasonable min/max values)
    levelInput->setRange(1, 999);
    hitPointsInput->setRange(1, 9999);
    damageInput->setRange(1, 999);

    // Add widgets to the form layout
    formLayout->addRow("Monster Name:", nameInput);
    formLayout->addRow("Level:", levelInput);
    formLayout->addRow("Hit Points:", hitPointsInput);
    formLayout->addRow("Damage:", damageInput);

    mainLayout->addLayout(formLayout);

    // 2. Description Text Area
    QLabel *descriptionLabel = new QLabel("Description:");
    descriptionInput = new QTextEdit();
    descriptionInput->setMinimumHeight(100);

    mainLayout->addWidget(descriptionLabel);
    mainLayout->addWidget(descriptionInput);

    mainLayout->addStretch();

    // 3. Control Buttons (Save and Cancel)
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *saveButton = new QPushButton("Save Monster");
    QPushButton *cancelButton = new QPushButton("Cancel");

    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connect slots
    connect(saveButton, &QPushButton::clicked, this, &MonsterEditorDialog::onSaveClicked);
    connect(cancelButton, &QPushButton::clicked, this, &MonsterEditorDialog::onCancelClicked);

    // Set initial values (for demonstration)
    nameInput->setText("Goblin");
    levelInput->setValue(3);
    hitPointsInput->setValue(15);
    damageInput->setValue(5);
    descriptionInput->setText("A small, weak monster often found on the upper levels of the dungeon. Easily defeated by starting adventurers.");
}

void MonsterEditorDialog::onSaveClicked() {
    // This slot is called when the user confirms the data.
    // In a real application, you would save the data to a file or database here.
    // We use accept() to close the dialog with a positive result.
    accept();
}

void MonsterEditorDialog::onCancelClicked() {
    // Close the dialog, discarding any changes.
    reject();
}

QString MonsterEditorDialog::getMonsterData() const {
    // Example function to retrieve all data as a formatted string
    QString data = QString("Name: %1\nLevel: %2\nHP: %3\nDamage: %4\nDescription: %5")
                       .arg(nameInput->text())
                       .arg(levelInput->value())
                       .arg(hitPointsInput->value())
                       .arg(damageInput->value())
                       .arg(descriptionInput->toPlainText().trimmed());

    return data;
}
