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
    case QtDebugMsg:
        std::cout << "[Qt Debug] " << localMsg.constData() << std::endl;
        break;
    case QtInfoMsg:
        std::cout << "[Qt Info] " << localMsg.constData() << std::endl;
        break;
    case QtWarningMsg:
        std::cerr << "[Qt Warning] " << localMsg.constData() << std::endl;
        break;
    case QtCriticalMsg:
        std::cerr << "[Qt Critical] " << localMsg.constData() << std::endl;
        break;
    case QtFatalMsg:
        std::cerr << "[Qt Fatal] " << localMsg.constData() << std::endl;
        abort();
    }
}

int main(int argc, char *argv[]) {
    qInstallMessageHandler(myMessageOutput);
    // Manual resource initialization
    Q_INIT_RESOURCE(axui_dashboard);
    Q_INIT_RESOURCE(axui_components);

    std::cout << "[Studio] Process started. Initializing QGuiApplication..." << std::endl;
    QGuiApplication app(argc, argv);
    app.setApplicationName("AXIOM Studio");
    app.setOrganizationName("AXIOM");
    app.setOrganizationDomain("axiom.engine");

    QQmlApplicationEngine engine;
    
    // Initialize Dashboard Manager
    axui::DashboardManager manager;
    
    // Check for mock mode from environment
    bool mockMode = qEnvironmentVariableIntValue("AXIOM_MOCK_MODE") == 1;
    if (mockMode) {
        std::cout << "[Studio] Starting in MOCK MODE" << std::endl;
        manager.startMockData();
    } else {
        int port = qEnvironmentVariableIntValue("AXIOM_IPC_PORT");
        if (port == 0) port = 5555;
        std::cout << "[Studio] Connecting to AXIOM Engine on port " << port << std::endl;
        // manager.connectToEngine(port); // TODO: Implement IPC connection
    }

    // Expose manager to QML
    engine.rootContext()->setContextProperty("dashboardManager", &manager);

    // Load main QML
    const QUrl url(QStringLiteral("qrc:/AxiomDashboard.qml"));
    std::cout << "[Studio] Loading QML from: " << url.toString().toStdString() << "..." << std::endl;
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            std::cerr << "[Studio] CRITICAL: QML Engine failed to create object for " << url.toString().toStdString() << std::endl;
            QCoreApplication::exit(-1);
        } else {
            std::cout << "[Studio] QML Object successfully created." << std::endl;
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
        std::cerr << "[Studio] ERROR: Root objects are empty after engine.load()" << std::endl;
    } else {
        std::cout << "[Studio] Studio ready. Executing app loop..." << std::endl;
    }

    // Set window properties if loaded
    if (!engine.rootObjects().isEmpty()) {
        QWindow *window = qobject_cast<QWindow *>(engine.rootObjects().first());
        if (window) {
            window->setTitle("AXIOM Studio | System Flow Monitor");
            window->setWidth(1280);
            window->setHeight(720);
            // window->setIcon(QIcon(":/assets/axiom_logo.png"));
        }
    }

    return app.exec();
}
