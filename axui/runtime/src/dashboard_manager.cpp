#include "axui/dashboard_manager.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include "sentry.h"
#include "telemetry.h"
#include "pmu_orchestrator.h"
#include "pluto_controller.h"
#include "zenith_jit.h"
#include "arena_allocator.h"

#ifdef __linux__
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

namespace axui {

DashboardManager::DashboardManager(QObject* parent)
    : QObject(parent),
      throughput_arena_(64 * 1024 * 1024), // 64MB Arena A
      latency_arena_(64 * 1024 * 1024)    // 64MB Arena B
{
    update_timer_ = new QTimer(this);
    update_timer_->setInterval(UPDATE_INTERVAL_MS);
    connect(update_timer_, &QTimer::timeout, this, &DashboardManager::onUpdateTick);
    update_timer_->start();
}

DashboardManager::~DashboardManager() = default;

// ═══════════════════════════════════════════════════════════════════
// STAGE KAYIT
// ═══════════════════════════════════════════════════════════════════

void DashboardManager::registerStage(
    std::string_view stage_id,
    std::string_view display_name,
    int order
) noexcept {
    std::lock_guard lock(stages_mutex_);

    auto it = std::find_if(stages_.begin(), stages_.end(),
                           [&](const auto& pair) { return pair.first == stage_id; });

    if (it == stages_.end()) {
        // Not found, create a new entry
        StageMetrics metrics;
        metrics.stage_id = stage_id;
        metrics.display_name = display_name;
        metrics.order = order;
        metrics.state = StageState::Idle;
        metrics.started_at = std::chrono::steady_clock::now();
        metrics.total_messages.store(0);
        metrics.total_bytes.store(0);
        stages_.push_back({stage_id, metrics});
    } else {
        // Found, update existing entry
        it->second.display_name = display_name;
        it->second.order = order;
        it->second.state = StageState::Idle;
        it->second.started_at = std::chrono::steady_clock::now();
        it->second.total_messages.store(0);
        it->second.total_bytes.store(0);
    }
}

void DashboardManager::registerLink(
    std::string_view source_id,
    std::string_view target_id
) noexcept {
    LinkMetrics link;
    link.source_id = source_id;
    link.target_id = target_id;
    link.active = false;
    links_.push_back(std::move(link));
}

// ═══════════════════════════════════════════════════════════════════
// RUNTIME GÜNCELLEME
// ═══════════════════════════════════════════════════════════════════

void DashboardManager::recordMessage(
    std::string_view stage_id,
    uint64_t byte_count
) noexcept {    std::lock_guard lock(stages_mutex_);
    
    auto it = std::find_if(stages_.begin(), stages_.end(),
                           [&](const auto& pair) { return pair.first == stage_id; });
    if (it == stages_.end()) return;

    it->second.total_messages.fetch_add(1, std::memory_order_relaxed);
    it->second.total_bytes.fetch_add(byte_count, std::memory_order_relaxed);
    it->second.last_activity = std::chrono::steady_clock::now();

    if (it->second.state == StageState::Idle) {
        it->second.state = StageState::Active;
    }

    // Throughput history
    auto it_history = std::find_if(throughput_history_.begin(), throughput_history_.end(),
                                   [&](const auto& pair) { return pair.first == stage_id; });

    if (it_history == throughput_history_.end()) {
        throughput_history_.push_back({stage_id, ThroughputVec(AXIOM::ArenaAllocator<ThroughputSample>(&throughput_arena_))});
        it_history = std::prev(throughput_history_.end()); // Point to the newly added element
    }

    it_history->second.push_back({
        std::chrono::steady_clock::now(),
        it->second.total_messages.load(),
        it->second.total_bytes.load()
    });
}

void DashboardManager::recordLatency(
    std::string_view stage_id,
    double latency_us
) noexcept {    std::lock_guard lock(stages_mutex_);

    // Latency history
    auto it_history = std::find_if(latency_history_.begin(), latency_history_.end(),
                                   [&](const auto& pair) { return pair.first == stage_id; });

    if (it_history == latency_history_.end()) {
        latency_history_.push_back({stage_id, LatencyVec(AXIOM::ArenaAllocator<LatencySample>(&latency_arena_))});
        it_history = std::prev(latency_history_.end()); // Point to the newly added element
    }

    it_history->second.push_back({
        std::chrono::steady_clock::now(),
        latency_us
    });
}

void DashboardManager::updateStageState(
    std::string_view stage_id,
    StageState state
) noexcept {    std::lock_guard lock(stages_mutex_);
    
    auto it = std::find_if(stages_.begin(), stages_.end(),
                           [&](const auto& pair) { return pair.first == stage_id; });
    if (it != stages_.end()) {
        it->second.state = state;
    }
}

void DashboardManager::updateQueueStatus(
    std::string_view stage_id,
    uint64_t current,
    uint64_t capacity
) noexcept {    std::lock_guard lock(stages_mutex_);
    
    auto it = std::find_if(stages_.begin(), stages_.end(),
                           [&](const auto& pair) { return pair.first == stage_id; });
    if (it != stages_.end()) {
        it->second.queue_current = current;
        it->second.queue_capacity = capacity;
        it->second.queue_fill_ratio = capacity > 0 
            ? static_cast<double>(current) / capacity 
            : 0.0;

        // Backpressure detection
        if (it->second.queue_fill_ratio > 0.9) {
            it->second.state = StageState::Blocked;
        }
    }
}

void DashboardManager::reportError(
    std::string_view stage_id,
    std::string_view error_msg
) noexcept {    std::lock_guard lock(stages_mutex_);
    
    auto it = std::find_if(stages_.begin(), stages_.end(),
                           [&](const auto& pair) { return pair.first == stage_id; });
    if (it != stages_.end()) {
        it->second.state = StageState::Error;
        it->second.error_message = error_msg;
    }

    emit errorOccurred(
        QString::fromUtf8(stage_id.data(), stage_id.size()),
        QString::fromUtf8(error_msg.data(), error_msg.size())
    );
}

// ═══════════════════════════════════════════════════════════════════
// PERİYODİK GÜNCELLEME
// ═══════════════════════════════════════════════════════════════════

void DashboardManager::onUpdateTick() {
    {
        std::lock_guard lock(stages_mutex_);
        calculateThroughput();
        calculateLatencyStats();
        updateLinkMetrics();
        pruneHistory();
    }

    collectSystemTelemetry();

    // [ZENITH FAST-PATH]: Update processor stage with real engine throughput
    double real_tp = AXIOM::TelemetryScribe::instance().read_throughput();
    recordMessage("processor", static_cast<uint64_t>(real_tp / 100.0));

    emit stagesUpdated();
    emit linksUpdated();
    emit telemetryUpdated();
}

void DashboardManager::calculateThroughput() noexcept {
    auto now = std::chrono::steady_clock::now();

    for (auto& stage_pair : stages_) {
        auto& id = stage_pair.first;
        auto& stage = stage_pair.second;

        auto it_history = std::find_if(throughput_history_.begin(), throughput_history_.end(),
                                       [&](const auto& pair) { return pair.first == id; });
        if (it_history == throughput_history_.end()) {
            stage.messages_per_second = 0;
            stage.bytes_per_second = 0;
            continue;
        }
        auto& history = it_history->second;
        if (history.size() < 2) {
            stage.messages_per_second = 0;
            stage.bytes_per_second = 0;
            continue;
        }

        // Son 1 saniyeye bak
        auto one_sec_ago = now - std::chrono::seconds(1);
        
        auto start_it = std::lower_bound(
            history.begin(), history.end(), one_sec_ago,
            [](const ThroughputSample& s, const auto& t) { 
                return s.time < t; 
            }
        );

        if (start_it != history.end() && !history.empty()) {
            auto duration = std::chrono::duration<double>(
                history.back().time - start_it->time
            ).count();

            if (duration > 0) {
                uint64_t msg_diff = history.back().messages - start_it->messages;
                uint64_t byte_diff = history.back().bytes - start_it->bytes;

                stage.messages_per_second = msg_diff / duration;
                stage.bytes_per_second = byte_diff / duration;
                stage.peak_messages_per_second = std::max(
                    stage.peak_messages_per_second,
                    stage.messages_per_second
                );
            }
        }

        // Idle detection (2 saniye veri yoksa)
        auto since_last = std::chrono::duration<double>(
            now - stage.last_activity
        ).count();

        if (since_last > 2.0 && stage.state == StageState::Active) {
            stage.state = StageState::Idle;
        }

        // Uptime
        stage.uptime_seconds = std::chrono::duration<double>(
            now - stage.started_at
        ).count();
    }
}

void DashboardManager::calculateLatencyStats() noexcept {
    auto now = std::chrono::steady_clock::now();
    auto window = now - std::chrono::seconds(10);

    for (auto& stage_pair : stages_) {
        auto& id = stage_pair.first;
        auto& stage = stage_pair.second;

        auto it_history = std::find_if(latency_history_.begin(), latency_history_.end(),
                                       [&](const auto& pair) { return pair.first == id; });
        if (it_history == latency_history_.end()) {
            stage.avg_latency_us = 0;
            stage.p99_latency_us = 0;
            stage.max_latency_us = 0;
            continue;
        }
        auto& history = it_history->second;

        // Son 10 saniyelik verileri filtrele
        AXIOM::FixedVector<double, 1024> recent; // Using 1024 as max capacity, same as LatencyVec
        for (const auto& sample : history) {
            if (sample.time >= window) {
                recent.push_back(sample.latency_us);
            }
        }

        if (recent.empty()) {
            stage.avg_latency_us = 0;
            stage.p99_latency_us = 0;
            stage.max_latency_us = 0;
            continue;
        }

        // Average
        stage.avg_latency_us = std::accumulate(
            recent.begin(), recent.end(), 0.0
        ) / recent.size();

        // Sort for percentiles
        std::sort(recent.begin(), recent.end());

        // P99
        size_t p99_idx = static_cast<size_t>(recent.size() * 0.99);
        stage.p99_latency_us = recent[std::min(p99_idx, recent.size() - 1)];

        // Max
        stage.max_latency_us = recent.back();
    }
}

void DashboardManager::updateLinkMetrics() noexcept {
    for (auto& link : links_) {
        auto src_it = std::find_if(stages_.begin(), stages_.end(),
                                   [&](const auto& pair) { return pair.first == link.source_id; });
        auto dst_it = std::find_if(stages_.begin(), stages_.end(),
                                   [&](const auto& pair) { return pair.first == link.target_id; });

        if (src_it == stages_.end() || dst_it == stages_.end()) continue;

        auto& src = src_it->second;
        auto& dst = dst_it->second;

        bool src_active = src.state == StageState::Active;
        bool dst_active = dst.state == StageState::Active;

        link.active = src_active && dst_active;

        // Throughput ratio (animasyon hızı için normalize)
        if (src.peak_messages_per_second > 0) {
            link.throughput_ratio = std::min(1.0,
                src.messages_per_second / 
                src.peak_messages_per_second
            );
        } else {
            link.throughput_ratio = 0.0;
        }

        // Drop rate
        if (src.messages_per_second > 0 && 
            dst.messages_per_second > 0) {
            link.drop_rate = 1.0 - (
                dst.messages_per_second / 
                src.messages_per_second
            );
            link.drop_rate = std::max(0.0, link.drop_rate);
        }
    }
}

void DashboardManager::pruneHistory(int max_seconds) noexcept {
    auto cutoff = std::chrono::steady_clock::now() 
                  - std::chrono::seconds(max_seconds);

    for (auto& [id, history] : throughput_history_) {
        history.erase(
            std::remove_if(history.begin(), history.end(),
                [&](const ThroughputSample& s) { return s.time < cutoff; }),
            history.end()
        );
    }

    for (auto& [id, history] : latency_history_) {
        history.erase(
            std::remove_if(history.begin(), history.end(),
                [&](const LatencySample& s) { return s.time < cutoff; }),
            history.end()
        );
    }
}

// ═══════════════════════════════════════════════════════════════════
// SYSTEM TELEMETRY
// ═══════════════════════════════════════════════════════════════════

void DashboardManager::collectSystemTelemetry() {
    auto pmu = AXIOM::PMUOrchestrator::instance().ReadContext();
    
    // Heisenberg Heatmap: Use L1 Misses and Branch Mispredictions
    system_telemetry_.cpu_usage_percent = (pmu.cycles > 0) ? 
        std::min(100.0, (double)pmu.instructions / pmu.cycles * 50.0) : 10.0;
    
    // [ZENITH]: GPU usage field is reused for Heatmap Intensity (L1 Cache Pressure)
    system_telemetry_.gpu_usage_percent = std::min(100.0, (double)pmu.l1_misses / 500.0);
    system_telemetry_.ipc_bytes_per_sec = AXIOM::TelemetryScribe::instance().read_throughput();

    // [ZENITH]: Memory Orchestrator Health Signals
    auto mem_stats = AXIOM::MemoryOrchestrator::instance().get_system_stats();
    system_telemetry_.ram_usage_percent = static_cast<double>(mem_stats.total_used_bytes) / 
                                         (mem_stats.total_reserved_bytes > 0 ? mem_stats.total_reserved_bytes : 1) * 100.0;
    system_telemetry_.ram_total_bytes = mem_stats.total_reserved_bytes;
    system_telemetry_.ram_used_bytes = mem_stats.total_used_bytes;
    
    // Custom mapping for backpressure visualization
    if (mem_stats.status >= AXIOM::MemoryOrchestrator::HealthStatus::CRITICAL) {
        system_telemetry_.gpu_temp_celsius = 95.0; // "Hot" signal for UI
    } else {
        system_telemetry_.gpu_temp_celsius = 40.0 + mem_stats.fragmentation_avg * 50.0;
    }

#ifdef _WIN32
    if (system_telemetry_.ram_total_bytes == 0) {
        MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem);
        if (GlobalMemoryStatusEx(&mem)) {
            system_telemetry_.ram_usage_percent = mem.dwMemoryLoad;
            system_telemetry_.ram_total_bytes = mem.ullTotalPhys;
        }
    }
#endif
}

// ═══════════════════════════════════════════════════════════════════
// QML MODEL
// ═══════════════════════════════════════════════════════════════════

int DashboardManager::sentryStatus() const noexcept {
    return static_cast<int>(AXIOM::Sentry::instance().get_state());
}

double DashboardManager::scalingMultiplier() const noexcept {
    return static_cast<double>(AXIOM::Pluto::PlutoController::instance().get_scaling_factor());
}

QVariantList DashboardManager::stagesModel() const {
    std::lock_guard lock(stages_mutex_);
    QVariantList list;

    // Sıralı stage listesi
    AXIOM::FixedVector<const StageMetrics*, 32> sorted;
    for (const auto& [id, stage] : stages_) {
        sorted.push_back(&stage);
    }
    std::sort(sorted.begin(), sorted.end(),
        [](const auto* a, const auto* b) { return a->order < b->order; });

    for (const auto* stage : sorted) {
        QVariantMap map;
        map["id"] = QString::fromUtf8(stage->stage_id.data(), stage->stage_id.size());
        map["name"] = QString::fromUtf8(stage->display_name.data(), stage->display_name.size());
        map["order"] = stage->order;
        map["state"] = static_cast<int>(stage->state);
        map["stateName"] = [&]() -> QString {
            switch (stage->state) {
                case StageState::Offline: return "offline";
                case StageState::Idle:    return "idle";
                case StageState::Active:  return "active";
                case StageState::Blocked: return "blocked";
                case StageState::Error:   return "error";
            }
            return "unknown";
        }();
        map["messagesPerSecond"] = stage->messages_per_second;
        map["bytesPerSecond"] = stage->bytes_per_second;
        map["peakMessagesPerSecond"] = stage->peak_messages_per_second;
        map["totalMessages"] = static_cast<qulonglong>(
            stage->total_messages.load());
        map["avgLatencyUs"] = stage->avg_latency_us;
        map["p99LatencyUs"] = stage->p99_latency_us;
        map["maxLatencyUs"] = stage->max_latency_us;
        map["queueCurrent"] = static_cast<qulonglong>(stage->queue_current);
        map["queueCapacity"] = static_cast<qulonglong>(stage->queue_capacity);
        map["queueFillRatio"] = stage->queue_fill_ratio;
        map["uptimeSeconds"] = stage->uptime_seconds;
        map["errorMessage"] = QString::fromUtf8(stage->error_message.data(), stage->error_message.size());

        list.append(map);
    }

    return list;
}

QVariantList DashboardManager::linksModel() const {
    QVariantList list;

    for (const auto& link : links_) {
        QVariantMap map;
        map["sourceId"] = QString::fromUtf8(link.source_id.data(), link.source_id.size());
        map["targetId"] = QString::fromUtf8(link.target_id.data(), link.target_id.size());
        map["throughputRatio"] = link.throughput_ratio;
        map["dropRate"] = link.drop_rate;
        map["active"] = link.active;
        list.append(map);
    }

    return list;
}

QVariantMap DashboardManager::telemetryModel() const {
    QVariantMap map;

    map["cpuUsage"] = system_telemetry_.cpu_usage_percent;
    map["cpuCores"] = system_telemetry_.cpu_cores;
    map["ramTotal"] = static_cast<qulonglong>(system_telemetry_.ram_total_bytes);
    map["ramUsed"] = static_cast<qulonglong>(system_telemetry_.ram_used_bytes);
    map["ramUsage"] = system_telemetry_.ram_usage_percent;
    map["gpuAvailable"] = system_telemetry_.gpu_available;
    map["gpuUsage"] = system_telemetry_.gpu_usage_percent;
    map["vramTotal"] = static_cast<qulonglong>(system_telemetry_.vram_total_bytes);
    map["vramUsed"] = static_cast<qulonglong>(system_telemetry_.vram_used_bytes);
    map["vramUsage"] = system_telemetry_.vram_usage_percent;
    map["gpuTemp"] = system_telemetry_.gpu_temp_celsius;
    map["diskReadBps"] = system_telemetry_.disk_read_bytes_per_sec;
    map["diskWriteBps"] = system_telemetry_.disk_write_bytes_per_sec;
    map["ipcBps"] = system_telemetry_.ipc_bytes_per_sec;
    
    // Memory Arena Stats
    auto pools = AXIOM::PoolManager::instance().get_all_stats();
    QVariantList arenaList;
    for (const auto& stats : pools) {
        QVariantMap arenaMap;
        arenaMap["totalSize"] = static_cast<qulonglong>(stats.total_size);
        arenaMap["usedSize"] = static_cast<qulonglong>(stats.used_size);
        arenaMap["freeSize"] = static_cast<qulonglong>(stats.free_size);
        arenaMap["peakUsage"] = static_cast<qulonglong>(stats.peak_usage);
        arenaMap["allocationCount"] = static_cast<qulonglong>(stats.allocation_count);
        arenaMap["freeCount"] = static_cast<qulonglong>(stats.free_count);
        arenaMap["fragmentationRatio"] = stats.fragmentation_ratio;
        arenaList.append(arenaMap);
    }
    map["arenaStats"] = arenaList;

    // ZenithJIT Stats
    map["jitLastCompileTimeMs"] = AXIOM::ZenithJIT::GetLastCompileTimeMs();

    // [ZENITH SENTRY & SCALING]
    map["sentryStatus"] = sentryStatus();
    map["scalingMultiplier"] = scalingMultiplier();

    // Per-core array
    QVariantList cores;
    for (double usage : system_telemetry_.per_core_usage) {
        cores.append(usage);
    }
    map["perCoreUsage"] = cores;

    return map;
}

double DashboardManager::totalThroughput() const noexcept {
    std::lock_guard lock(stages_mutex_);
    double total = 0;
    for (const auto& [id, stage] : stages_) {
        total += stage.messages_per_second;
    }
    return total;
}

double DashboardManager::totalLatency() const noexcept {
    std::lock_guard lock(stages_mutex_);
    double total = 0;
    for (const auto& [id, stage] : stages_) {
        total += stage.avg_latency_us;
    }
    return total;
}

int DashboardManager::activeStageCount() const noexcept {
    std::lock_guard lock(stages_mutex_);
    int count = 0;
    for (const auto& [id, stage] : stages_) {
        if (stage.state == StageState::Active) count++;
    }
    return count;
}

QVariantList DashboardManager::getThroughputHistory(
    const QString& stage_id, int seconds
) const {
    std::lock_guard lock(stages_mutex_);
    QVariantList list;

    // Create a string_view from QString without dynamic allocation for comparison
    std::string_view stage_id_sv(stage_id.toUtf8().constData(), stage_id.size()); 

    auto it = std::find_if(throughput_history_.begin(), throughput_history_.end(),
                           [&](const auto& pair) { return pair.first == stage_id_sv; });

    if (it == throughput_history_.end()) return list;

    auto cutoff = std::chrono::steady_clock::now() 
                  - std::chrono::seconds(seconds);

    for (const auto& sample : it->second) {
        if (sample.time >= cutoff) {
            QVariantMap point;
            point["time"] = std::chrono::duration<double>(
                sample.time.time_since_epoch()).count();
            point["messages"] = static_cast<qulonglong>(sample.messages);
            point["bytes"] = static_cast<qulonglong>(sample.bytes);
            list.append(point);
        }
    }

    return list;
}

void DashboardManager::startRecording() noexcept {
    recording_ = true;
    emit recordingChanged();
}

void DashboardManager::stopRecording() noexcept {
    recording_ = false;
    emit recordingChanged();
}

void DashboardManager::resetCounters() noexcept {
    std::lock_guard lock(stages_mutex_);
    for (auto& [id, stage] : stages_) {
        stage.total_messages.store(0);
        stage.total_bytes.store(0);
        stage.peak_messages_per_second = 0;
    }
    throughput_history_.clear();
    latency_history_.clear();
    emit stagesUpdated();
}

QVariantList DashboardManager::getLatencyHistory(
    const QString& stage_id, int seconds
) const {
    std::lock_guard lock(stages_mutex_);
    QVariantList list;

    // Create a string_view from QString without dynamic allocation for comparison
    std::string_view stage_id_sv(stage_id.toUtf8().constData(), stage_id.size());

    auto it = std::find_if(latency_history_.begin(), latency_history_.end(),
                           [&](const auto& pair) { return pair.first == stage_id_sv; });

    if (it == latency_history_.end()) return list;

    auto cutoff = std::chrono::steady_clock::now() 
                  - std::chrono::seconds(seconds);

    for (const auto& sample : it->second) {
        if (sample.time >= cutoff) {
            QVariantMap point;
            point["time"] = std::chrono::duration<double>(
                sample.time.time_since_epoch()).count();
            point["latencyUs"] = sample.latency_us;
            list.append(point);
        }
    }

    return list;
}

void DashboardManager::startMockData() noexcept {
    std::cout << "[Dashboard] Initializing mock stages..." << std::endl;
    registerStage("ingress", "Input Source", 0);
    registerStage("pipe", "Named Pipe", 1);
    registerStage("queue", "SPSC Queue", 2);
    registerStage("processor", "Processor", 3);
    registerStage("plotter", "Plotter", 4);

    registerLink("ingress", "pipe");
    registerLink("pipe", "queue");
    registerLink("queue", "processor");
    registerLink("processor", "plotter");

    // Mock data generation timer
    // [ZENITH]: Disabled to prioritize real TelemetryScribe data flow
    /* static QTimer* mockTimer = new QTimer(this);
    connect(mockTimer, &QTimer::timeout, this, [this]() {
        ...
    });
    mockTimer->start(100); */
}

} // namespace axui
