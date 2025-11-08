#include "helplesson.h"
#include <QPushButton>
#include <QAction>
#include <QDebug>
#include <QTextDocument>
// Behövs för att använda QTextBrowser
#include <QTextBrowser> 
#include <QHBoxLayout>
#include <QVBoxLayout> // Se till att inkludera denna om den saknas i din fil

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

    // 2. Skapa och lägg till innehållsytan (Text)
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

// --- Innehållsytan (TextEdit) ---
QWidget* HelpLessonDialog::createContentArea()
{
    QWidget *container = new QWidget(this);
    // Vi använder fortfarande QHBoxLayout, men den kommer bara att innehålla QTextBrowser.
    QHBoxLayout *hLayout = new QHBoxLayout(container);
    // FIX: Justera vänster marginalen till 10px för stoppning, och ta bort bilden.
    hLayout->setContentsMargins(10, 10, 10, 10); 

    // Höger: Textinnehållet (QTextBrowser)
    m_textEdit = new QTextBrowser(this);
    m_textEdit->setReadOnly(true);
    // Ta bort kanten för att smälta in i dialogen
    m_textEdit->setFrameShape(QFrame::NoFrame);
    
    // Styla länkar för att matcha den gamla blå färgen
    m_textEdit->document()->setDefaultStyleSheet("a { color: blue; text-decoration: none; }");
    
    // Ladda in innehållet
    m_textEdit->setHtml(createHelpContentHtml());

    // Koppla klick på länkar.
    connect(m_textEdit, static_cast<void (QTextBrowser::*)(const QUrl&)>(&QTextBrowser::anchorClicked),
            this, &HelpLessonDialog::handleAnchorClicked);
    
    // Lägg till textinnehållet och låt det ta upp allt utrymme.
    hLayout->addWidget(m_textEdit, 1);
    
    // Dvärgbild-koden har tagits bort härifrån.
    
    return container;
}

// --- HTML-innehållet ---
QString HelpLessonDialog::createHelpContentHtml()
{
    // Återskapa stilen med HTML/CSS. Notera den mörkblå färgen för rubriken.
    // HTML-koden är densamma som i föregående steg, men ligger här för fullständighets skull.
    QString html = R"(
        <html>
        <body style='font-family: sans-serif; background-color: #f0f0f0; margin: 0; padding: 0;'>
            
            <div style='padding: 10px;'>
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
                    <p style='margin-left: 20px;'><a href='#banishspells'>Banish Spells</a></p>
                    <p style='margin-left: 20px;'><a href='#bindspells'>Bind Spells</a></p>
                    <p style='margin-left: 20px;'><a href='#charmspells'>Charm Spells</a></p>
                    <p style='margin-left: 20px;'><a href='#coldspells'>Cold Spells</a></p>
                    <p style='margin-left: 20px;'><a href='#electricalspells'>Electrical Spells</a></p>
                </div>

                <div style='width: 48%;'>
                    <p><a href='#parties'>Parties</a></p>
                    <p style='margin-left: 20px;'><a href='#companions'>Companions</a></p>
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
                    <p><a href='#automap'>AutoMap</a></p>
                </div>
            </div>
            
            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />
            
            <h2 id='automap' style='margin-top: 40px; font-weight: bold; color: #00008b;'>AutoMap</h2>
            
            <p>This neat feature keeps track of where your character(s) have been and gives a player access to a graphical map of exactly where they are (if the character isn't lost), what direction they're facing, and where they have been. The nice thing is that it's shared. If one character goes down and completely maps level 1, then all of the other characters in the game have access to the same Map. Mapping ability is based on a character's Perception ability (See Guild Statistics). As a character becomes more powerful, they will be able to map more completely, not missing little things. Once a square is properly mapped, it will not be "erased by another character with a lower perception ability.</p>

            <p>The map for a level is automatically updated as a character walks into unmapped areas. Newly mapped areas will have small black squares with a little gray around them to show that is where a character has recently mapped.</p>

            <h3 style='font-weight: bold;'>Getting Lost</h3>
            <p>If your character manages to get lost (by hitting a Teleporter, or something), the Automap will no longer reflect where you are. A lost character can find themself by either stepping back into explored territory or figuring out where they are (by using their perception ability).</p>

            <h3 style='font-weight: bold;'>Resizing the Automap</h3>
            <p>You can resize the Automap by dragging the corners or double-clicking on the map itself. When the Automap is smaller than the base size, the map will center itself in the window, allowing the area closest to the character to be seen.</p>

            <p>If the **Auto Change Level** option in the Characters Control Panel is set to true, the Automap will load each level and display it as the character moves between levels. If this flag is not set, the Automap will display the same level until another level is manually selected with the Scrollbar.</p>

            <h3 style='font-weight: bold;'>Redrawing the Automap</h3>
            <p>You may notice as you explore that the 'new' areas you step into are black squares with surrounding gray. This can sometimes cause other map items (i.e. fog, water, etc) to look strange. The reason for this 'grey mapping' is to show you where you most recently explored. If you wish to redraw the Automap (i.e. get rid of the explore squares), you can **Right Click** on the Automap, or switch to another level.</p>

            <h3 style='font-weight: bold;'>Changing Levels</h3>
            <p>You can either use the scroll bar located in the Automap window, or use the **<** and **>** keys from any other window in the game to page through the levels. The Automap will only allow levels that have been 'visited' to be accessible (i.e. if the deepest you have been is level 6, the Automap will not allow you to look at any levels deeper than 6 until somebody ventures down there!). You can access the AutoMap from the Dungeon by pressing **F8** or clicking on the **Map** command in the Dungeon Commands Window.</p>
            <p>**NOTE**: even though you may have the Automap Window minimized, it will still be updated as you are walking through the maze just as if it was open. This may cause the game to play slower on some systems. If this happens, be sure to close the Automap window completely to increase the game play speed.</p>

            <p>Move to the next section to read about Encounters.</p>
            <p>Move to the previous section to read about the Dungeon.</p>


            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />

            <h2 id='banishspells' style='margin-top: 40px; font-weight: bold; color: #00008b;'>Banish Spells</h2>

            <p>Taken from the art of demonology, spells of this kind are of ancient magic, of which little is known. Used to banish demons and devils back to the planes of Hell from whence they came, the amount of damage inflicted by these spells can sometimes be extensive. The effectiveness of these spells depends on the caster's experience and the Magic Resistance of the monster.</p>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 15%;'>Spell</th>
                    <th style='width: 15%;'>Base Level</th>
                    <th style='width: 15%;'>Guilds</th>
                    <th style='width: 25%;'>Required Stats</th>
                    <th style='width: 30%;'>Description</th>
                </tr>
                <tr>
                    <td>**Banish Devil**</td>
                    <td>5</td>
                    <td>Wiz (10), Hea</td>
                    <td>13 Int, 15 Wis, 10 Con</td>
                    <td>COMBAT Spell, will cause 8 Monsters in 4 Groups to be Banished. An ancient spell that invokes the fury of Hell to yank the specified target(s) back to the plane from whence they came.</td>
                </tr>
                <tr>
                    <td>**Banish Demon**</td>
                    <td>6</td>
                    <td>Wiz (12), Hea</td>
                    <td>12 Int, 14 Wis, 10 Con</td>
                    <td>COMBAT Spell, will cause 8 Monsters in 4 Groups to be Banished. A dark spell of old, much like Banish Devil, this spell calls upon Hades to take back those who have escaped.</td>
                </tr>
            </table>

            <p>Move to the next section to read about Dispell Spells.</p>
            <p>Move to the previous section to read about Movement Spells.</p>


            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />

            <h2 id='bindspells' style='margin-top: 40px; font-weight: bold; color: #00008b;'>Bind Spells</h2>

            <p>Once the **Charm** spells were mastered, the Mages guild then realized that once a creature had been befriended or dominated that a way to keep the creature under control was required. It was for this reason that the **Bind** class of spells was introduced - to reinforce a companion's binding to make it harder for them to break away from their master. Bind spells will not work on companions that have joined an adventurer free of will.</p>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 15%;'>Spell</th>
                    <th style='width: 15%;'>Base Level</th>
                    <th style='width: 15%;'>Guilds</th>
                    <th style='width: 25%;'>Required Stats</th>
                    <th style='width: 30%;'>Description</th>
                </tr>
                <tr>
                    <td>**Control**</td>
                    <td>3</td>
                    <td>Mag</td>
                    <td>12 Int, 12 Wis, 10 Cha</td>
                    <td>NON-COMBAT Spell, will increase the Bind level of a companion. This spell can be used to reinforce the Bind that is already imposed on a companion. Note: This spell will not function on companions that have joined you of their own free will.</td>
                </tr>
                <tr>
                    <td>**Bind**</td>
                    <td>6</td>
                    <td>Mag</td>
                    <td>14 Int, 14 Wis, 16 Cha</td>
                    <td>NON-COMBAT Spell, will increase the binding level of a bound companion. This spell is a more powerful version of Control that will renew the Bind on a companion when it begins to fade.</td>
                </tr>
                <tr>
                    <td>**Domination**</td>
                    <td>12</td>
                    <td>Mag</td>
                    <td>18 Int, 18 Wis, 18 Con, 18 Cha</td>
                    <td>NON-COMBAT Spell, will increase the binding level of a bound companion. This is the most powerful of the Binding spells, and in the hands of a master Mage, almost no creature can resist it.</td>
                </tr>
            </table>

            <p>Move to the next section to read about Heal Spells.</p>
            <p>Move to the previous section to read about Charm Spells.</p>


            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />

            <h2 id='characterinfo' style='margin-top: 40px; font-weight: bold; color: #00008b;'>Character Information</h2>

            <p>To play Mordor, you must first create a character (your Mordor alter-ego). Each character requires several basic pieces of information that the game uses in various ways.</p>

            <ul>
                <li>**Name**: The name is used whenever another character wishes to refer to you specifically, such as when giving gold or items, or when healing. Choose your name well since it cannot ever be changed again, and only one character in the game can have a given name at a time.</li>
                <li>**Race**: Your chosen race determines your maximum age, how quickly you learn (gain experience), and the potential maximum values for your Strength, Intelligence, Wisdom, Constitution, Charisma, and Dexterity. It also determines the number of initial stat points you have when creating your character.</li>
            </ul>

            <h3 style='font-weight: bold;'>Stats</h3>
            <p>There are six basic statistics (**stats**) that determine how well or poorly your character will do when faced with certain tasks (e.g., fighting, casting spells, etc.). Some Items and Spells, as well as some guilds, may require a minimum stat to use, cast, or join.</p>

            <h4 style='font-weight: bold; margin-top: 10px;'>Natural Stats vs. Modified Stats</h4>
            <p>Certain items found in the dungeon may modify a stat while equipped, but the unmodified (**natural**) stat is still used in some circumstances, such as for guild joining requirements, equipping/using an item, and calculating maximum spell points. You may raise your natural stats by using certain items found in the dungeon. When a stat has reached its natural peak, it will appear in bold type on the screen.</p>

            <p>The six basic statistics and their effects are:</p>

            <ul>
                <li>**Strength (Str)**: Determines your chances of hitting a monster in combat and how much damage is done when you hit a monster.</li>
                <li>**Intelligence (Int)**: Determines your chances of hitting a monster in combat, the maximum spell points allowed, your ability to identify an item, and your ability to identify a monster.</li>
                <li>**Wisdom (Wis)**: Represents common sense and insight. It determines your chances of hitting a monster in combat, the maximum spell points allowed, your ability to recognize a trap on a chest, your ability to identify an item, your ability to recognize a monster, and your chances of leaving combat without being hit.</li>
                <li>**Constitution (Con)**: Represents your "healthiness". It determines the number of hits you receive when you make a level, the chance of being raised when dead, and your ability to resist special attacks from monsters (poison, disease, stoning, aging, paralysis, 'breath' attacks, and spells cast by monsters).</li>
                <li>**Charisma (Cha)**: Represents beauty and leadership. It determines your chances of having monsters join and become your companions, how long a companion will stay with you, and the chance of monsters becoming hostile towards you.</li>
                <li>**Dexterity (Dex)**: Represents speed and agility. It determines your chance of hitting a monster in combat, your ability to disarm traps, how long it takes your character to open a chest while in combat, the chances a monster has of stealing from you, your chances of leaving combat without being hit, and your chances of surprising monsters.</li>
            </ul>

            <h3 style='font-weight: bold;'>Attack/Defense (Att/Def)</h3>
            <p>These numbers vary with what items you are wearing, your statistics, your character's fighting ability, and experience level. Some spells will modify your Att/Def, making your character harder to hit. The Att/Def is a percentage representation of how well you do in combat. For example, **9/12** means you have a **9% additional chance of hitting** a monster, while the monster has an additional **12% chance of missing you**. As you head deeper into the dungeon, monsters will be more experienced and have a higher Att/Def.</p>

            <h3 style='font-weight: bold;'>Hits</h3>
            <p>Hits determine how much abuse and damage your character can take before dying. They depend heavily on the level in your strongest guild and your Constitution.</p>

            <ul>
                <li>You can receive "bonus" hits when you make a level if your Constitution is above 16 (and you have not yet reached Maximum Level in the current guild you are receiving hits from).</li>
                <li>When your hits reach **0**, you are dead, but can be raised by another character or in the morgue.</li>
                <li>Hits are expressed in a fraction (e.g., **12/12** is full strength, **6/12** is half strength). The first number is current hits, and the second is maximum hits.</li>
                <li>You can raise your hits by achieving higher guild levels or having special events occur. Some guilds (e.g., Healers) have the ability to cast spells and heal lost hits.</li>
            </ul>

            <h3 style='font-weight: bold;'>Experience</h3>
            <p>Experience is gained from fighting, casting spells, opening chests, etc.. When you have reached a certain amount of experience, your guild will allow you to "make level," which increases your character's powers. Both your **race** and **guild** will affect how much experience is required to make a level in your current guild.</p>

            <h3 style='font-weight: bold;'>Gold</h3>
            <p>Gold is practically weightless, so there is no limit to how much your character can carry. Gold is essential for buying items, making levels in guilds, and raising dead characters or companions. It's recommended to use the **bank** to store gold, as some monsters can steal it, sometimes in great amounts.</p>

            <h3 style='font-weight: bold;'>Spell Points & Spell Level</h3>
            <p>Every character has a certain number of **Spell Points** that they can use for casting spells. As a character's Intelligence and Wisdom go up, so does the maximum number of spell points. Like Hit Points, Spell Points are expressed in a fraction showing current and maximum points.</p>

            <ul>
                <li>Every character also has a **'spell level,'** which is related to their current Guild and Guild Level.</li>
                <li>The power of a spell is based on the character's spell level, which goes up as a character raises levels in their guild(s).</li>
            </ul>

            <p>Move to the next section to read about Races.</p>
            <p>Move to the previous section to read the Game Options Window.</p>


            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />

            <h2 id='charmspells' style='margin-top: 40px; font-weight: bold; color: #00008b;'>Charm Spells</h2>

            <p>The pride of the Mages guild, these spells can control certain types of creatures (from Humanoids to Dragons), causing them to befriend the spellcaster and become their **companion**. The effectiveness depends on the caster's experience and the Magic and Charm Resistance of the monster.</p>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 15%;'>Spell</th>
                    <th style='width: 15%;'>Base Level</th>
                    <th style='width: 15%;'>Guilds</th>
                    <th style='width: 25%;'>Required Stats</th>
                    <th style='width: 30%;'>Description</th>
                </tr>
                <tr>
                    <td>**Charm Insect**</td>
                    <td>1</td>
                    <td>Mag</td>
                    <td>12 Int, 12 Wis</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Binds one or more insects to the caster.</td>
                </tr>
                <tr>
                    <td>**Charm Animal**</td>
                    <td>2</td>
                    <td>Mag</td>
                    <td>12 Int, 12 Wis, 14 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Excellent for charming vicious animals.</td>
                </tr>
                <tr>
                    <td>**Charm Elemental**</td>
                    <td>3</td>
                    <td>Mag</td>
                    <td>13 Int, 13 Wis, 15 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Causes a group of Elementals to join the caster.</td>
                </tr>
                <tr>
                    <td>**Charm Humanoid**</td>
                    <td>4</td>
                    <td>Mag</td>
                    <td>16 Int, 16 Wis, 18 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Can befriend almost any humanoid, depending on caster power and target resistance.</td>
                </tr>
                <tr>
                    <td>**Charm Undead**</td>
                    <td>5</td>
                    <td>Mag</td>
                    <td>15 Int, 17 Wis, 10 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Attempts to magically bind the Undead to the caster.</td>
                </tr>
                <tr>
                    <td>**Charm Giant**</td>
                    <td>5</td>
                    <td>Mag</td>
                    <td>15 Int, 15 Wis, 16 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Advanced version of Charm Humanoid, affecting only Giants.</td>
                </tr>
                <tr>
                    <td>**Bind Animal**</td>
                    <td>5</td>
                    <td>Mag</td>
                    <td>13 Int, 17 Wis, 10 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. More advanced version of Charm Animal.</td>
                </tr>
                <tr>
                    <td>**Charm Monster**</td>
                    <td>6</td>
                    <td>Mag</td>
                    <td>12 Int, 17 Wis, 10 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Attempts to Charm creatures that no other charm-specific spell will work on.</td>
                </tr>
                <tr>
                    <td>**Bind Elemental**</td>
                    <td>6</td>
                    <td>Mag</td>
                    <td>15 Int, 15 Wis, 12 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. More advanced version of Charm Elemental, Binding more powerful creatures.</td>
                </tr>
                <tr>
                    <td>**Charm Dragon**</td>
                    <td>7</td>
                    <td>Mag</td>
                    <td>18 Int, 18 Wis, 18 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Attempts to bind one or more dragons to the caster; failure may result in adverse response.</td>
                </tr>
                <tr>
                    <td>**Dominate Demon**</td>
                    <td>8</td>
                    <td>Mag</td>
                    <td>15 Int, 17 Wis, 12 Con, 17 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Uses earth powers to bind a Demon for an unknown period.</td>
                </tr>
                <tr>
                    <td>**Dominate Devil**</td>
                    <td>8</td>
                    <td>Mag</td>
                    <td>15 Int, 18 Wis, 12 Con, 18 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Uses dark elements powers to bind one or more devils.</td>
                </tr>
                <tr>
                    <td>**Dominate Humanoid**</td>
                    <td>8</td>
                    <td>Mag</td>
                    <td>16 Int, 16 Wis, 16 Cha</td>
                    <td>COMBAT Spell, will cause 4 humanoids in 1 Group to be Charmed. Puts humanoids well under the Charm of the caster and is among the most difficult to break.</td>
                </tr>
                <tr>
                    <td>**Dominate Giant**</td>
                    <td>10</td>
                    <td>Mag</td>
                    <td>18 Int, 18 Wis, 18 Cha</td>
                    <td>COMBAT Spell, will cause 4 giants in 1 Group to be Charmed. Affects giants with the potency of the Dominate class of spells.</td>
                </tr>
                <tr>
                    <td>**Soul Entrapment**</td>
                    <td>12</td>
                    <td>Mag</td>
                    <td>18 Int, 21 Wis, 18 Con, 22 Cha</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 1 Group to be Charmed. Most advanced charm spell, only a master Mage can cast this to Charm any creature, living or not.</td>
                </tr>
            </table>

            <p>Move to the next section to read about Bind Spells.</p>
            <p>Move to the previous section to read about Kill Spells.</p>


            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />

            <h2 id='coldspells' style='margin-top: 40px; font-weight: bold; color: #00008b;'>Cold Spells</h2>

            <p>After mastering Fire, the Sorcerer's guild mastered the element of coldness to control both Fire and Cold. The effectiveness of these spells depends on the experience of the caster and the **Cold Resistance** of the target.</p>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 15%;'>Spell</th>
                    <th style='width: 15%;'>Base Level</th>
                    <th style='width: 15%;'>Guilds</th>
                    <th style='width: 25%;'>Required Stats</th>
                    <th style='width: 30%;'>Description</th>
                </tr>
                <tr>
                    <td>**Cold Blast**</td>
                    <td>2</td>
                    <td>Sor</td>
                    <td>12 Int, 9 Wis, 9 Dex</td>
                    <td>COMBAT Spell, will cause 4 Monsters in 2 Groups to be Frozen. Creates a blast of air cold enough to kill most small creatures.</td>
                </tr>
                <tr>
                    <td>**Ice Spray**</td>
                    <td>5</td>
                    <td>Sor</td>
                    <td>16 Int, 8 Wis, 9 Dex</td>
                    <td>COMBAT Spell, will cause 6 Monsters in 3 Groups to be Frozen. Creates a wall of ice that blasts towards opponents, likely freezing the victim instantly.</td>
                </tr>
                <tr>
                    <td>**Hail Storm**</td>
                    <td>8</td>
                    <td>Sor</td>
                    <td>19 Int, 18 Wis, 12 Dex</td>
                    <td>COMBAT Spell, will cause 8 Monsters in 4 Groups to be Frozen. Creates a storm of magical hail that crushes and smothers victims, attempting to freeze and shatter them.</td>
                </tr>
                <tr>
                    <td>**Arctic Storm**</td>
                    <td>12</td>
                    <td>Sor</td>
                    <td>21 Int, 20 Wis, 18 Dex</td>
                    <td>COMBAT Spell, will cause 12 Monsters in 4 Groups to be Frozen. The most feared spell by fire creatures, this storm is an intense collection of other cold spells that few can survive.</td>
                </tr>
            </table>

            <p>Move to the next section to read about Electrical Spells.</p>
            <p>Move to the previous section to read about Fire Spells.</p>


            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />

            <h2 id='commands' style='margin-top: 40px; font-weight: bold; color: #00008b;'>Commands</h2>

            <p>Commands are broken into two sections: **Game Commands** (work in the City and Dungeon anytime) and **Dungeon Commands** (work only in the dungeon, specified as Combat or Non-Combat).</p>

            <h3 style='font-weight: bold;'>Game Commands (Function Keys and Single Keys)</h3>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 15%;'>Key</th>
                    <th style='width: 85%;'>Command</th>
                </tr>
                <tr><td>**F1**</td><td>HelpLesson</td></tr>
                <tr><td>**F2**</td><td>Area Look (User List of Area)</td></tr>
                <tr><td>**F3**</td><td>Character Stats Information</td></tr>
                <tr><td>**F4**</td><td>Resistance Information</td></tr>
                <tr><td>**F5**</td><td>Experience Information</td></tr>
                <tr><td>**F6**</td><td>Character Information</td></tr>
                <tr><td>**F7**</td><td>Guild/Abilities Information</td></tr>
                <tr><td>**F9**</td><td>Buffer Information</td></tr>
                <tr><td>**F10**</td><td>Game Options</td></tr>
                <tr><td>**F11**</td><td>Toolbar Toggle</td></tr>
                <tr><td>**a**</td><td>Ask character to Join</td></tr>
                <tr><td>**e**</td><td>Equip Item (currently selected item)</td></tr>
                <tr><td>**d**</td><td>Drop Item (currently selected item)</td></tr>
                <tr><td>**i**</td><td>Item Info (currently selected item)</td></tr>
                <tr><td>**g**</td><td>Give Item/Gold</td></tr>
                <tr><td>**p**</td><td>Pickup Body</td></tr>
                <tr><td>**l**</td><td>Drop Body</td></tr>
                <tr><td>**s**</td><td>Swap Items</td></tr>
                <tr><td>**ESC**</td><td>Backout (of a Dialog Box, Text Box, etc.)</td></tr>
                <tr><td>**SHIFT-F6**</td><td>Outputs current character stats, statistics and information to CHARSTAT.TXT (in the Root Mordor directory)</td></tr>
                <tr><td>**CTRL-&lt;letter&gt;**</td><td>Switch to any Character Window Tab or Item Window tab anytime (e.g., Ctrl-P for Spells Tab)</td></tr>
            </table>

            <h4 style='font-weight: bold; margin-top: 10px;'>Tab Keys (use CTRL key to access from any window)</h4>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 40%;'>Window</th>
                    <th style='width: 20%;'>Key</th>
                    <th style='width: 40%;'>Tab</th>
                </tr>
                <tr><td>**Character Window**</td><td>**CTRL-l**</td><td>Look in Room Tab</td></tr>
                <tr><td></td><td>**CTRL-b**</td><td>Buffers Tab</td></tr>
                <tr><td></td><td>**CTRL-m**</td><td>Miscellaneous Tab</td></tr>
                <tr><td></td><td>**CTRL-s**</td><td>Character Stats Tab</td></tr>
                <tr><td></td><td>**CTRL-r**</td><td>Resistances Tab</td></tr>
                <tr><td></td><td>**CTRL-c**</td><td>Character Info Tab</td></tr>
                <tr><td></td><td>**CTRL-g**</td><td>Guild Info Tab</td></tr>
                <tr><td>**Objects & Spells Window**</td><td>**CTRL-i**</td><td>Items Tab</td></tr>
                <tr><td></td><td>**CTRL-p**</td><td>Spells Tab</td></tr>
                <tr><td></td><td>**CTRL-o**</td><td>Companions Tab</td></tr>
            </table>

            <h4 style='font-weight: bold; margin-top: 10px;'>Party Keys</h4>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 15%;'>Key</th>
                    <th style='width: 85%;'>Command</th>
                </tr>
                <tr><td>**n**</td><td>Select next Character</td></tr>
                <tr><td>**ALT-s**</td><td>Switch to Character (updates all windows)</td></tr>
                <tr><td>**ALT-o**</td><td>Party Options</td></tr>
                <tr><td>**ALT-l**</td><td>Ask character to leave</td></tr>
                <tr><td>**ALT-1-4**</td><td>Switch to character</td></tr>
                <tr><td>**ALT-p**</td><td>Pool party's on-hand gold to current character</td></tr>
            </table>

            <h4 style='font-weight: bold; margin-top: 10px;'>Automap Keys</h4>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 15%;'>Key</th>
                    <th style='width: 85%;'>Command</th>
                </tr>
                <tr><td>**F8**</td><td>View AutoMap</td></tr>
                <tr><td>**<**</td><td>Display Previous Automap Level</td></tr>
                <tr><td>**>**</td><td>Display Next Automap Level</td></tr>
            </table>

            <h3 style='font-weight: bold;'>Dungeon Commands</h3>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 10%;'>Key</th>
                    <th style='width: 50%;'>Command</th>
                    <th style='width: 40%;'>Combat Type</th>
                </tr>
                <tr><td>**f**</td><td>Fight</td><td>Combat</td></tr>
                <tr><td>**d**</td><td>Drop</td><td>Non-Combat</td></tr>
                <tr><td>**d**</td><td>Defend</td><td>Combat</td></tr>
                <tr><td>**c**</td><td>Cast Spell</td><td>Non/Combat</td></tr>
                <tr><td>**o**</td><td>Open Chest</td><td>Non/Combat</td></tr>
                <tr><td>**1-4**</td><td>Select Monster Group</td><td>Combat</td></tr>
                <tr><td>**U**</td><td>Use Item (currently sel. item)</td><td>Non/Combat</td></tr>
                <tr><td>**t**</td><td>Take Stairs, Chute, etc.</td><td>Non/Combat</td></tr>
                <tr><td>**j**</td><td>Allow monster(s) to join</td><td>Combat</td></tr>
                <tr><td>**<space>**</td><td>Stop fighting, opening, etc.</td><td>Combat</td></tr>
                <tr><td>**Shift 0-9**</td><td>Cast Spell or Use Item in Buffer</td><td>Non/Combat</td></tr>
            </table>

            <p>**Note on ESC in Combat:** The ESC key is for backing out of selections made during combat; for example, hitting `c` and then `ESC` will return focus to the Main Dungeon Window. Using the buffers (`Shift 0-9`) is highly recommended for speeding up spell casting and item use during combat.</p>

            <h3 style='font-weight: bold;'>Click Commands (on the Main Dungeon Window)</h3>

            <table border='1' cellpadding='5' cellspacing='0' style='width: 100%; border-collapse: collapse;'>
                <tr style='background-color: #e9e9e9;'>
                    <th style='width: 25%;'>Action</th>
                    <th style='width: 75%;'>Command</th>
                </tr>
                <tr><td>**Click** Monster Label</td><td>Same as **1-4**</td></tr>
                <tr><td>**Click** Companion Name Label</td><td>Name a Companion</td></tr>
                <tr><td>**Double Click** Automap</td><td>Either Resets or Resizes the Automap</td></tr>
                <tr><td>**Double Click** Party Character</td><td>Switch all windows to Character</td></tr>
                <tr><td>**Double Click** Companion Picture</td><td>Get Companion Stats</td></tr>
                <tr><td>**Double Click** Item in Items Tab</td><td>Use the Item</td></tr>
                <tr><td>**Double Click** Spell in Spell Book</td><td>Cast the Spell</td></tr>
                <tr><td>**Double Click** Buffer Entry</td><td>Cast Spell/Use Item in Buffer</td></tr>
                <tr><td>**Right Click** Companion Picture</td><td>Attack Companion</td></tr>
                <tr><td>**Right Click** Automap</td><td>Redraw/Refresh Level</td></tr>
            </table>

            <p>Move to the next section to read the Control Panel.</p>
            <p>Move to the previous section to read about Parties.</p>


            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />

            <h2 id='companions' style='margin-top: 40px; font-weight: bold; color: #00008b;'>Companions</h2>

            <p>Companions are very important when fighting; a solo character is always the sole target of every monster, while a character with companions can split the attackers' attention and use the companions' abilities against the enemy. If a companion kills a monster your character is quested for, you get full credit for the kill.</p>

            <h3 style='font-weight: bold;'>Disadvantages of Companions:</h3>
            <ul>
                <li>Some companions can be greedy when sharing treasure.</li>
                <li>There's a chance a bound companion may turn on its master.</li>
                <li>If two or more characters form a party and each has companions, the companions can become disoriented and attack a party member.</li>
            </ul>

            <p>You can find current information on a companion by **double-clicking** its picture in the **Companions Tab** of the **Objects & Spells window**. Companions can be sold by dragging and dropping their picture into the store. They cannot be traded between characters, and only a companion's master can heal or resurrect them.</p>

            <h3 style='font-weight: bold;'>Charming & Binding Companions</h3>
            <p>Unless a creature offers to join, companions are acquired using magic (a **Charm** spell or an Item) that binds the creature against its will.</p>
            <ul>
                <li>The initial spell binds the creature at a certain level.</li>
                <li>Depending on this level and the character's level, the monster may break free and either leave or turn hostile.</li>
                <li>A lower bind level increases the chance of the creature breaking free and weakening the binding.</li>
                <li>Low-level characters cannot easily regulate high-level monsters. The power of regulation comes with experience and guild levels.</li>
                <li>If the binding level reaches **0**, the creature will always break free.</li>
            </ul>

            <h3 style='font-weight: bold;'>Increasing Bind Levels</h3>
            <p>A character can fortify a companion's bind level by:</p>
            <ol>
                <li>Casting a **Bind** spell (Mages can cast Control, Bind, and Domination).</li>
                <li>Using an Item.</li>
                <li>Using the Bind services in the **Holding & Confinement** store.</li>
            </ol>
            <p>A highly bound companion with an experienced character is very unlikely to break free.</p>

            <h3 style='font-weight: bold;'>Combat & Companions</h3>
            <p>Having four companions for a single character is good, but in a party with multiple characters, confusion can lead to companions attacking party members or their companions, especially with powerful magic users and fire-breathing creatures.</p>
            <ul>
                <li>The chance of a companion mistaking a party member for an enemy is based on the companion's experience level (creatures from upper levels are less experienced at this and hit party members more often).</li>
                <li>The more "bodies" in combat, the better the chance of a mistake.</li>
                <li>It's recommended that the total 'body' count for an adventuring group never exceeds **6**.</li>
            </ul>

            <h3 style='font-weight: bold;'>Handling Companions</h3>
            <ul>
                <li>**Dedicated Companions**: Monsters that offer to join are about 50% dedicated to staying, provided the character heals and tends to them. The other 50% may leave when they get bored or too beat up.</li>
                <li>**Voluntary Companions**: Monsters that join free of will cannot be re-bound and will not attack the character if they leave.</li>
                <li>**Surprise Attacks**: If a bound companion breaks free, they have a good chance of turning hostile (often taking other weakly bound companions with them). This is dependent on the creature's alignment versus the character's alignment (oppositely aligned creatures always attack) and whether the creature is afraid of the character. Evil creatures tend to always attack when breaking free.</li>
                <li>**Warning**: Do not take on a companion significantly more powerful than yourself, as they could break free and surprise attack the party.</li>
                <li>**Attacking a Companion**: One can attack their companion by **right-clicking** on the companion's picture.</li>
                <li>**Naming Companions**: You can assign a name to a companion by **clicking on the label** that holds the companion's name.</li>
                <li>**Learning about Companions**: Having a monster as a companion allows the character to learn about its abilities, strengths, and weaknesses. A character with a good ID skill (combined Intelligence, Wisdom, and Guild Level) can deduce almost all information, which can be used to their advantage if they later fight that monster type. Characters with a low ID skill will need to use the ID features of the **Holding & Confinement** store.</li>
                <li>**Healing & Resurrecting**: Companions can be healed by casting a heal spell or using an item that casts a spell. Characters cannot heal the companions of other party members. A hurt companion can break a binding level much easier or become restless. Companions can be raised from the dead in the dungeon with standard resurrect spells.</li>
                <li>**Moving & Reorganizing**: Companions can be moved by dragging and dropping their picture onto another companion slot, allowing players to place stronger companions in front and weaker (or spell-casting) companions in the back.</li>
                <li>**Selling Companions**: Companions can be sold in the city for profit at **Holding and Confinement**, which pays handsome prices for powerful creatures. More powerful creatures are harder to contain and regulate.</li>
                <li>**Library Identification**: The only two ways to get a completely identified monster in the library are to have it as a companion and study it, or take the creature to the **Holding & Confinement** store for identification.</li>
                <li>**Casting**: Only one spell-casting companion will be able to cast a spell each round.</li>
                <li>Companions can be obtained by casting a "charm" spell, using an item that casts a "charm" spell, or allowing monsters to join if they offer.</li>
            </ul>

            <p>Move to the next section to read about Dying.</p>
            <p>Move to the previous section to read about Monsters and Treasure.</p>


            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />

            <h2 id='controlpanel' style='margin-top: 40px; font-weight: bold; color: #00008b;'>Control Panel</h2>

            <p>A character can access the Control Panel by Pressing **F10** in either the City or Dungeon. From here, various options can be set.</p>

            <h3 style='font-weight: bold;'>Available Options</h3>

            <ul>
                <li>**Hide**: Setting this to **True** prevents other characters from picking you up and carrying you anywhere.</li>
                <li>**Preload Character**: If selected, the character will be automatically loaded (without password verification) the next time Mordor is started.</li>
                <li>**No Perception Msg.**: Disables any perception messages (e.g., "You are pretty sure you are standing on an Anti-Magic square") while walking in the dungeon.</li>
                <li>**Auto Change Lev.**: Turning this on causes the Automap to automatically update to reflect the character's current level (if the character is not lost).</li>
                <li>**No Party Sound**: Disables all background combat sounds from other characters and companions in the party.</li>
            </ul>

            <h3 style='font-weight: bold;'>Available Functions</h3>

            <ul>
                <li>**Window Control**: Allows a player to reset the size of one or all windows back to the original size. The **Adjust to Display** button can resize (adjust) windows to match the current resolution.</li>
                <li>**Game Options**: Displays the Game Options screen, allowing the user to set volume, etc.</li>
            </ul>

            <p>Move to the next section to read the Hints & Tips.</p>
            <p>Move to the previous section to read about Commands.</p>


            <hr style='border: none; border-top: 3px double #333333; margin-top: 40px;' />

            <h2 id='createchar' style='margin-top: 40px; font-weight: bold; color: #00008b;'>Creating a Character</h2>

            <p>When you select **Create Character** from the Main Menu, the character creation window will appear. To save a Character, you must select a **Name** and a **Race**.</p>

            <ul>
                <li>**Statistics**: On the right-hand side are your character's statistics, which you can modify by clicking on the adjuster (up and down) arrows. The amount of extra "points" you have to disperse to your stats are shown below the stats.</li>
                <li>**Guilds**: On the right side of the window is a section showing the guilds the character can currently join with the selected Race.
                    <ul>
                        <li>**Bold** guilds: Character can join given the selected stats, race, and alignment.</li>
                        <li>Non-bold guilds: Selected race and alignment can join, but not with your current stats.</li>
                        <li>**Italic** guilds: Selected race can join, but not with your selected alignment (Good, Neutral, or Evil).</li>
                    </ul>
                </li>
            </ul>

            <p>If the stat requirements for a guild are too high to select initially, you can create the character as a Nomad (or another guild) and run around to find enough items to raise your stats to the minimum requirements.</p>

            <p>Once you've chosen your Name, Race, Alignment, Sex, and Stats, you can save your character and start playing.</p>
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
