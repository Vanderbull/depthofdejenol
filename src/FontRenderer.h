// A dedicated helper for sprite-based text rendering
class FontRenderer {
public:
    static void init(const QString& spritePath);
    
    // Static helper called by Widgets' paintEvents
    static void drawText(QPainter* painter, const QString& text, const QPoint& pos);

private:
    static QPixmap m_spriteSheet;
};
