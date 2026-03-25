#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>


int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // --- ΝΕΟ: Ενεργοποιούμε το Material Design ---
    QQuickStyle::setStyle("Material");

    QQmlApplicationEngine engine;

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("FishingEngine", "Main");

    return app.exec();
}