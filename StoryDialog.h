#ifndef STORYDIALOG_H
#define STORYDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPauseAnimation>
#include <QSequentialAnimationGroup>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmap>
#include <QDebug>

class StoryDialog : public QDialog {
    Q_OBJECT
public:
    explicit StoryDialog(QWidget *parent = nullptr) : QDialog(parent) 
    {
        setWindowFlags(Qt::FramelessWindowHint);    
        QVBoxLayout *layout = new QVBoxLayout(this);
        m_label = new QLabel("", this);
        m_label->setAlignment(Qt::AlignCenter);
        m_label->setWordWrap(true);
        QPalette pal = m_label->palette();
        pal.setColor(QPalette::WindowText, Qt::white);
        m_label->setPalette(pal);
        m_label->setFont(QFont("Georgia", 24));
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
        m_label->setGraphicsEffect(eff);
        layout->addWidget(m_label);
        m_group = new QSequentialAnimationGroup(this);
        // --- LOAD TILES FROM ONE IMAGE ---
        loadStoryTiles("resources/images/story_spritesheet.png"); // Use your actual filename
        showFullScreen();
        showNextPart();
    }

protected:
    void paintEvent(QPaintEvent *event) override 
    {
        Q_UNUSED(event);
        QPainter painter(this);
        if (!m_currentBg.isNull()) {
            painter.drawPixmap(rect(), m_currentBg.scaled(size(), 
                               Qt::KeepAspectRatioByExpanding, 
                               Qt::SmoothTransformation));
        } else {
            painter.fillRect(rect(), Qt::black);
        }
    }

    void keyPressEvent(QKeyEvent *event) override 
    {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            m_group->stop();      
            showNextPart();       
        } else if (event->key() == Qt::Key_Escape) {
            accept();             
        } else {
            QDialog::keyPressEvent(event);
        }
    }

private:
    QLabel *m_label;
    QSequentialAnimationGroup *m_group;
    QPixmap m_currentBg;
    QList<QPixmap> m_storyTiles; // Stores the extracted tiles
    int m_currentPart = 0;
    QStringList m_storyParts = { 
        "The Golden Age of Rohin\n Fifteen hundred years ago, the land of Mordor was home to the legendary city of Rohin—known to historians as 'Dejenol,' or the 'City of the Mines'. Built by the Dwarves, it was the wealthiest city in the ancient world. From its deep shafts, they extracted a fortune in Platinum, Gold, Mithril, and the indestructible Adamantite. For centuries, the city thrived on the infinite riches of the earth.", 
        "The Doorway to Hell\n the upper veins were depleted, the miners dug ever deeper, descending for days into the lightless depths. Then, the disaster struck—a floor gave way in the lower levels, releasing foul and mystical creatures that killed everything in their path. The few who escaped returned with glazed, terrified looks, referring to the abyss as 'The Doorway to Hell'.", 
        "The Magical Entombment\n To save Mordor, a massive door of ElvenOak and Adamantite was fashioned to entomb the evil within. Though an evil wizard and his thieves once broke the seal to steal the door’s riches, the city elders retaliated by locking the criminals inside and sealing the mines with magic so strong that only secret knowledge could ever reopen them. For a thousand years, the city of Dejenol was lost to time.",
        "The Discovery of Marlith\n Today, the great city of Marlith stands upon the ruins of ancient Rohin. The entrance to the mines has been rediscovered, and scholars have spent years studying ancient manuscripts to reclaim the lost knowledge of the depths. Early explorers have already begun to push back the darkness, but they speak of a terrifying new threat: a 'Prince of Devils' said to dwell in the deepest shadows.",
        "Your Journey Begins\n With the ranks of experienced explorers thinning, the city elders have formed the Guilds to train a new generation of adventurers. You are the newest student, tasked with mapping the unknown and reclaiming the treasures of the past. Step forward into the dark, for by your efforts, the unknown will become known.",
        "Welcome to The Depth of Dejenol."
    };
    // Helper to slice the large image into 6 pieces (2x3 grid)
    void loadStoryTiles(const QString &fileName) 
    {
        QPixmap fullImage;
        if (!fullImage.load(fileName)) {
            qDebug() << "Failed to load composite image:" << fileName;
            return;
        }
        int tileWidth = fullImage.width() / 2;
        int tileHeight = fullImage.height() / 3;
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 2; ++col) {
                // Extract the specific rectangle for this part
                m_storyTiles.append(fullImage.copy(col * tileWidth, row * tileHeight, tileWidth, tileHeight));
            }
        }
    }
    
    void showNextPart() 
    {
        if (m_currentPart >= m_storyParts.size()) {
            accept(); 
            return;
        }

        // Set the current background from the sliced tiles
        if (m_currentPart < m_storyTiles.size()) {
            m_currentBg = m_storyTiles[m_currentPart];
            update(); 
        }

        m_label->setText(m_storyParts[m_currentPart]);
        m_currentPart++;
        m_label->graphicsEffect()->setProperty("opacity", 0.0);
        m_group->clear();
        QPropertyAnimation *fadeIn = new QPropertyAnimation(m_label->graphicsEffect(), "opacity");
        fadeIn->setDuration(1000);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        QPauseAnimation *pause = new QPauseAnimation(4000, this);
        QPropertyAnimation *fadeOut = new QPropertyAnimation(m_label->graphicsEffect(), "opacity");
        fadeOut->setDuration(1000);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        m_group->addAnimation(fadeIn);
        m_group->addAnimation(pause);
        m_group->addAnimation(fadeOut);
        disconnect(m_group, &QSequentialAnimationGroup::finished, nullptr, nullptr);
        connect(m_group, &QSequentialAnimationGroup::finished, this, &StoryDialog::showNextPart);
        m_group->start();
    }
};

#endif
