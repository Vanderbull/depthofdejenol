#ifndef RESOLUTIONSCROLLER_H
#define RESOLUTIONSCROLLER_H

#include <QGuiApplication>
#include <QScreen>
#include <QSize>
#include <QPoint>

class ResolutionScaler {
public:
    static ResolutionScaler& instance() {
        static ResolutionScaler instance;
        return instance;
    }

    float scaleFactor() const {
        if (!m_scaleFactorValid) {
            computeScaleFactor();
        }
        return m_scaleFactor;
    }

    float scaleSize(int size) const {
        return static_cast<float>(size) * scaleFactor();
    }

    float scaleFontSize(float size) const {
        return size * scaleFactor();
    }

    QSize scaledSize(const QSize& size) const {
        return QSize(static_cast<int>(scaleSize(size.width())),
                     static_cast<int>(scaleSize(size.height())));
    }

    QPoint scaledPosition(const QPoint& pos) const {
        return QPoint(static_cast<int>(scaleSize(pos.x())),
                      static_cast<int>(scaleSize(pos.y())));
    }

    void setDesignResolution(int w, int h) {
        m_designWidth = w;
        m_designHeight = h;
        m_scaleFactorValid = false;
    }

    QSize designResolution() const {
        return QSize(m_designWidth, m_designHeight);
    }

private:
    ResolutionScaler() : m_scaleFactor(1.0f), m_designWidth(1920), m_designHeight(1080) {}

    void computeScaleFactor() const {
        QGuiApplication* app = qGuiApp;
        if (!app) return;

        QScreen* screen = app->primaryScreen();
        if (!screen) return;

        QRect available = screen->availableGeometry();
        if (available.isEmpty()) return;

        // Compute scale factor as min of width and height ratios
        float widthScale = static_cast<float>(available.width()) / m_designWidth;
        float heightScale = static_cast<float>(available.height()) / m_designHeight;
        m_scaleFactor = std::min(widthScale, heightScale);

        // Clamp to reasonable range [0.5, 2.0]
        m_scaleFactor = std::max(0.5f, std::min(2.0f, m_scaleFactor));
        m_scaleFactorValid = true;
    }

    mutable float m_scaleFactor;
    mutable int m_designWidth;
    mutable int m_designHeight;
    mutable bool m_scaleFactorValid = true;
};

#endif // RESOLUTIONSCROLLER_H
