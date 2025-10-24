#include "helplesson.h"
#include <QPushButton>
#include <QAction>
#include <QDebug>
#include <QTextDocument>

// --- Konstruktor ---
HelpLessonDialog::HelpLessonDialog(QWidget *parent)
    : QDialog(parent)
{
    // Sätt fönstertiteln
    setWindowTitle(tr("Mordor HelpLesson"));
    // Sätt en rimlig startstorlek
    resize(700, 600);

    // Huvudlayout för hela fönstret
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Ta bort marginaler för en renare look

    // 1. Skapa och lägg till verktygsfältet (Toolbar)
    QToolBar *toolbar = createToolBar();
    mainLayout->addWidget(toolbar);

    // 2. Skapa och lägg till innehållsytan (Bild + Text)
    QWidget *contentArea = createContentArea();
    mainLayout->addWidget(contentArea);

    // (Valfritt) Ge fönstret en klassisk grå bakgrundsfärg
    setStyleSheet("QDialog { background-color: #f0f0f0; }");
}

HelpLessonDialog::~HelpLessonDialog()
{
}

// --- Verktygsfältet ---
QToolBar* HelpLessonDialog::createToolBar()
{
    QToolBar *toolbar = new QToolBar(this);
    toolbar->setStyleSheet("QToolBar { background-color: #e5e5e5; border-bottom: 1px solid #c0c0c0; }");

    // Knappar till vänster
    toolbar->addAction(tr("Contents"));
    toolbar->addAction(tr("Search"));
    toolbar->addAction(tr("Back"));
    toolbar->addAction(tr("History"));

    // Separator och dubbelpilar
    toolbar->addSeparator();
    toolbar->addAction(tr("<<"));
    toolbar->addAction(tr(">>"));

    // Koppla alla actions till en gemensam slot
    connect(toolbar, &QToolBar::actionTriggered, this, &HelpLessonDialog::handleToolbarAction);

    return toolbar;
}

// --- Innehållsytan (Bild + TextEdit) ---
QWidget* HelpLessonDialog::createContentArea()
{
    QWidget *container = new QWidget(this);
    // Använd en H-layout för att placera bilden (dvärgen) bredvid textinnehållet
    QHBoxLayout *hLayout = new QHBoxLayout(container);
    hLayout->setContentsMargins(10, 10, 10, 10); // Standard marginaler för att innehållet inte ska klibba mot kanterna

    // Vänster: Bilden (QLabel)
    m_dwarfImageLabel = new QLabel(this);
    // OBS: Du måste ersätta "dwarf_image.png" med sökvägen till din bild eller en QResource.
    // QPixmap pixmap(":/images/dwarf_image.png"); 
    // m_dwarfImageLabel->setPixmap(pixmap.scaled(100, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_dwarfImageLabel->setText(""); // Placeholder tills en bild laddas
    m_dwarfImageLabel->setFixedSize(150, 200); // Fixerad storlek baserat på bilden

    hLayout->addWidget(m_dwarfImageLabel, 0); // Väger 0 (tar minimalt utrymme)

    // Höger: Textinnehållet (QTextEdit)
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    // Ta bort kanten för att smälta in i dialogen
    m_textEdit->setFrameShape(QFrame::NoFrame);
    
    // Styla länkar för att matcha den gamla blå färgen
    m_textEdit->document()->setDefaultStyleSheet("a { color: blue; text-decoration: none; }");
    
    // Ladda in innehållet
    m_textEdit->setHtml(createHelpContentHtml());

    // Koppla klick på länkar
//    connect(m_textEdit, &QTextEdit::anchorClicked, this, &HelpLessonDialog::handleAnchorClicked);
//connect(m_textEdit, QOverload<const QUrl&>::of(&QTextEdit::anchorClicked), this, &HelpLessonDialog::handleAnchorClicked);
// Fix: Använd static_cast för att explicit peka på den överlagrade anchorClicked(const QUrl&) signalen.
//    connect(m_textEdit, static_cast<void (QTextEdit::*)(const QUrl&)>(&QTextEdit::anchorClicked),
//            this, &HelpLessonDialog::handleAnchorClicked);
connect(m_textEdit, SIGNAL(anchorClicked(QUrl)),
            this, SLOT(handleAnchorClicked(QUrl)));
    hLayout->addWidget(m_textEdit, 1); // Väger 1 (tar upp resterande utrymme)
    
    return container;
}

// --- HTML-innehållet ---
QString HelpLessonDialog::createHelpContentHtml()
{
    // Återskapa stilen med HTML/CSS. Notera den mörkblå färgen för rubriken.
    QString html = R"(
        <html>
        <body style='font-family: sans-serif; background-color: #f0f0f0;'>
            
            <p style='margin-bottom: 20px;'></p> <h1 style='color: #00008b; margin-bottom: 5px;'>Mordor 1.1 HelpLesson</h1>
            <p style='font-size: 10pt; color: #555;'>(Last Edited 7/5/95)</p>
            
            <p>Welcome to the Mordor HelpLesson! If this is your first time, it is recommended that you use the available browse buttons to go through the sequential order of the HelpLesson.</p>

            <hr style='border: none; border-top: 1px solid #cccccc;' />

            <h2 style='margin-top: 20px; font-weight: bold;'>Below is a list of Mordor Game Playing Topics.</h2>
            
            <div style='display: flex; justify-content: space-between;'>
                
                <div style='width: 48%;'>
                    <p><a href='#introduction'>Introduction</a></p>
                    <p><a href='#history'>The History of Mordor</a></p>
                    <p><a href='#overview'>Game Overview</a></p>
                    <p><a href='#mainmenu'>Main Menu</a></p>
                    <p><a href='#gameoptions'>Game Options Window</a></p>
                    <p><a href='#characterinfo'>Character Information</a></p>
                    <p><a href='#races'>Races</a></p>
                    <p><a href='#createchar'>Creating a Character</a></p>
                    <p><a href='#gamescreen'>The Game Screen</a></p>
                    <p><a href='#toolbar'>The Toolbar</a></p>
                    <p><a href='#city'>The City</a></p>
                    <p><a href='#items'>Items</a></p>
                    <p><a href='#guilds'>Guilds</a></p>
                    <p><a href='#library'>The Library</a></p>
                    <p><a href='#spells'>Spells</a></p>
                </div>

                <div style='width: 48%;'>
                    <p><a href='#parties'>Parties</a></p>
                    <p><a href='#commands'>Commands</a></p>
                    <p><a href='#controlpanel'>Control Panel</a></p>
                    <p style='margin-top: 20px;'></p>
                    <p><a href='#beginnerhints'>Beginner Hints & Tips</a></p>
                    <p><a href='#faq'>Frequently Asked Questions</a></p>
                    
                    <h2 style='margin-top: 20px; font-weight: bold; border-top: 1px solid #cccccc; padding-top: 10px;'>Mordor Technical Information</h2>
                    <p><a href='#systemreq'>System Requirements</a></p>
                    <p><a href='#troubleshoot'>General Troubleshooting</a></p>
                    <p><a href='#midi'>MIDI Mapper & Troubleshooting</a></p>
                    
                    <h2 style='margin-top: 20px; font-weight: bold; border-top: 1px solid #cccccc; padding-top: 10px;'>Mordor Tutorial</h2>
                    <p><a href='#tutorial'>Tutorial</a></p>
                </div>
            </div>
            
        </body>
        </html>
    )";
    return html;
}

// --- Slots/Signalhantering ---
void HelpLessonDialog::handleAnchorClicked(const QUrl &link)
{
    // I en riktig applikation skulle du ladda en ny sida baserat på "link.toString()"
    qDebug() << "Link clicked: " << link.toString();
    // Exempel: m_textEdit->scrollToAnchor(link.fragment()); // Om alla sektioner är på samma sida
}

void HelpLessonDialog::handleToolbarAction(QAction *action)
{
    // Här hanterar du klick på Toolbar-knapparna
    qDebug() << "Toolbar action triggered: " << action->text();
}
