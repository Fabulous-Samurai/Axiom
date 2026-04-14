#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "axui/dashboard_manager.h"
#include "axui/jit_execution_manager.h"
#include "cpu_optimization.h"
#include "expressway/axiom_expressway_node.h"
#include "pluto_controller.h"
#include "pluto_navigator.h"
#include "telemetry.h"
#include "throughput_node.h"

// Simulated Engine Thread
void run_engine_simulation(std::atomic<bool> &running) {
  // [ZENITH CORE PINNING]: Pin simulation to Core 1
  AXIOM::CPUOptimization::SetThreadAffinity(1);

  std::cout << "[Engine] Simulation thread started on Core 1." << std::endl;

  // Initialize IPC Shared Memory
  AXIOM::TelemetryScribe::instance().start("AXIOM_STUDIO_LIVE");

  // Initialize Pluto Controller
  AXIOM::Pluto::PlutoController::instance().init(10, 100);

  double time = 0;
  while (running) {
    // Simulate 2.5M ops/sec with some harmonic jitter
    double base_ops = 2500000.0;
    double jitter = 500000.0 * std::sin(time * 2.0);

    AXIOM::TelemetryScribe::instance().log_throughput(base_ops + jitter);

    // Also step Pluto search
    AXIOM::Pluto::PlutoController::instance().step_search();

    time += 0.01;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::cout << "[Engine] Simulation thread stopped." << std::endl;
}

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);
  app.setApplicationName("AXIOM Studio");
  app.setOrganizationName("AXIOM");
  app.setApplicationVersion("1.0.0");

  std::cout << "[Studio] Initializing AXIOM Studio Suite..." << std::endl;

  QCommandLineParser parser;
  parser.setApplicationDescription(
      "AXIOM Studio | Integrated Development Environment");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption recoveryOption("recovery",
                                    "Force start in recovery mode.");
  parser.addOption(recoveryOption);

  QCommandLineOption noSimOption("no-sim",
                                 "Disable the engine simulation thread.");
  parser.addOption(noSimOption);

  parser.process(app);

  // Start Engine Simulator
  std::atomic<bool> engine_running{true};
  std::thread engine_thread;

  if (!parser.isSet(noSimOption)) {
    engine_thread =
        std::thread(run_engine_simulation, std::ref(engine_running));
  } else {
    std::cout << "[Studio] Engine simulation disabled via CLI." << std::endl;
  }

  qmlRegisterType<expressway::AxiomExpresswayNode>("Axiom.Expressway", 1, 0,
                                                   "ExpresswayView");
  qmlRegisterType<AXIOM::AxiomThroughputItem>("Axiom.UI", 1, 0,
                                              "ThroughputGraph");
  qmlRegisterType<AXIOM::PlutoSwarmNavigator>("Axiom.UI", 1, 0,
                                              "PlutoNavigator");

  QQmlApplicationEngine qml_engine;

  // Managers
  static axui::DashboardManager dashboardManager;
  static axui::JitExecutionManager jitExecutionManager;
  qml_engine.rootContext()->setContextProperty("dashboardManager",
                                               &dashboardManager);
  qml_engine.rootContext()->setContextProperty("jitExecutionManager",
                                               &jitExecutionManager);

  const QUrl mainUrl(QStringLiteral("qrc:/ui/main.qml"));
  const QUrl recoveryUrl(QStringLiteral("qrc:/ui/Recovery.qml"));

  QObject::connect(
      &qml_engine, &QQmlApplicationEngine::objectCreated, &app,
      [mainUrl, recoveryUrl, &qml_engine](QObject *obj, const QUrl &objUrl) {
        if (!obj && objUrl == mainUrl) qml_engine.load(recoveryUrl);
      },
      Qt::QueuedConnection);

  if (parser.isSet(recoveryOption)) {
    std::cout << "[Studio] Forcing recovery mode via CLI." << std::endl;
    qml_engine.load(recoveryUrl);
  } else {
    qml_engine.load(mainUrl);
  }

  int result = app.exec();

  // Cleanup
  engine_running = false;
  if (engine_thread.joinable()) engine_thread.join();

  return result;
}
