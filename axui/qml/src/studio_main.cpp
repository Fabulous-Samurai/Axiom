#include <cstdlib>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QIcon>
#include <QResource>
#include <axui/dashboard_manager.h>
#include <iostream>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:   std::cout << "[Qt Debug] " << localMsg.constData() << std::endl; break;
    case QtInfoMsg:    std::cout << "[Qt Info] " << localMsg.constData() << std::endl; break;
    case QtWarningMsg: std::cerr << "[Qt Warning] " << localMsg.constData() << std::endl; break;
    case QtCriticalMsg: std::cerr << "[Qt Critical] " << localMsg.constData() << std::endl; break;
    case QtFatalMsg:   std::cerr << "[Qt Fatal] " << localMsg.constData() << std::endl; abort();
    }
}

int main(int argc, char *argv[]) {
    qInstallMessageHandler(myMessageOutput);
    
    // Manual resource initialization (Studio-specific)
    Q_INIT_RESOURCE(axui_studio);
    Q_INIT_RESOURCE(axui_components);

    std::cout << "[Studio] Process started. Initializing QGuiApplication..." << std::endl;
    QGuiApplication app(argc, argv);
    app.setApplicationName("AXIOM Studio");
    app.setOrganizationName("AXIOM");

    QQmlApplicationEngine engine;
    
    // Initialize Dashboard Manager (Reuse for now, eventually StudioManager)
    axui::DashboardManager manager;
    engine.rootContext()->setContextProperty("dashboardManager", &manager);

    // Load Studio QML
    const QUrl url(QStringLiteral("qrc:/AxiomStudio.qml"));
    std::cout << "[Studio] Loading QML from: " << url.toString().toStdString() << "..." << std::endl;
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            std::cerr << "[Studio] CRITICAL: QML Engine failed to create object for " << url.toString().toStdString() << std::endl;
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
        std::cerr << "[Studio] ERROR: Root objects are empty after engine.load()" << std::endl;
    } else {
        std::cout << "[Studio] Studio ready. Executing app loop..." << std::endl;
    }

    return app.exec();
}
