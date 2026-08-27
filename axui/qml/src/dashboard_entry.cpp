#include <axui/dashboard_manager.h>

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QResource>
#include <cstdlib>
#include <iostream>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context,
                     const QString &msg) {
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

  Q_INIT_RESOURCE(axui_dashboard);
  Q_INIT_RESOURCE(axui_components);

  std::cout << "[Dashboard] Process started. Initializing QGuiApplication..."
            << std::endl;
  QGuiApplication app(argc, argv);
  app.setApplicationName("AXIOM Dashboard");
  app.setOrganizationName("AXIOM");

  QQmlApplicationEngine engine;

  axui::DashboardManager manager;

  bool mockMode = qEnvironmentVariableIntValue("AXIOM_MOCK_MODE") == 1;
  if (mockMode) {
    std::cout << "[Dashboard] Starting in MOCK MODE" << std::endl;
    manager.startMockData();
  } else {
    int port = qEnvironmentVariableIntValue("AXIOM_IPC_PORT");
    if (port == 0) port = 5555;
    std::cout << "[Dashboard] Connecting to AXIOM Engine on port " << port
              << std::endl;
  }

  engine.rootContext()->setContextProperty("dashboardManager", &manager);

  const QUrl url(QStringLiteral("qrc:/AxiomDashboard.qml"));
  std::cout << "[Dashboard] Loading QML from: " << url.toString().toStdString()
            << "..." << std::endl;

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
          std::cerr
              << "[Dashboard] CRITICAL: QML Engine failed to create object for "
              << url.toString().toStdString() << std::endl;
          QCoreApplication::exit(-1);
        }
      },
      Qt::QueuedConnection);

  engine.load(url);
  if (engine.rootObjects().isEmpty()) {
    std::cerr << "[Dashboard] ERROR: Root objects are empty after engine.load()"
              << std::endl;
  } else {
    std::cout << "[Dashboard] Dashboard ready. Executing app loop..."
              << std::endl;
  }

  if (!engine.rootObjects().isEmpty()) {
    QWindow *window = qobject_cast<QWindow *>(engine.rootObjects().first());
    if (window) {
      window->setTitle("AXIOM Dashboard | System Flow Monitor");
      window->setWidth(1280);
      window->setHeight(720);
    }
  }

  return app.exec();
}
