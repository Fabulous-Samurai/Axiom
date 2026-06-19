// [MANDATE]: ZENITH PILLAR COMPLIANCE - Operation HARMONY
#include "app_runner.h"

#include <QQmlContext>
#include <QQuickWindow>
#include <iostream>

namespace {
void myMessageOutput(QtMsgType type, const QMessageLogContext& context,
                     const QString& msg) {
  QByteArray localMsg = msg.toLocal8Bit();
  const char* prefix = "[Qt]";
  switch (type) {
    case QtDebugMsg:
      prefix = "[Qt Debug] ";
      break;
    case QtInfoMsg:
      prefix = "[Qt Info] ";
      break;
    case QtWarningMsg:
      prefix = "[Qt Warning] ";
      break;
    case QtCriticalMsg:
      prefix = "[Qt Critical] ";
      break;
    case QtFatalMsg:
      prefix = "[Qt Fatal] ";
      break;
  }
  std::cout << prefix << localMsg.constData() << std::endl;
  if (type == QtFatalMsg) abort();
}
}  // namespace

namespace axui {

void AppRunner::setupMessageHandler() {
  qInstallMessageHandler(myMessageOutput);
}

AppRunner::AppRunner(int& argc, char** argv, const std::string& app_name)
    : app_(argc, argv) {
  setupMessageHandler();
  app_.setApplicationName(QString::fromStdString(app_name));
  app_.setOrganizationName("AXIOM");
}

int AppRunner::run(const std::string& main_qml_path,
                   const std::string& window_title) {
  const QUrl url(QString::fromStdString(main_qml_path));

  QObject::connect(
      &engine_, &QQmlApplicationEngine::objectCreated, &app_,
      [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) {
          std::cerr
              << "[AppRunner] CRITICAL: QML Engine failed to create object for "
              << url.toString().toStdString() << std::endl;
          QCoreApplication::exit(-1);
        }
      },
      Qt::QueuedConnection);

  engine_.load(url);

  if (engine_.rootObjects().isEmpty()) {
    std::cerr << "[AppRunner] ERROR: Root objects are empty after engine.load()"
              << std::endl;
    return -1;
  }

  QWindow* window = qobject_cast<QWindow*>(engine_.rootObjects().first());
  if (window) {
    window->setTitle(QString::fromStdString(window_title));
    window->setWidth(1280);
    window->setHeight(720);
  }

  std::cout << "[AppRunner] " << app_.applicationName().toStdString()
            << " ready. Executing app loop..." << std::endl;
  return app_.exec();
}

}  // namespace axui
