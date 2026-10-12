// fontManager.h
#ifndef fontManager_H
#define fontManager_H

#include <QObject>
#include <QFont>
#include <QPixmap>
#include <QPainter>
#include <QPoint>
#include <QMap>

class fontManager : public QObject {
    Q_OBJECT
public:
    static fontManager* instance();

    // Sprite Font Logic
    void loadSpriteSheet(const QString& path, int charWidth, int charHeight, int kerning);
    void drawSpriteText(QPainter* painter, const QString& text, const QPoint& position);

    // Standard Qt Font Logic
    void setProportionalFont(const QFont& font);
    void setFixedFont(const QFont& font);
    
    QFont proportionalFont() const { return m_proportionalFont; }
    QFont fixedFont() const { return m_fixedFont; }

signals:
    void fontsChanged();

private:
    fontManager() = default;
    
    QPixmap m_spriteSheet;
    int m_charWidth = 0;
    int m_charHeight = 0;
    int m_kerning = 0;

    QFont m_proportionalFont;
    QFont m_fixedFont;
    
    // The character map configuration - FIXED: proper alphanumeric sequence
    const QString m_layout = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; 
};

#endif