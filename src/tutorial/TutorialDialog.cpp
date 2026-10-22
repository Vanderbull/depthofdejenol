#include "TutorialDialog.h"

#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QDialogButtonBox>
#include <QIcon>

TutorialDialog::TutorialDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Tutorial");
    resize(500, 400);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // List widget for tutorial steps
    m_listWidget = new QListWidget(this);
    m_listWidget->setIconSize(QSize(32, 32));
    mainLayout->addWidget(m_listWidget);

    // Button box
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_buttonBox->setCenterButtons(true);
    mainLayout->addWidget(m_buttonBox);

    // Connect button box
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Populate steps
    m_listWidget->addItem(new QListWidgetItem(QString("1. Create a character")));
    m_listWidget->addItem(new QListWidgetItem(QString("2. Buy a weapon in the general store")));
    m_listWidget->addItem(new QListWidgetItem(QString("3. Go down to the dungeon")));
    m_listWidget->addItem(new QListWidgetItem(QString("4. Fight a monster")));

    // Show first step
    showStep(0);
}

void TutorialDialog::showStep(int step)
{
    if (step < 0 || step >= m_listWidget->count())
        return;

    m_currentStep = step;

    // Update button box text based on current step
    if (step == m_listWidget->count() - 1) {
        m_buttonBox->setStandardButtons(QDialogButtonBox::Close);
    } else {
        m_buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    }

    m_listWidget->setCurrentRow(step);
    m_listWidget->scrollToItem(m_listWidget->currentItem());
}
