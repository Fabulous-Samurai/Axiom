#include <axui/core/app_runner.h>
#include <axui/dashboard_manager.h>  // Using this temporarily

#include <QResource>

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(axui_studio);
  Q_INIT_RESOURCE(axui_components);

  axui::AppRunner app(argc, argv, "AXIOM Studio");

  // Placeholder manager
  axui::DashboardManager manager;
  app.registerContextProperty("dashboardManager", &manager);

  return app.run("qrc:/AxiomStudio.qml", "AXIOM Studio");
}
