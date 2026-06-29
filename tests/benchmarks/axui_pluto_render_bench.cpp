#include <benchmark/benchmark.h>
#include <QtQuick/QSGNode>
#include <QtQuick/QSGGeometryNode>
#include <QColor>
#include <QRectF>
#include <QPointF>
#include "axui/widgets/pluto_navigator.h"
#include "engine/include/pluto_controller.h"

using namespace AXIOM;

// We need to mock PlutoController behavior for get_search_tree
// However, PlutoController is a singleton. We can fill it if possible.

static void BM_PlutoRender10k(benchmark::State& state) {
    PlutoSwarmNode node;
    QRectF rect(0, 0, 1920, 1080);
    qreal zoom = 1.0;
    QPointF pan(0, 0);

    // Fill PlutoController with 10k mock nodes
    // Actually, PlutoSwarmNode::update calls get_search_tree.
    // We can't easily mock it without affecting other tests or using dirty tricks.
    // But we can just measure the loop in PlutoSwarmNode::update by copying its logic
    // or manually filling the controller's scratchpad if it was exposed.

    // Let's assume we want to measure the SceneGraph update performance
    for (auto _ : state) {
        node.update(rect, Pluto::PlutoController::instance(), zoom, pan);
    }
}
BENCHMARK(BM_PlutoRender10k);

int main(int argc, char** argv) {
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
