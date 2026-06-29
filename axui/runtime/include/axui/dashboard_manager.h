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

enum class StageState : uint8_t {
    Offline  = 0,
    Idle     = 1,
    Active   = 2,
    Blocked  = 3,
    Error    = 4
};

/**
 * @brief Thread-safe Stage Metrics using atomics.
 * [ZENITH PILLAR 3]: Deterministic lock-free updates.
 */
struct StageMetrics {
    std::string_view stage_id;
    std::string_view display_name;
    int order = 0;

    std::atomic<StageState> state{StageState::Offline};
    std::atomic<std::string_view> error_message{""};

    std::atomic<uint64_t> total_messages{0};
    std::atomic<uint64_t> total_bytes{0};
    std::atomic<double> messages_per_second{0.0};
    std::atomic<double> bytes_per_second{0.0};
    std::atomic<double> peak_messages_per_second{0.0};

    std::atomic<double> avg_latency_us{0.0};
    std::atomic<double> p99_latency_us{0.0};
    std::atomic<double> max_latency_us{0.0};

    std::atomic<uint64_t> queue_capacity{0};
    std::atomic<uint64_t> queue_current{0};
    std::atomic<double> queue_fill_ratio{0.0};

    std::atomic<std::chrono::steady_clock::time_point> last_activity{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point started_at{std::chrono::steady_clock::now()};
    std::atomic<double> uptime_seconds{0.0};
};

struct LinkMetrics {
    std::string_view source_id;
    std::string_view target_id;
    std::atomic<double> throughput_ratio{0.0};
    std::atomic<double> drop_rate{0.0};
    std::atomic<bool> active{false};
};

struct SystemTelemetry {
    std::atomic<double> cpu_usage_percent{0.0};
    std::atomic<int> cpu_cores{0};
    AXIOM::FixedVector<double, 64> per_core_usage;

    std::atomic<uint64_t> ram_total_bytes{0};
    std::atomic<uint64_t> ram_used_bytes{0};
    std::atomic<double> ram_usage_percent{0.0};

    std::atomic<bool> gpu_available{false};
    std::atomic<double> gpu_usage_percent{0.0};
    std::atomic<uint64_t> vram_total_bytes{0};
    std::atomic<uint64_t> vram_used_bytes{0};
    std::atomic<double> vram_usage_percent{0.0};
    std::atomic<double> gpu_temp_celsius{0.0};

    std::atomic<double> disk_read_bytes_per_sec{0.0};
    std::atomic<double> disk_write_bytes_per_sec{0.0};
    std::atomic<double> ipc_bytes_per_sec{0.0};
};

class DashboardManager : public QObject {
    Q_OBJECT
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

    void registerStage(std::string_view stage_id, std::string_view display_name, int order) noexcept;
    void registerLink(std::string_view source_id, std::string_view target_id) noexcept;

    // --- High-Performance Lock-Free Updates ---
    void recordMessage(std::string_view stage_id, uint64_t byte_count = 0) noexcept;
    void recordLatency(std::string_view stage_id, double latency_us) noexcept;
    void updateStageState(std::string_view stage_id, StageState state) noexcept;
    void updateQueueStatus(std::string_view stage_id, uint64_t current, uint64_t capacity) noexcept;
    void reportError(std::string_view stage_id, std::string_view error_msg) noexcept;

    QVariantList stagesModel() const;
    QVariantList linksModel() const;
    QVariantMap telemetryModel() const;
    double totalThroughput() const noexcept;
    double totalLatency() const noexcept;
    int activeStageCount() const noexcept;
    bool isRecording() const { return recording_; }

    Q_INVOKABLE void resetCounters() noexcept;

signals:
    void stagesUpdated();
    void linksUpdated();
    void telemetryUpdated();
    void recordingChanged();
    void errorOccurred(const QString& stage_id, const QString& message);

private:
    QTimer* update_timer_ = nullptr;
    static constexpr int UPDATE_INTERVAL_MS = 100;

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

    // [ZENITH]: Multi-producer registry using minimal mutex for rare write operations
    mutable std::mutex registry_mutex_;
    AXIOM::FixedVector<std::pair<std::string_view, StageMetrics*>, 32> stages_;
    AXIOM::FixedVector<LinkMetrics*, 64> links_;

    // Lock-free history buffers per stage (pre-allocated)
    AXIOM::FixedVector<std::pair<std::string_view, ThroughputVec*>, 32> throughput_history_;
    AXIOM::FixedVector<std::pair<std::string_view, LatencyVec*>, 32> latency_history_;

    SystemTelemetry system_telemetry_;

    void collectSystemTelemetry();
    void calculateThroughput() noexcept;
    void calculateLatencyStats();
    void updateLinkMetrics();

    bool recording_ = false;

private slots:
    void onUpdateTick();
};

} // namespace axui
