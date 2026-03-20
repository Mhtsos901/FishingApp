#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "include/FishingEngine/EngineController.hpp"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // --- ΝΕΟ: Ενεργοποιούμε το Material Design ---
    QQuickStyle::setStyle("Material");

    EngineController backendController;
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("Backend", &backendController);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("FishingEngine", "Main");

    return app.exec();
}