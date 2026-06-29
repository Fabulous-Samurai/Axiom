#include <benchmark/benchmark.h>

#include <QCoreApplication>
#include <QThread>
#include <string>
#include <vector>

#include "axui/style_pipeline.h"

// We need a QCoreApplication to run QTimer and signals/slots used by
// StylePipeline
static int argc = 1;
static char* argv[] = {(char*)"benchmark"};
static QCoreApplication* app = nullptr;

class StylePipelineBenchmark : public benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State& state) {
    if (!app) {
      app = new QCoreApplication(argc, argv);
    }
  }

  void TearDown(const ::benchmark::State& state) {
    // We can keep the app alive between benchmark runs
  }
};

BENCHMARK_F(StylePipelineBenchmark, PushLatency)(benchmark::State& state) {
  axui::StylePipeline pipeline;

  std::string sample_json =
      R"({"color": "#ff0000", "padding": 10, "margin": {"top": 5, "bottom": 5}})";

  for (auto _ : state) {
    bool success = pipeline.pushUpdate(sample_json);
    benchmark::DoNotOptimize(success);
  }
}

BENCHMARK_F(StylePipelineBenchmark, EndToEndProcessing)
(benchmark::State& state) {
  axui::StylePipeline pipeline;

  std::string sample_json =
      R"({"color": "#ff0000", "padding": 10, "margin": {"top": 5, "bottom": 5}})";
  int num_items = state.range(0);

  for (auto _ : state) {
    // Producer: push items
    for (int i = 0; i < num_items; ++i) {
      pipeline.pushUpdate(sample_json);
    }

    // Consumer: process items manually since we're not running the event loop
    // StylePipeline uses QMetaObject::invokeMethod or direct call in tests
    // ideally, but its processQueue is private slot. We can process events to
    // trigger it.
    app->processEvents();
  }
}
BENCHMARK_REGISTER_F(StylePipelineBenchmark, EndToEndProcessing)->Range(8, 64);

int main(int argc, char** argv) {
  QCoreApplication qapp(argc, argv);
  app = &qapp;
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();
  return 0;
}
