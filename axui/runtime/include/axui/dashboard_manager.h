#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <optional>
#include <QObject>
#include <QVariantMap>
#include <QTimer>
#include "arena_allocator.h"

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
    std::string stage_id;           // "ingress", "named_pipe", "spsc_queue", etc.
    std::string display_name;       // "Input Source", "Named Pipe", etc.
    int order = 0;                  // Pipeline sırası (0 = ilk)

    // Durum
    StageState state = StageState::Offline;
    std::string error_message;

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
    std::string source_id;
    std::string target_id;
    double throughput_ratio = 0.0;  // 0.0 - 1.0 (animasyon hızı için)
    double drop_rate = 0.0;         // Kayıp oranı
    bool active = false;
};

struct SystemTelemetry {
    // CPU
    double cpu_usage_percent = 0.0;
    int cpu_cores = 0;
    std::vector<double> per_core_usage;

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

public:
    explicit DashboardManager(QObject* parent = nullptr);
    ~DashboardManager() override;

    // ─── Stage Kayıt ─────────────────────────────────────────────
    
    // Pipeline stage'i kaydet
    void registerStage(const std::string& stage_id,
                       const std::string& display_name,
                       int order);

    // Stage'ler arası bağlantı tanımla
    void registerLink(const std::string& source_id,
                      const std::string& target_id);

    // ─── Runtime Güncelleme (Backend tarafından çağrılır) ────────

    // Mesaj işlendiğinde çağır
    void recordMessage(const std::string& stage_id, 
                       uint64_t byte_count = 0);

    // Latency ölçümü
    void recordLatency(const std::string& stage_id, 
                       double latency_us);

    // Stage durumu güncelle
    void updateStageState(const std::string& stage_id, 
                          StageState state);

    // Queue doluluk güncelle
    void updateQueueStatus(const std::string& stage_id,
                           uint64_t current, 
                           uint64_t capacity);

    // Hata bildir
    void reportError(const std::string& stage_id, 
                     const std::string& error_msg);

    // ─── QML Model Erişimi ───────────────────────────────────────
    
    QVariantList stagesModel() const;
    QVariantList linksModel() const;
    QVariantMap telemetryModel() const;
    double totalThroughput() const;
    double totalLatency() const;
    int activeStageCount() const;
    bool isRecording() const { return recording_; }

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

    using ThroughputVec = std::vector<ThroughputSample, AXIOM::ArenaAllocator<ThroughputSample>>;
    using LatencyVec = std::vector<LatencySample, AXIOM::ArenaAllocator<LatencySample>>;

    // Stage verileri
    mutable std::mutex stages_mutex_;
    std::unordered_map<std::string, StageMetrics> stages_;
    std::vector<LinkMetrics> links_;

    // Throughput hesaplama (sliding window)
    std::unordered_map<std::string, ThroughputVec> throughput_history_;

    // Latency histogram (sliding window)
    std::unordered_map<std::string, LatencyVec> latency_history_;

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
