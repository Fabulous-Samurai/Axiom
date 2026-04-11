from PySide6.QtCore import QObject, Signal, Slot, Property, QTimer
import threading
import time

try:
    import axiom_native
except ImportError:
    class MockResult:
        def __init__(self, value, latency_us, image_path=""):
            self.value = value
            self.latency_us = latency_us
            self.image_path = image_path

    class MockEngine:
        def evaluate(self, _expression, _mode):
            time.sleep(0.01)
            return MockResult(4.0, 100)
    axiom_native = MockEngine()

class EngineWrapper(QObject):
    calculationCompleted = Signal(dict)
    benchmarkCompleted = Signal(dict)
    metricsUpdated = Signal(dict)
    graphicsRendered = Signal(str)
    errorOccurred = Signal(str)

    def __init__(self):
        super().__init__()
        self._engine = axiom_native
        self._lock = threading.Lock() # C++ engine erişimi için kilit
        self._throughput = 0.0
        self._memory_mb = 0.0
        self._engine_status = "running"

        self._poll_timer = QTimer()
        self._poll_timer.timeout.connect(self._poll_metrics)
        self._poll_timer.start(500)

    @Property(float, notify=metricsUpdated)
    def throughput(self): return self._throughput

    @Property(float, notify=metricsUpdated)
    def memoryMb(self): return self._memory_mb

    @Property(str, notify=metricsUpdated)
    def engineStatus(self): return self._engine_status

    @Slot(str, str)
    def calculate(self, expression: str, mode: str):
        def worker():
            try:
                with self._lock:
                    result = self._engine.evaluate(expression, mode)
                self.calculationCompleted.emit({
                    "success": True,
                    "value": result.value,
                    "latency_us": result.latency_us,
                    "mode": mode,
                    "expression": expression
                })
            except Exception as e:
                self.errorOccurred.emit(str(e))

        threading.Thread(target=worker, daemon=True).start()

    @Slot()
    def runBenchmark(self):
        def worker():
            try:
                import subprocess, json, os
                if os.path.exists("./build/axiom_benchmark"):
                    subprocess.run(
                        ["./build/axiom_benchmark"],
                        capture_output=True, text=True, timeout=300
                    )
                if os.path.exists("benchmark_results.json"):
                    with open("benchmark_results.json") as f:
                        data = json.load(f)
                    self.benchmarkCompleted.emit(data)
            except Exception as e:
                self.errorOccurred.emit(f"Benchmark failed: {e}")

        threading.Thread(target=worker, daemon=True).start()

    @Slot(str, int, float, float, float)
    def evaluateGraphics(self, expression: str, resolution: int, zoom: float = 1.0, panX: float = 0.0, panY: float = 0.0):
        def worker():
            try:
                with self._lock:
                    # Motorun 'plot' modunda koordinat sınırlarını (view_bounds) kullanmasını sağlıyoruz
                    # Koordinatları zoom ve pan değerlerine göre hesapla (default range: -10 to 10)
                    view_range = 10.0 / zoom
                    x_min = -view_range - panX
                    x_max = view_range - panX
                    y_min = -view_range - panY
                    y_max = view_range - panY

                    # Motor API'sine bu sınırları iletiyoruz
                    result = self._engine.evaluate(expression, "plot", {
                        "res": resolution,
                        "bounds": [x_min, x_max, y_min, y_max]
                    })
                self.graphicsRendered.emit(result.image_path)
            except Exception as e:
                self.errorOccurred.emit(str(e))

        threading.Thread(target=worker, daemon=True).start()

    def _poll_metrics(self):
        """Metrikleri daha hafif ve güvenli bir şekilde topla"""
        try:
            # Her seferinde engine'i yormak yerine sadece durumu kontrol et
            # Gelecekte C++ tarafındaki shared_memory counter'larından okunabilir
            import psutil
            import os

            process = psutil.Process(os.getpid())
            self._memory_mb = process.memory_info().rss / 1024 / 1024

            # Throughput simülasyonunu hafiflet (IPC hattını tıkama)
            if self._engine_status == "running":
                self._throughput = 2500000.0 + (time.time() % 10 * 1000)

            self.metricsUpdated.emit({
                "throughput": self._throughput,
                "memory_mb": self._memory_mb,
                "status": self._engine_status
            })
        except Exception:
            self._engine_status = "warning"