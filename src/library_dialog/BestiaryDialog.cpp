#include "BestiaryDialog.h"
#include "gameStateManager.h"
#include "character.h"
#include <QtWidgets>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPixmap>
#include <QSet>

namespace {

// One load per process; the dialog is opened repeatedly and the file is a
// few hundred KB.
QList<QVariantMap> loadBestiary()
{
    QList<QVariantMap> out;

    // Same search order the spell data uses: cwd, then next to the binary,
    // then one level up (build/bin -> project root).
    const QStringList candidates = {
        QStringLiteral("data/bestiary.json"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/data/bestiary.json"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../data/bestiary.json"),
        QStringLiteral("../data/bestiary.json"),
    };

    for (const QString& path : candidates) {
        QFile file(path);
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) continue;

        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (!doc.isObject()) continue;

        const QJsonArray arr = doc.object().value(QStringLiteral("monsters")).toArray();
        for (const QJsonValue& v : arr) {
            const QJsonObject o = v.toObject();
            QVariantMap m;
            m.insert("name", o.value("name").toString());
            m.insert("id", o.value("id").toInt());
            m.insert("category", o.value("category").toString());
            m.insert("picture", o.value("picture").toInt());
            m.insert("image", o.value("image").toString());
            m.insert("hits", o.value("hits").toInt());
            m.insert("att", o.value("att").toInt());
            m.insert("def", o.value("def").toInt());
            m.insert("level", o.value("level").toInt());
            m.insert("groups", o.value("groups").toInt());
            m.insert("goldFactor", o.value("goldFactor").toInt());
            m.insert("alignment", o.value("alignment").toInt());
            m.insert("type", o.value("type").toInt());
            m.insert("subtype", o.value("subtype").toInt());
            m.insert("size", o.value("size").toString());
            m.insert("abilitiesText", o.value("abilitiesText").toString());
            m.insert("walkthroughHits", o.value("walkthroughHits").toString());
            m.insert("walkthroughAD", o.value("walkthroughAD").toString());
            m.insert("walkthroughGroup", o.value("walkthroughGroup").toString());

            // Resistances arrive as [{name, percent}, ...]; keep only the
            // non-zero ones and flatten to "Fire 25%" strings for display.
            QStringList res;
            const QJsonArray resArr = o.value("resistances").toArray();
            for (const QJsonValue& rv : resArr) {
                const QJsonObject ro = rv.toObject();
                const int pct = ro.value("percent").toInt();
                if (pct > 0) {
                    res << QStringLiteral("%1 %2%")
                               .arg(ro.value("name").toString())
                               .arg(pct);
                }
            }
            m.insert("resistances", res);

            QStringList walkRes;
            const QJsonArray walkArr = o.value("walkthroughResistances").toArray();
            for (const QJsonValue& rv : walkArr) {
                const QString s = rv.toString().trimmed();
                if (!s.isEmpty() && s.compare(QStringLiteral("None"),
                                              Qt::CaseInsensitive) != 0) {
                    walkRes << s;
                }
            }
            m.insert("walkthroughResistances", walkRes);

            // Stats arrive as {Strength: n, ...}; keep only the non-zero.
            QStringList stats;
            const QJsonObject st = o.value("stats").toObject();
            for (const QString& key : st.keys()) {
                const int v = st.value(key).toInt();
                if (v > 0) stats << QStringLiteral("%1 %2").arg(key).arg(v);
            }
            m.insert("stats", stats);

            if (!m.value("name").toString().isEmpty()) out.append(m);
        }
        if (!out.isEmpty()) break;
    }
    return out;
}

const QList<QVariantMap>& cached()
{
    static const QList<QVariantMap> data = loadBestiary();
    return data;
}

} // namespace

BestiaryDialog::BestiaryDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    refreshList();
}

BestiaryDialog::~BestiaryDialog() = default;

// ---------------------------------------------------------------- data access

const QList<QVariantMap>& BestiaryDialog::entries() {
    return cached();
}

QStringList BestiaryDialog::categories() {
    QStringList out;
    for (const QVariantMap& m : cached()) {
        const QString c = m.value("category").toString();
        if (!c.isEmpty() && !out.contains(c)) out << c;
    }
    out.sort();
    return out;
}

QVariantMap BestiaryDialog::entry(const QString& name) {
    for (const QVariantMap& m : cached()) {
        if (m.value("name").toString() == name) return m;
    }
    return {};
}

// ------------------------------------------------------- encounter tracking
//
// The party has to actually meet a monster before the bestiary describes it.
// The set lives here rather than in the dialog so it survives the dialog being
// closed and reopened, and so the self-tests can drive it directly.

namespace {
QSet<QString>& encounteredSet()
{
    static QSet<QString> s;
    return s;
}
} // namespace

void BestiaryDialog::recordEncounter(const QString& name) {
    if (!name.isEmpty()) encounteredSet().insert(name);
}

bool BestiaryDialog::isEncountered(const QString& name) {
    return encounteredSet().contains(name);
}

int BestiaryDialog::encounteredCount() {
    return encounteredSet().size();
}

void BestiaryDialog::resetEncounters() {
    encounteredSet().clear();
}

QString BestiaryDialog::displayName(const QString& name) {
    return isEncountered(name) ? name : QStringLiteral("???");
}

QString BestiaryDialog::imagePath(const QVariantMap& entry) {
    const QString rel = entry.value("image").toString();
    if (rel.isEmpty()) return {};

    // The dialog may run from the project root or from build/bin, so try the
    // stored path and the same path relative to the binary's directory.
    const QStringList candidates = {
        rel,
        QCoreApplication::applicationDirPath() + QLatin1Char('/') + rel,
        QCoreApplication::applicationDirPath() + QStringLiteral("/../") + rel,
        QStringLiteral("../") + rel,
    };
    for (const QString& p : candidates) {
        if (QFile::exists(p)) return p;
    }
    return rel;  // let QPixmap try; isNull() tells the caller it failed
}

// ----------------------------------------------------------------------- UI

void BestiaryDialog::setupUi() {
    setWindowTitle(tr("Bestiary"));
    setMinimumSize(820, 560);

    auto *mainLayout = new QVBoxLayout(this);

    // --- Filter row: category, floor range and a name search.
    auto *filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(tr("Filter:")));

    m_filterCombo = new QComboBox;
    m_filterCombo->addItem(tr("All"));
    m_filterCombo->addItem(tr("Floor 1-5"));
    m_filterCombo->addItem(tr("Floor 6-10"));
    m_filterCombo->addItem(tr("Floor 11-15"));
    m_filterCombo->addItem(tr("Floor 16+"));
    filterLayout->addWidget(m_filterCombo);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText(tr("Search name or category..."));
    filterLayout->addWidget(m_searchEdit, 1);
    mainLayout->addLayout(filterLayout);

    // --- Content: list on the left, portrait + detail on the right.
    auto *contentLayout = new QHBoxLayout;

    m_monsterList = new QListWidget;
    m_monsterList->setMaximumWidth(220);
    contentLayout->addWidget(m_monsterList);

    auto *detailLayout = new QVBoxLayout;
    m_imageLabel = new QLabel;
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setMinimumHeight(220);
    m_imageLabel->setFrameShape(QFrame::StyledPanel);
    detailLayout->addWidget(m_imageLabel);

    m_descriptionText = new QTextEdit;
    m_descriptionText->setReadOnly(true);
    detailLayout->addWidget(m_descriptionText, 1);
    contentLayout->addLayout(detailLayout, 1);

    mainLayout->addLayout(contentLayout, 1);

    // --- Footer
    auto *footer = new QHBoxLayout;
    m_countLabel = new QLabel;
    footer->addWidget(m_countLabel);
    footer->addStretch(1);
    m_exitBtn = new QPushButton(tr("Close"));
    footer->addWidget(m_exitBtn);
    mainLayout->addLayout(footer);

    connect(m_monsterList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *current, QListWidgetItem *) {
                onMonsterSelected(current);
            });
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BestiaryDialog::onFilterChanged);
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &BestiaryDialog::onSearchChanged);
    connect(m_exitBtn, &QPushButton::clicked, this, &BestiaryDialog::onExitClicked);
}

void BestiaryDialog::refreshList() {
    m_monsterList->clear();

    int filterIdx = m_filterCombo->currentIndex();
    int minFloor = 0, maxFloor = 999;
    if (filterIdx == 1) { minFloor = 1; maxFloor = 5; }
    else if (filterIdx == 2) { minFloor = 6; maxFloor = 10; }
    else if (filterIdx == 3) { minFloor = 11; maxFloor = 15; }
    else if (filterIdx == 4) { minFloor = 16; maxFloor = 999; }

    const QString needle = m_searchEdit->text().trimmed().toLower();

    int count = 0;
    for (const QVariantMap& m : cached()) {
        const int floor = m.value("level").toInt();
        if (floor < minFloor || floor > maxFloor) continue;

        const QString name = m.value("name").toString();
        if (!needle.isEmpty()) {
            const QString category = m.value("category").toString().toLower();
            if (!name.toLower().contains(needle) && !category.contains(needle)) {
                continue;
            }
        }

        // Unmet monsters stay anonymous in the list; the tooltip gives no
        // stats away either.
        const bool met = isEncountered(name);
        auto *item = new QListWidgetItem(met ? name : QStringLiteral("???"), m_monsterList);
        item->setData(Qt::UserRole, name);
        item->setToolTip(met ? QStringLiteral("%1 — floor %2")
                                   .arg(m.value("category").toString())
                                   .arg(floor)
                             : tr("Not yet encountered"));
        if (!met) item->setForeground(QColor(120, 120, 120));
        ++count;
    }

    m_countLabel->setText(tr("Monsters: %1 of %2").arg(count).arg(cached().size()));

    if (m_monsterList->count() > 0) {
        m_monsterList->setCurrentRow(0);
    } else {
        m_imageLabel->clear();
        m_descriptionText->clear();
    }
}

void BestiaryDialog::onMonsterSelected(QListWidgetItem *item) {
    if (!item) return;
    // The visible text is "???" for unmet monsters; the real name rides in
    // UserRole so showEntry() can still find the data.
    const QString name = item->data(Qt::UserRole).toString();
    showEntry(name.isEmpty() ? item->text() : name);
}

void BestiaryDialog::showEntry(const QString& name) {
    const QVariantMap m = entry(name);
    if (m.isEmpty()) {
        m_imageLabel->clear();
        m_descriptionText->setHtml(tr("No data available."));
        return;
    }

    // Unmet monster: no portrait, no numbers — the entry stays a mystery
    // until the party has actually met it.
    if (!isEncountered(name)) {
        m_imageLabel->setPixmap(QPixmap());
        m_imageLabel->setText(tr("???"));
        m_descriptionText->setHtml(
            QStringLiteral("<h2>???</h2><p>%1</p>")
                .arg(tr("You have not yet encountered this creature. "
                        "Its entry in the bestiary remains a mystery.")));
        return;
    }

    // --- Portrait
    const QString path = imagePath(m);
    QPixmap pix(path);
    if (pix.isNull()) {
        m_imageLabel->setText(tr("No image\n(MON%1)")
                                  .arg(m.value("picture").toInt()));
        m_imageLabel->setPixmap(QPixmap());
    } else {
        m_imageLabel->setText(QString());
        m_imageLabel->setPixmap(pix.scaled(m_imageLabel->size(),
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
    }

    // --- Text
    QString html = QStringLiteral("<h2>%1</h2>").arg(name.toHtmlEscaped());

    const QString category = m.value("category").toString();
    if (!category.isEmpty()) {
        html += QStringLiteral("<p><b>%1</b>").arg(category.toHtmlEscaped());
        const QString size = m.value("size").toString();
        if (!size.isEmpty()) html += QStringLiteral(" &mdash; Size: %1").arg(size.toHtmlEscaped());
        html += QStringLiteral("</p>");
    }

    html += QStringLiteral("<table cellspacing='0' cellpadding='3'>");
    auto row = [&html](const QString& label, const QString& value) {
        if (value.isEmpty()) return;
        html += QStringLiteral("<tr><td><b>%1</b></td><td>%2</td></tr>")
                    .arg(label, value.toHtmlEscaped());
    };

    row(tr("Floor"), m.value("level").toInt() > 0
                        ? QString::number(m.value("level").toInt()) : tr("Unknown"));
    row(tr("Hits"), QString::number(m.value("hits").toInt()));
    row(tr("Attack"), QString::number(m.value("att").toInt()));
    row(tr("Defense"), QString::number(m.value("def").toInt()));
    row(tr("Groups"), QString::number(m.value("groups").toInt()));
    row(tr("Gold factor"), QString::number(m.value("goldFactor").toInt()));

    const QStringList stats = m.value("stats").toStringList();
    if (!stats.isEmpty()) row(tr("Stats"), stats.join(QStringLiteral(", ")));

    const QStringList res = m.value("resistances").toStringList();
    if (!res.isEmpty()) row(tr("Resistances"), res.join(QStringLiteral(", ")));

    // Walkthrough prose: the abilities/resistances the CSV only stores as
    // opaque bit flags, plus the group size and average hits it lists.
    const QString abilities = m.value("abilitiesText").toString();
    if (!abilities.isEmpty() && abilities.compare(QStringLiteral("None"),
                                                  Qt::CaseInsensitive) != 0) {
        row(tr("Abilities"), abilities);
    }
    const QStringList walkRes = m.value("walkthroughResistances").toStringList();
    if (!walkRes.isEmpty()) {
        row(tr("Noted resistances"), walkRes.join(QStringLiteral(", ")));
    }
    const QString wHits = m.value("walkthroughHits").toString();
    const QString wAD = m.value("walkthroughAD").toString();
    if (!wHits.isEmpty()) {
        row(tr("Avg. hits"), wAD.isEmpty() ? wHits
                                           : QStringLiteral("%1 (A/D %2)").arg(wHits, wAD));
    }
    const QString group = m.value("walkthroughGroup").toString();
    if (!group.isEmpty()) row(tr("Encounter"), group);

    html += QStringLiteral("</table>");
    m_descriptionText->setHtml(html);
}

void BestiaryDialog::onFilterChanged(int index) {
    Q_UNUSED(index)
    refreshList();
}

void BestiaryDialog::onSearchChanged(const QString& text) {
    Q_UNUSED(text)
    refreshList();
}

void BestiaryDialog::onExitClicked() {
    accept();
}
