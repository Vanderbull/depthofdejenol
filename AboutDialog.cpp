#include "AboutDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("About This Game");
    // Make the dialog non-resizeable
    setFixedSize(400, 300);
    // Remove the context help button (common for simple dialogs)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi();
}

AboutDialog::~AboutDialog() {}

void AboutDialog::setupUi() {
    // Main vertical layout for the dialog
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. Title Label
    QLabel *titleLabel = new QLabel("Retro Dungeon Crawler");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");

    // 2. Information/Description Text
    QLabel *infoText = new QLabel(
        "Version 1.0.0\n\n"
        "Created by [Your Name/Studio Name]\n\n"
        "This project is a classic-style dungeon crawler game built with Qt 5."
        );
    infoText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    infoText->setWordWrap(true); // Ensure the text wraps

    // 3. Close Button
    QPushButton *closeButton = new QPushButton("Close");
    // Use the QDialog standard slot for accepting (closing) the dialog
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    // Add widgets to the main layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(infoText);

    // Add stretch to push the button to the bottom
    mainLayout->addStretch();

    // Layout for the button to keep it centered
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
}
