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

  std::cout << "[Dashboard] Process started. Initializing QGuiApplication..."
            << std::endl;
  QGuiApplication app(argc, argv);
  app.setApplicationName("AXIOM Dashboard");
  app.setOrganizationName("AXIOM");

  QQmlApplicationEngine engine;

  // Initialize Dashboard Manager
  axui::DashboardManager manager;

  // [ZENITH]: Data collection starts automatically via QTimer in Manager ctor

  // Expose manager to QML
  engine.rootContext()->setContextProperty("dashboardManager", &manager);

  // Load main QML
  const QUrl url(QStringLiteral("qrc:/ui/AxiomDashboard.qml"));
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
    std::cerr << "[Dashboard] ERROR: Root objects are empty!" << std::endl;
    return -1;
  }

  std::cout << "[Dashboard] Ready. Executing app loop..." << std::endl;
  return app.exec();
}
