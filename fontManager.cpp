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

void fontManager::drawSpriteText(QPainter* painter, const QString& text, const QPoint& position)
{
    if (m_spriteSheet.isNull() || !painter) {
        return;
    }

    // The character sequence exactly as it appears in the image
    const QString layout = "ABCDEFGHI"   // Row 0
                           "HIJKLMM"     // Row 1
                           "NOPQRSTUUV"  // Row 2
                           "UWXYZ"       // Row 3
                           "124578901";  // Row 4

    QString upperText = text.toUpper();

    for (int i = 0; i < upperText.length(); ++i) {
        QChar c = upperText[i];
        
        if (c.isSpace()) {
            continue; 
        }

        int index = layout.indexOf(c);
        if (index == -1) {
            continue; 
        }

        int row = 0;
        int col = 0;

        // Map the character index to specific grid coordinates
        if (index < 9) { 
            row = 0; col = index; 
        } else if (index < 16) { 
            row = 1; col = (index - 9); 
        } else if (index < 26) { 
            row = 2; col = (index - 16); 
        } else if (index < 31) { 
            row = 3; col = (index - 26); 
        } else { 
            row = 4; col = (index - 31); 
        }

        // Source rectangle: Where the letter is in the PNG
        QRect sourceRect(col * m_charWidth, row * m_charHeight, m_charWidth, m_charHeight);
        
        // Target rectangle: Where to draw on the painter
        // Uses the kerning value to control horizontal spacing
        QRect targetRect(position.x() + (i * m_kerning), position.y(), m_charWidth, m_charHeight);

        painter->drawPixmap(targetRect, m_spriteSheet, sourceRect);
    }
}
