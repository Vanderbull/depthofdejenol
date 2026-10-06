#include "NPCDialog.h"
#include <QtWidgets>
#include <QMessageBox>

QList<NPC> NPCDialog::allNPCs() {
    QList<NPC> npcs;

    auto make = [](const QString& name, const QString& role, const QString& personality,
                   const QString& greeting, const QStringList& hints, const QString& location) {
        NPC n;
        n.name = name;
        n.role = role;
        n.personality = personality;
        n.greeting = greeting;
        n.hints = hints;
        n.location = location;
        return n;
    };

    npcs.append(make("Guildmaster Borin", "Guildmaster",
                     "Grizzled and pragmatic. Respects strength and gold.",
                     "Another adventurer. The guild always has work for those who survive.",
                     {"Floor 5 has a guardian. Bring a full party and healing potions.",
                      "The deeper you go, the deadlier the monsters. Do not rush.",
                      "Bank your gold. Death in the dungeon costs more than you think."},
                     "Guild Hall"));

    npcs.append(make("Captain Rella", "Captain of the Guard",
                     "Blunt and no-nonsense. Hates goblins.",
                     "The city walls hold, but the roads do not. Watch your back.",
                     {"Goblins raid caravans on floor 3. They travel in packs.",
                      "Orcs on floor 10 hit like mules. Shield up.",
                      "Report kills to the quest board. The city pays."},
                     "Guard Barracks"));

    npcs.append(make("Priest Aldric", "Priest of Light",
                     "Calm and compassionate. Believes in second chances.",
                     "The light watches over you, even in the dark.",
                     {"Skeletons on floor 7 fear holy magic. Bring a priest.",
                      "The crypts are cursed. Cure poison before it kills you.",
                      "Resurrection is possible, but it is not cheap."},
                     "Temple"));

    npcs.append(make("Temple Acolyte Mira", "Temple Acolyte",
                     "Eager and nervous. New to the temple.",
                     "Blessings upon you. The moonpetal herbs grow on floor 2.",
                     {"Moonpetal herbs are needed for healing potions.",
                      "The temple accepts donations. Gold for blessings.",
                      "Evil creatures fear the light. Carry a holy symbol."},
                     "Temple"));

    npcs.append(make("Innkeeper Greta", "Innkeeper",
                     "Warm and gossipy. Knows everyone's business.",
                     "Welcome, dear. A warm bed and a cold beer await.",
                     {"Rest here to restore HP and mana. It costs gold, but it is worth it.",
                      "Cure poison and blindness before you delve. The dungeon is cruel.",
                      "Time passes quickly in the dark. Do not tarry."},
                     "Tavern"));

    npcs.append(make("Librarian Sage", "Librarian",
                     "Quiet and studious. Speaks in riddles.",
                     "Knowledge is power. The library holds many secrets.",
                     {"The bestiary records every monster you have faced.",
                      "Read the journals of fallen heroes. Learn from their mistakes.",
                      "The Prince of Devils waits on floor 15. Prepare well."},
                     "Library"));

    npcs.append(make("Seer Elara", "Seer",
                     "Cryptic and distant. Sees fragments of the future.",
                     "The threads of fate are tangled. I see... darkness and light.",
                     {"The gate can be closed, but the price is high.",
                      "The Prince is not the only danger. The dungeon itself is alive.",
                      "Trust your party. Alone, you will fall."},
                     "Seer's Tower"));

    npcs.append(make("Banker Lord Vex", "Banker",
                     "Cold and calculating. Gold is his only god.",
                     "Your gold is safe with us. For a fee.",
                     {"Deposit gold before you delve. The dungeon takes everything.",
                      "Interest accrues over time. Patience is profitable.",
                      "A dead adventurer's gold is forfeit. Do not die."},
                     "Royal Bank"));

    npcs.append(make("Blacksmith Dain", "Blacksmith",
                     "Gruff and proud. Respects craftsmanship.",
                     "Need steel? I forge the best in the city.",
                     {"Bronze is for beginners. Iron and steel for the serious.",
                      "Adamantite and mithril are found deep. Very deep.",
                      "A sharp blade saves lives. A dull one costs them."},
                     "Blacksmith"));

    npcs.append(make("Morgue Keeper Silas", "Morgue Keeper",
                     "Somber and matter-of-fact. Death is his business.",
                     "Another one for the cold room. What can I do for you?",
                     {"The dead can be resurrected, but the cost scales with level.",
                      "Bodies left in the dungeon can be recovered. For a price.",
                      "Hardcore mode means no resurrection. Choose wisely."},
                     "Morgue"));

    return npcs;
}

NPC NPCDialog::npcByName(const QString& name) {
    for (const NPC& n : allNPCs()) {
        if (n.name == name) return n;
    }
    NPC invalid;
    invalid.name = QString();
    return invalid;
}

QStringList NPCDialog::allHints() {
    QStringList hints;
    for (const NPC& n : allNPCs()) {
        hints.append(n.hints);
    }
    return hints;
}

QStringList NPCDialog::hintsFrom(const QString& name) {
    NPC n = npcByName(name);
    if (n.name.isEmpty()) return {};
    return n.hints;
}

// --- UI ---

NPCDialog::NPCDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    refreshList();
}

NPCDialog::~NPCDialog() = default;

void NPCDialog::setupUi() {
    setWindowTitle(tr("NPCs"));
    setMinimumSize(500, 400);

    auto *mainLayout = new QVBoxLayout(this);

    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(tr("NPCs:")));
    mainLayout->addLayout(filterLayout);

    m_npcList = new QListWidget;
    m_npcList->setMaximumWidth(200);
    mainLayout->addWidget(m_npcList);

    m_detailText = new QTextEdit;
    m_detailText->setReadOnly(true);
    mainLayout->addWidget(m_detailText);

    m_nameLabel = new QLabel;
    m_nameLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(m_nameLabel);

    m_roleLabel = new QLabel;
    mainLayout->addWidget(m_roleLabel);

    auto *btnLayout = new QHBoxLayout;
    m_talkBtn = new QPushButton(tr("Talk"));
    m_hintBtn = new QPushButton(tr("Hints"));
    m_exitBtn = new QPushButton(tr("Close"));
    btnLayout->addWidget(m_talkBtn);
    btnLayout->addWidget(m_hintBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_exitBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_npcList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *previous) {
                Q_UNUSED(previous);
                onNPCSelected(current);
            });
    connect(m_talkBtn, &QPushButton::clicked, this, &NPCDialog::onTalkClicked);
    connect(m_hintBtn, &QPushButton::clicked, this, &NPCDialog::onHintClicked);
    connect(m_exitBtn, &QPushButton::clicked, this, &NPCDialog::onExitClicked);
}

void NPCDialog::refreshList() {
    m_npcList->clear();
    for (const NPC& n : allNPCs()) {
        new QListWidgetItem(n.name, m_npcList);
    }
    if (m_npcList->count() > 0) {
        m_npcList->setCurrentRow(0);
    }
}

void NPCDialog::refreshDetail() {
    QListWidgetItem *item = m_npcList->currentItem();
    if (!item) return;
    NPC n = npcByName(item->text());
    if (n.name.isEmpty()) return;

    m_nameLabel->setText(n.name);
    m_roleLabel->setText(tr("Role: %1 — %2").arg(n.role, n.location));

    m_detailText->setHtml(tr("<b>%1</b><br><i>%2</i><br><br>"
                              "<b>Personality:</b> %3<br><br>"
                              "<b>Greeting:</b> \"%4\"")
        .arg(n.name, n.role, n.personality, n.greeting));
}

void NPCDialog::onNPCSelected(QListWidgetItem *item) {
    Q_UNUSED(item)
    refreshDetail();
}

void NPCDialog::onTalkClicked() {
    QListWidgetItem *item = m_npcList->currentItem();
    if (!item) return;
    NPC n = npcByName(item->text());
    if (n.name.isEmpty()) return;

    m_detailText->setHtml(tr("<b>%1</b><br><i>%2</i><br><br>"
                              "<b>Personality:</b> %3<br><br>"
                              "<b>Greeting:</b> \"%4\"")
        .arg(n.name, n.role, n.personality, n.greeting));
}

void NPCDialog::onHintClicked() {
    QListWidgetItem *item = m_npcList->currentItem();
    if (!item) return;
    NPC n = npcByName(item->text());
    if (n.name.isEmpty()) return;

    QStringList lines;
    for (const QString& hint : n.hints) {
        lines << tr("• %1").arg(hint);
    }

    m_detailText->setHtml(tr("<b>%1</b><br><i>%2</i><br><br>"
                              "<b>Dungeon Hints:</b><br>%3")
        .arg(n.name, n.role, lines.join("<br>")));
}

void NPCDialog::onExitClicked() {
    accept();
}
