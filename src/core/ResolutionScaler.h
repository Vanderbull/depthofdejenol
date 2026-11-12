#ifndef RESOLUTIONSCROLLER_H
#define RESOLUTIONSCROLLER_H

#include <QGuiApplication>
#include <QScreen>
#include <QSize>
#include <QPoint>
#include <algorithm>

class ResolutionScaler
{
public:
    static ResolutionScaler& instance()
    {
        static ResolutionScaler instance;
        return instance;
    }

    float scaleFactor() const { if (!m_scaleFactorValid) { computeScaleFactor(); } return m_scaleFactor; }
    float scaleSize(int size) const { return static_cast<int>(static_cast<float>(size) * scaleFactor()); }
    float scaleFontSize(float size) const { return size * scaleFactor(); }
    QSize scaledSize(const QSize& size) const { return QSize(static_cast<int>(scaleSize(size.width())), static_cast<int>(scaleSize(size.height()))); }
    QPoint scaledPosition(const QPoint& pos) const { return QPoint(static_cast<int>(scaleSize(pos.x())), static_cast<int>(scaleSize(pos.y()))); }
    QSize designResolution() const { return QSize(m_designWidth, m_designHeight); }
    void setDesignResolution(int w, int h) { m_designWidth = w; m_designHeight = h; m_scaleFactorValid = false; }
    void setDefaultDesignResolution(int w, int h) { m_designWidth = w; m_designHeight = h; }

private:
    explicit ResolutionScaler() : m_scaleFactor(1.0f), m_designWidth(1920), m_designHeight(1080) {}
    ~ResolutionScaler() = default;
    void computeScaleFactor() const;

    mutable float m_scaleFactor;
    mutable bool m_scaleFactorValid = true;
    int m_designWidth;
    int m_designHeight;
};

#endif // RESOLUTIONSCROLLER_H
