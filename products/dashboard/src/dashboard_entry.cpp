#include <axui/core/app_runner.h>
#include <axui/dashboard_manager.h>
#include <QResource>

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE(axui_dashboard);
    Q_INIT_RESOURCE(axui_components);

    axui::AppRunner app(argc, argv, "AXIOM Dashboard");

    axui::DashboardManager manager;
    app.registerContextProperty("dashboardManager", &manager);

    // Logic for mock mode or IPC connection can be handled inside the runner or here.
    // For now, keeping it simple.

    return app.run("qrc:/AxiomDashboard.qml", "AXIOM Dashboard | System Flow Monitor");
}
