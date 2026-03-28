#pragma once

#include <chrono>
#include <atomic>
#include <mutex>
#include <functional>
#include <optional>
#include <QObject>
#include <QVariantMap>
#include <QTimer>
#include "arena_allocator.h"
#include "fixed_vector.h"

namespace axui {

// ═══════════════════════════════════════════════════════════════════
// Pipeline Stage Tanımları
// ═══════════════════════════════════════════════════════════════════

enum class StageState : uint8_t {
    Offline  = 0,   // Stage başlatılmamış
    Idle     = 1,   // Çalışıyor ama veri yok
    Active   = 2,   // Aktif veri işliyor
    Blocked  = 3,   // Downstream bekleniyor (backpressure)
    Error    = 4    // Hata durumunda
};

struct StageMetrics {
    // Kimlik
    std::string_view stage_id;           // "ingress", "named_pipe", "spsc_queue", etc.
    std::string_view display_name;       // "Input Source", "Named Pipe", etc.
    int order = 0;                  // Pipeline sırası (0 = ilk)

    // Durum
    StageState state = StageState::Offline;
    std::string_view error_message;

    // Throughput
    std::atomic<uint64_t> total_messages{0};
    std::atomic<uint64_t> total_bytes{0};
    double messages_per_second = 0.0;
    double bytes_per_second = 0.0;
    double peak_messages_per_second = 0.0;

    // Latency
    double avg_latency_us = 0.0;    // Microseconds
    double p99_latency_us = 0.0;
    double max_latency_us = 0.0;

    // Queue/Buffer durumu (varsa)
    uint64_t queue_capacity = 0;
    uint64_t queue_current = 0;
    double queue_fill_ratio = 0.0;  // 0.0 - 1.0

    // Zaman
    std::chrono::steady_clock::time_point last_activity;
    std::chrono::steady_clock::time_point started_at;
    double uptime_seconds = 0.0;
};

struct LinkMetrics {
    std::string_view source_id;
    std::string_view target_id;
    double throughput_ratio = 0.0;  // 0.0 - 1.0 (animasyon hızı için)
    double drop_rate = 0.0;         // Kayıp oranı
    bool active = false;
};

struct SystemTelemetry {
    // CPU
    double cpu_usage_percent = 0.0;
    int cpu_cores = 0;
    AXIOM::FixedVector<double, 64> per_core_usage;

    // Memory
    uint64_t ram_total_bytes = 0;
    uint64_t ram_used_bytes = 0;
    double ram_usage_percent = 0.0;

    // GPU (opsiyonel)
    bool gpu_available = false;
    double gpu_usage_percent = 0.0;
    uint64_t vram_total_bytes = 0;
    uint64_t vram_used_bytes = 0;
    double vram_usage_percent = 0.0;
    double gpu_temp_celsius = 0.0;

    // Disk I/O
    double disk_read_bytes_per_sec = 0.0;
    double disk_write_bytes_per_sec = 0.0;

    // Network (IPC throughput)
    double ipc_bytes_per_sec = 0.0;
};

// ═══════════════════════════════════════════════════════════════════
// Dashboard Manager — QML Bridge
// ═══════════════════════════════════════════════════════════════════

class DashboardManager : public QObject {
    Q_OBJECT

    // QML'den erişilebilir property'ler
    Q_PROPERTY(QVariantList stages READ stagesModel NOTIFY stagesUpdated)
    Q_PROPERTY(QVariantList links READ linksModel NOTIFY linksUpdated)
    Q_PROPERTY(QVariantMap systemTelemetry READ telemetryModel NOTIFY telemetryUpdated)
    Q_PROPERTY(double totalThroughput READ totalThroughput NOTIFY stagesUpdated)
    Q_PROPERTY(double totalLatency READ totalLatency NOTIFY stagesUpdated)
    Q_PROPERTY(int activeStageCount READ activeStageCount NOTIFY stagesUpdated)
    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
    Q_PROPERTY(int sentryStatus READ sentryStatus NOTIFY telemetryUpdated)
    Q_PROPERTY(double scalingMultiplier READ scalingMultiplier NOTIFY telemetryUpdated)

public:
    explicit DashboardManager(QObject* parent = nullptr);
    ~DashboardManager() override;

    // ─── Stage Kayıt ─────────────────────────────────────────────
    
    // Pipeline stage'i kaydet
    void registerStage(std::string_view stage_id,
                       std::string_view display_name,
                       int order);

    // Stage'ler arası bağlantı tanımla
    void registerLink(std::string_view source_id,
                      std::string_view target_id);

    // ─── Runtime Güncelleme (Backend tarafından çağrılır) ────────

    // Mesaj işlendiğinde çağır
    void recordMessage(std::string_view stage_id, 
                       uint64_t byte_count = 0);

    // Latency ölçümü
    void recordLatency(std::string_view stage_id, 
                       double latency_us);

    // Stage durumu güncelle
    void updateStageState(std::string_view stage_id, 
                          StageState state);

    // Queue doluluk güncelle
    void updateQueueStatus(std::string_view stage_id,
                           uint64_t current, 
                           uint64_t capacity);

    // Hata bildir
    void reportError(std::string_view stage_id, 
                     std::string_view error_msg);

    // ─── QML Model Erişimi ───────────────────────────────────────
    
    QVariantList stagesModel() const;
    QVariantList linksModel() const;
    QVariantMap telemetryModel() const;
    double totalThroughput() const;
    double totalLatency() const;
    int activeStageCount() const;
    bool isRecording() const { return recording_; }
    int sentryStatus() const;
    double scalingMultiplier() const;

    // ─── QML'den çağrılabilir metodlar ───────────────────────────
    
    void startMockData();
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void resetCounters();
    Q_INVOKABLE QVariantList getThroughputHistory(
        const QString& stage_id, int seconds = 60) const;
    Q_INVOKABLE QVariantList getLatencyHistory(
        const QString& stage_id, int seconds = 60) const;

signals:
    void stagesUpdated();
    void linksUpdated();
    void telemetryUpdated();
    void recordingChanged();
    void errorOccurred(const QString& stage_id, const QString& message);
    void throughputAlert(const QString& stage_id, double current, double threshold);

private:
    // Periyodik güncelleme (her 100ms)
    QTimer* update_timer_ = nullptr;
    static constexpr int UPDATE_INTERVAL_MS = 100;

    // Dual-channel Arenas for telemetry history
    AXIOM::MemoryArena throughput_arena_;
    AXIOM::MemoryArena latency_arena_;

    struct ThroughputSample {
        std::chrono::steady_clock::time_point time;
        uint64_t messages;
        uint64_t bytes;
    };

    struct LatencySample {
        std::chrono::steady_clock::time_point time;
        double latency_us;
    };

    using ThroughputVec = AXIOM::FixedVector<ThroughputSample, 1024>;
    using LatencyVec = AXIOM::FixedVector<LatencySample, 1024>;

    // Stage verileri
    mutable std::mutex stages_mutex_;
    AXIOM::FixedVector<std::pair<std::string_view, StageMetrics>, 32> stages_;
    AXIOM::FixedVector<LinkMetrics, 64> links_;

    // Throughput hesaplama (sliding window)
    AXIOM::FixedVector<std::pair<std::string_view, ThroughputVec>, 32> throughput_history_;

    // Latency histogram (sliding window)
    AXIOM::FixedVector<std::pair<std::string_view, LatencyVec>, 32> latency_history_;

    // System telemetry
    SystemTelemetry system_telemetry_;
    
    // Telemetry Smoothing (EMA)
    double cpu_ema_ = 0.0;
    double ram_ema_ = 0.0;
    double gpu_ema_ = 0.0;
    const double ema_alpha_ = 0.3;
    void collectSystemTelemetry();

    // Hesaplama
    void calculateThroughput();
    void calculateLatencyStats();
    void updateLinkMetrics();
    void pruneHistory(int max_seconds = 300);

    // Recording
    bool recording_ = false;

private slots:
    void onUpdateTick();
};

} // namespace axui
