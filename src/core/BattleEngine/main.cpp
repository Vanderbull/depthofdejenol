#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <QDebug>
#include "BattleEngine.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    
    // 1. Instantiate the engine
    BattleEngine battleEngine;

    // 2. Explicitly set it on the root context *before* loading the QML file
    engine.rootContext()->setContextProperty("battleEngine", &battleEngine);

    qDebug() << "Attempting to load BattleView.qml...";
    
    // 3. Load the QML file
    engine.load(QUrl::fromLocalFile("BattleView.qml"));

    if (engine.rootObjects().isEmpty()) {
        qDebug() << "FAILED to load QML component!";
        return -1;
    }

    qDebug() << "UI loaded successfully, starting event loop...";
    return app.exec();
}
