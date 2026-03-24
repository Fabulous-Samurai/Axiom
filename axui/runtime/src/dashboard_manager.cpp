#include "axui/dashboard_manager.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <fstream>
#include <iostream>

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
    const std::string& stage_id,
    const std::string& display_name,
    int order
) {
    std::lock_guard lock(stages_mutex_);

    auto [it, inserted] = stages_.try_emplace(stage_id);
    auto& metrics = it->second;

    metrics.stage_id = stage_id;
    metrics.display_name = display_name;
    metrics.order = order;
    metrics.state = StageState::Idle;
    metrics.started_at = std::chrono::steady_clock::now();
    metrics.total_messages.store(0);
    metrics.total_bytes.store(0);
}

void DashboardManager::registerLink(
    const std::string& source_id,
    const std::string& target_id
) {
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
    const std::string& stage_id, 
    uint64_t byte_count
) {
    std::lock_guard lock(stages_mutex_);
    
    auto it = stages_.find(stage_id);
    if (it == stages_.end()) return;

    it->second.total_messages.fetch_add(1, std::memory_order_relaxed);
    it->second.total_bytes.fetch_add(byte_count, std::memory_order_relaxed);
    it->second.last_activity = std::chrono::steady_clock::now();

    if (it->second.state == StageState::Idle) {
        it->second.state = StageState::Active;
    }

    // Throughput history
    auto it_history = throughput_history_.find(stage_id);
    if (it_history == throughput_history_.end()) {
        it_history = throughput_history_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(stage_id),
            std::forward_as_tuple(AXIOM::ArenaAllocator<ThroughputSample>(&throughput_arena_))
        ).first;
    }

    it_history->second.push_back({
        std::chrono::steady_clock::now(),
        it->second.total_messages.load(),
        it->second.total_bytes.load()
    });
}

void DashboardManager::recordLatency(
    const std::string& stage_id, 
    double latency_us
) {
    std::lock_guard lock(stages_mutex_);

    // Latency history
    auto it_history = latency_history_.find(stage_id);
    if (it_history == latency_history_.end()) {
        it_history = latency_history_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(stage_id),
            std::forward_as_tuple(AXIOM::ArenaAllocator<LatencySample>(&latency_arena_))
        ).first;
    }

    it_history->second.push_back({
        std::chrono::steady_clock::now(),
        latency_us
    });
}

void DashboardManager::updateStageState(
    const std::string& stage_id, 
    StageState state
) {
    std::lock_guard lock(stages_mutex_);
    
    auto it = stages_.find(stage_id);
    if (it != stages_.end()) {
        it->second.state = state;
    }
}

void DashboardManager::updateQueueStatus(
    const std::string& stage_id,
    uint64_t current, 
    uint64_t capacity
) {
    std::lock_guard lock(stages_mutex_);
    
    auto it = stages_.find(stage_id);
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
    const std::string& stage_id, 
    const std::string& error_msg
) {
    std::lock_guard lock(stages_mutex_);
    
    auto it = stages_.find(stage_id);
    if (it != stages_.end()) {
        it->second.state = StageState::Error;
        it->second.error_message = error_msg;
    }

    emit errorOccurred(
        QString::fromStdString(stage_id),
        QString::fromStdString(error_msg)
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

    emit stagesUpdated();
    emit linksUpdated();
    emit telemetryUpdated();
}

void DashboardManager::calculateThroughput() {
    auto now = std::chrono::steady_clock::now();

    for (auto& [id, stage] : stages_) {
        auto& history = throughput_history_[id];
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

void DashboardManager::calculateLatencyStats() {
    auto now = std::chrono::steady_clock::now();
    auto window = now - std::chrono::seconds(10);

    for (auto& [id, stage] : stages_) {
        auto& history = latency_history_[id];

        // Son 10 saniyelik verileri filtrele
        std::vector<double> recent;
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

void DashboardManager::updateLinkMetrics() {
    for (auto& link : links_) {
        auto src = stages_.find(link.source_id);
        auto dst = stages_.find(link.target_id);

        if (src == stages_.end() || dst == stages_.end()) continue;

        bool src_active = src->second.state == StageState::Active;
        bool dst_active = dst->second.state == StageState::Active;

        link.active = src_active && dst_active;

        // Throughput ratio (animasyon hızı için normalize)
        if (src->second.peak_messages_per_second > 0) {
            link.throughput_ratio = std::min(1.0,
                src->second.messages_per_second / 
                src->second.peak_messages_per_second
            );
        } else {
            link.throughput_ratio = 0.0;
        }

        // Drop rate
        if (src->second.messages_per_second > 0 && 
            dst->second.messages_per_second > 0) {
            link.drop_rate = 1.0 - (
                dst->second.messages_per_second / 
                src->second.messages_per_second
            );
            link.drop_rate = std::max(0.0, link.drop_rate);
        }
    }
}

void DashboardManager::pruneHistory(int max_seconds) {
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
#ifdef __linux__
    // CPU Usage
    static long prev_idle = 0, prev_total = 0;

    std::ifstream stat("/proc/stat");
    std::string cpu_label;
    long user, nice, system, idle, iowait, irq, softirq, steal;

    stat >> cpu_label >> user >> nice >> system >> idle 
         >> iowait >> irq >> softirq >> steal;

    long total = user + nice + system + idle + iowait + irq + softirq + steal;
    long total_diff = total - prev_total;
    long idle_diff = idle - prev_idle;

    if (total_diff > 0) {
        system_telemetry_.cpu_usage_percent = 
            100.0 * (1.0 - static_cast<double>(idle_diff) / total_diff);
    }

    prev_idle = idle;
    prev_total = total;

    // Per-core usage
    system_telemetry_.per_core_usage.clear();
    std::string line;
    while (std::getline(stat, line)) {
        if (line.substr(0, 3) == "cpu" && line[3] != ' ') {
            // Parse per-core (simplified)
            system_telemetry_.per_core_usage.push_back(
                system_telemetry_.cpu_usage_percent  // Placeholder
            );
        }
    }
    system_telemetry_.cpu_cores = system_telemetry_.per_core_usage.size();

    // Memory
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        system_telemetry_.ram_total_bytes = si.totalram * si.mem_unit;
        system_telemetry_.ram_used_bytes = 
            (si.totalram - si.freeram) * si.mem_unit;
        system_telemetry_.ram_usage_percent = 
            100.0 * system_telemetry_.ram_used_bytes / 
            system_telemetry_.ram_total_bytes;
    }

    // GPU (nvidia-smi veya /sys/class/drm üzerinden)
    std::ifstream gpu_usage("/sys/class/drm/card0/device/gpu_busy_percent");
    if (gpu_usage.is_open()) {
        system_telemetry_.gpu_available = true;
        gpu_usage >> system_telemetry_.gpu_usage_percent;
    }

    // GPU Temperature
    std::ifstream gpu_temp("/sys/class/drm/card0/device/hwmon/hwmon0/temp1_input");
    if (gpu_temp.is_open()) {
        double temp_millicelsius;
        gpu_temp >> temp_millicelsius;
        system_telemetry_.gpu_temp_celsius = temp_millicelsius / 1000.0;
    }
#endif
}

// ═══════════════════════════════════════════════════════════════════
// QML MODEL
// ═══════════════════════════════════════════════════════════════════

QVariantList DashboardManager::stagesModel() const {
    std::lock_guard lock(stages_mutex_);
    QVariantList list;

    // Sıralı stage listesi
    std::vector<const StageMetrics*> sorted;
    for (const auto& [id, stage] : stages_) {
        sorted.push_back(&stage);
    }
    std::sort(sorted.begin(), sorted.end(),
        [](const auto* a, const auto* b) { return a->order < b->order; });

    for (const auto* stage : sorted) {
        QVariantMap map;
        map["id"] = QString::fromStdString(stage->stage_id);
        map["name"] = QString::fromStdString(stage->display_name);
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
        map["errorMessage"] = QString::fromStdString(stage->error_message);

        list.append(map);
    }

    return list;
}

QVariantList DashboardManager::linksModel() const {
    QVariantList list;

    for (const auto& link : links_) {
        QVariantMap map;
        map["sourceId"] = QString::fromStdString(link.source_id);
        map["targetId"] = QString::fromStdString(link.target_id);
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

    // Per-core array
    QVariantList cores;
    for (double usage : system_telemetry_.per_core_usage) {
        cores.append(usage);
    }
    map["perCoreUsage"] = cores;

    return map;
}

double DashboardManager::totalThroughput() const {
    std::lock_guard lock(stages_mutex_);
    double total = 0;
    for (const auto& [id, stage] : stages_) {
        total += stage.messages_per_second;
    }
    return total;
}

double DashboardManager::totalLatency() const {
    std::lock_guard lock(stages_mutex_);
    double total = 0;
    for (const auto& [id, stage] : stages_) {
        total += stage.avg_latency_us;
    }
    return total;
}

int DashboardManager::activeStageCount() const {
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

    auto it = throughput_history_.find(stage_id.toStdString());
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

void DashboardManager::startRecording() {
    recording_ = true;
    emit recordingChanged();
}

void DashboardManager::stopRecording() {
    recording_ = false;
    emit recordingChanged();
}

void DashboardManager::resetCounters() {
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

    auto it = latency_history_.find(stage_id.toStdString());
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

void DashboardManager::startMockData() {
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
    static QTimer* mockTimer = new QTimer(this);
    connect(mockTimer, &QTimer::timeout, this, [this]() {
        static double t = 0;
        t += 0.1;

        // Simulate varying throughput
        recordMessage("ingress", 1024 + 512 * std::sin(t));
        if (std::sin(t*0.5) > -0.5) recordMessage("pipe", 1024 + 256 * std::cos(t));
        if (std::sin(t*0.7) > -0.3) recordMessage("queue", 1024 + 128 * std::sin(t*1.1));
        if (std::sin(t*0.9) > -0.1) recordMessage("processor", 1024 + 64 * std::cos(t*0.8));
        recordMessage("plotter", 1024 + 32 * std::sin(t*2.0));

        updateQueueStatus("queue", 500 + 450 * std::sin(t * 1.2), 1000);
        
        recordLatency("processor", 150 + 50 * std::cos(t));
        recordLatency("plotter", 50 + 10 * std::sin(t * 2));

        // Simulate system telemetry (Sprint 1 Smoothing)
        double raw_cpu = 45.0 + 15.0 * std::sin(t * 0.3);
        double raw_gpu = 30.0 + 20.0 * std::cos(t * 0.4);
        
        cpu_ema_ = (ema_alpha_ * raw_cpu) + ((1.0 - ema_alpha_) * cpu_ema_);
        gpu_ema_ = (ema_alpha_ * raw_gpu) + ((1.0 - ema_alpha_) * gpu_ema_);

        system_telemetry_.cpu_usage_percent = cpu_ema_;
        system_telemetry_.gpu_usage_percent = gpu_ema_;
        system_telemetry_.gpu_available = true;

        system_telemetry_.ram_total_bytes = 16ULL * 1024 * 1024 * 1024;
        system_telemetry_.ram_used_bytes = 8ULL * 1024 * 1024 * 1024 + (1ULL * 1024 * 1024 * 1024 * std::sin(t * 0.1));
        
        double raw_ram_percent = 100.0 * system_telemetry_.ram_used_bytes / system_telemetry_.ram_total_bytes;
        ram_ema_ = (ema_alpha_ * raw_ram_percent) + ((1.0 - ema_alpha_) * ram_ema_);
        system_telemetry_.ram_usage_percent = ram_ema_;
        
        system_telemetry_.ipc_bytes_per_sec = 250.0 * 1024 * 1024 + (50.0 * 1024 * 1024 * std::sin(t * 0.2));

        emit telemetryUpdated();
    });
    mockTimer->start(100); // 10Hz mock generation for stability
}

} // namespace axui
