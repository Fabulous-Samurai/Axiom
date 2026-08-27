#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <iostream>

#include "axui/qml_loader.h"

int main(int argc, char* argv[]) {
  // Qt Application setup
  QGuiApplication app(argc, argv);
  app.setApplicationName("AXUI Viewer");
  app.setApplicationVersion("0.4.0");

  // AXUI Loader
  axui::QmlLoader loader;

  // Check command line arguments
  if (argc < 2) {
    std::cerr << "Usage: axui_viewer <file.axui> [theme.axtheme]\n";
    std::cerr << "\nExample:\n";
    std::cerr << "  axui_viewer dashboard.axui shadow_soft.axtheme\n";
    return 1;
  }

  std::string axui_path = argv[1];
  std::string theme_path =
      (argc >= 3) ? argv[2] : "axui/themes/shadow_soft.axtheme";

  // Load and render
  if (!loader.loadFromFile(axui_path, theme_path)) {
    std::cerr << "Failed to load AXUI file: " << axui_path << "\n";
    return 1;
  }

  std::cout << "AXUI application loaded successfully\n";
  std::cout << "Press F5 to reload\n";

  // Hot-reload keybinding (F5) can be handled in QML or via event filter
  // For simplicity in this example, we assume there's a way to trigger reload.

  return app.exec();
}
