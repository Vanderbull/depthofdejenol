
#include "fontManager.h"
#include <QApplication>
#include <QWidget>
#include <QDebug>

fontManager* fontManager::instance()
{
    static fontManager instance;
    return &instance;
}

void fontManager::loadSpriteSheet(const QString& path, int charWidth, int charHeight, int kerning)
{
    if (!m_spriteSheet.load(path)) {
        qWarning() << "fontManager: Failed to load sprite sheet from" << path;
        return;
    }
    m_charWidth = charWidth;
    m_charHeight = charHeight;
    m_kerning = kerning;
    qDebug() << "fontManager: Sprite sheet loaded successfully.";
}

void fontManager::setProportionalFont(const QFont& font)
{
    m_proportionalFont = font;
    
    // Apply to the entire application instance
    if (qApp) {
        qApp->setFont(font);
    }

    // Force all active widgets to update their font immediately
    for (QWidget *widget : QApplication::allWidgets()) {
        widget->setFont(font);
        widget->update();
    }
    
    emit fontsChanged();
}

void fontManager::setFixedFont(const QFont& font)
{
    m_fixedFont = font;
    emit fontsChanged();
}

// 1080x480px 9 chars x 120px and 4 rows x 120
void fontManager::drawSpriteText(QPainter* painter, const QString& text, const QPoint& position)
{
    if (m_spriteSheet.isNull() || !painter) return;

    const QString layout = "ABCDEFGHI"
                           "JKLMNOPQR"
                           "STUVWXYZ1"
                           "234567890";

    const int charsPerRow = 9;
    const int boxSize = 120;
    const int m_kerning = 1;

    QString upperText = text.toUpper();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);

    for (int i = 0; i < upperText.length(); ++i) {
        QChar c = upperText[i];
        
        // Handle spacing: move the cursor even if it's a space
        if (c.isSpace()) continue; 

        int index = layout.indexOf(c);
        if (index == -1) continue;

        int row = index / charsPerRow;
        int col = index % charsPerRow;

        // Source: The 120x120 slice from the sheet
        QRect sourceRect(col * boxSize, row * boxSize, boxSize, boxSize);
        
        // FIX: Calculate X by multiplying index by (Box Width + Kerning)
        // If m_kerning is 0, they will touch perfectly.
        // If m_kerning is 10, there will be a 10px gap.
        int xOffset = position.x() + (i * (boxSize + m_kerning));
        
        QRect targetRect(xOffset, position.y(), boxSize, boxSize);

        painter->drawPixmap(targetRect, m_spriteSheet, sourceRect);
    }
}
