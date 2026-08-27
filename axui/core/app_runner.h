// [MANDATE]: ZENITH PILLAR COMPLIANCE - Operation HARMONY
/**
 * @file app_runner.h
 * @brief Centralized Application Runner for all AXUI frontends.
 * Consolidates QGuiApplication and QQmlApplicationEngine setup.
 */

#pragma once

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <string>

namespace axui {

class AppRunner {
 public:
  AppRunner(int& argc, char** argv, const std::string& app_name);

  QQmlApplicationEngine& engine() { return engine_; }

  // Register a context property before running
  template <typename T>
  void registerContextProperty(const QString& name, T* object) {
    engine_.rootContext()->setContextProperty(name, object);
  }

  int run(const std::string& main_qml_path, const std::string& window_title);

 private:
  QGuiApplication app_;
  QQmlApplicationEngine engine_;

  static void setupMessageHandler();
};

}  // namespace axui
