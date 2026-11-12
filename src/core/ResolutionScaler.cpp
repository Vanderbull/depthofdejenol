#include "ResolutionScaler.h"

#include <QGuiApplication>
#include <QScreen>
#include <algorithm>

void ResolutionScaler::computeScaleFactor() const {
    QGuiApplication* app = qGuiApp;
    if (!app) return;

    QScreen* screen = app->primaryScreen();
    if (!screen) return;

    QRect available = screen->availableGeometry();
    if (available.isEmpty()) return;

    float widthScale = static_cast<float>(available.width()) / m_designWidth;
    float heightScale = static_cast<float>(available.height()) / m_designHeight;
    m_scaleFactor = std::min(widthScale, heightScale);

    m_scaleFactor = std::max(0.5f, std::min(2.0f, m_scaleFactor));
    m_scaleFactorValid = true;
}
