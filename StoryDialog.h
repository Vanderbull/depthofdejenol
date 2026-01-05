#ifndef STORYDIALOG_H
#define STORYDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPauseAnimation> // Added for the delay
#include <QSequentialAnimationGroup>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>

class StoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit StoryDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::FramelessWindowHint);
        setStyleSheet("background-color: black;");
        
        QVBoxLayout *layout = new QVBoxLayout(this);
        m_label = new QLabel("", this);
        m_label->setAlignment(Qt::AlignCenter);
        m_label->setStyleSheet("color: white; font-size: 32px; font-family: 'Georgia';");
        m_label->setWordWrap(true);
        // Attach opacity effect to the label
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
        m_label->setGraphicsEffect(eff);
        layout->addWidget(m_label);
        
        showFullScreen();
        showNextPart();
    }

private:
    QLabel *m_label;
    int m_currentPart = 0;
    QStringList m_storyParts = { 
        "Screen 1: The Golden Age of Rohin\n [Visual: A majestic underground city shimmering with the glow of precious metals.]\n Text: Fifteen hundred years ago, the land of Mordor was home to the legendary city of Rohin—known to historians as 'Dejenol,' or the 'City of the Mines'. Built by the Dwarves, it was the wealthiest city in the ancient world. From its deep shafts, they extracted a fortune in Platinum, Gold, Mithril, and the indestructible Adamantite. For centuries, the city thrived on the infinite riches of the earth.", 
        "Screen 2: The Doorway to Hell\n [Visual: A dark, jagged breach in a deep cavern floor with malevolent shadows emerging.]\n Text: As the upper veins were depleted, the miners dug ever deeper, descending for days into the lightless depths. Then, the disaster struck—a floor gave way in the lower levels, releasing foul and mystical creatures that killed everything in their path. The few who escaped returned with glazed, terrified looks, referring to the abyss as 'The Doorway to Hell'.", 
        "Screen 3: The Magical Entombment\n [Visual: A massive door of ElvenOak and Adamantite being sealed with glowing magical runes.]\n Text: To save Mordor, a massive door of ElvenOak and Adamantite was fashioned to entomb the evil within. Though an evil wizard and his thieves once broke the seal to steal the door’s riches, the city elders retaliated by locking the criminals inside and sealing the mines with magic so strong that only secret knowledge could ever reopen them. For a thousand years, the city of Dejenol was lost to time.",
        "Screen 4: The Discovery of Marlith\n [Visual: The bustling modern city of Marlith built atop ancient, moss-covered ruins.]\n Text: Today, the great city of Marlith stands upon the ruins of ancient Rohin. The entrance to the mines has been rediscovered, and scholars have spent years studying ancient manuscripts to reclaim the lost knowledge of the depths. Early explorers have already begun to push back the darkness, but they speak of a terrifying new threat: a 'Prince of Devils' said to dwell in the deepest shadows.",
        "Screen 5: Your Journey Begins\” [Visual: A torch-lit hallway leading down into total darkness.]\n Text: With the ranks of experienced explorers thinning, the city elders have formed the Guilds to train a new generation of adventurers. You are the newest student, tasked with mapping the unknown and reclaiming the treasures of the past. Step forward into the dark, for by your efforts, the unknown will become known.",
        "Welcome to The Depth of Dejenol."
    };

    void showNextPart() {
        if (m_currentPart >= m_storyParts.size()) {
            accept(); 
            return;
        }

        m_label->setText(m_storyParts[m_currentPart]);
        m_currentPart++;

        // 1. Fade In
        QPropertyAnimation *fadeIn = new QPropertyAnimation(m_label->graphicsEffect(), "opacity");
        fadeIn->setDuration(1000);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);

        // 2. Pause (Replaces the non-existent setStartDelay)
        QPauseAnimation *pause = new QPauseAnimation(1500, this);

        // 3. Fade Out
        QPropertyAnimation *fadeOut = new QPropertyAnimation(m_label->graphicsEffect(), "opacity");
        fadeOut->setDuration(2000);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);

        // Sequence: Fade In -> Wait -> Fade Out
        QSequentialAnimationGroup *group = new QSequentialAnimationGroup(this);
        group->addAnimation(fadeIn);
        group->addAnimation(pause);
        group->addAnimation(fadeOut);

        connect(group, &QSequentialAnimationGroup::finished, this, &StoryDialog::showNextPart);
        group->start();
    }
};

#endif
