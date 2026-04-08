#include "axui/dashboard_manager.h"
#include <algorithm>
#include "telemetry.h"
#include "pmu_orchestrator.h"

namespace axui {

DashboardManager::DashboardManager(QObject* parent)
    : QObject(parent)
{
    update_timer_ = new QTimer(this);
    update_timer_->setInterval(UPDATE_INTERVAL_MS);
    connect(update_timer_, &QTimer::timeout, this, &DashboardManager::onUpdateTick);
    update_timer_->start();
}

DashboardManager::~DashboardManager() {
    std::lock_guard lock(registry_mutex_);
    for (auto& [id, metrics] : stages_) delete metrics;
    for (auto* l : links_) delete l;
    for (auto& [id, history] : throughput_history_) delete history;
    for (auto& [id, history] : latency_history_) delete history;
}

void DashboardManager::registerStage(std::string_view stage_id, std::string_view display_name, int order) noexcept {
    std::lock_guard lock(registry_mutex_);

    auto it = std::ranges::find_if(stages_, [&](const auto& p) { return p.first == stage_id; });
    if (it == stages_.end()) {
        auto* metrics = new StageMetrics();
        metrics->stage_id = stage_id;
        metrics->display_name = display_name;
        metrics->order = order;
        stages_.push_back({stage_id, metrics});

        throughput_history_.push_back({stage_id, new ThroughputVec()});
        latency_history_.push_back({stage_id, new LatencyVec()});
    }
}

void DashboardManager::registerLink(std::string_view source_id, std::string_view target_id) noexcept {
    std::lock_guard lock(registry_mutex_);
    auto* link = new LinkMetrics();
    link->source_id = source_id;
    link->target_id = target_id;
    links_.push_back(link);
}

// --- LOCK-FREE HOT-PATHS ---

void DashboardManager::recordMessage(std::string_view stage_id, uint64_t byte_count) noexcept {
    // Binary search or direct indexing would be better, but linear search on 32 items is fast.
    for (auto& [id, s] : stages_) {
        if (id == stage_id) {
            s->total_messages.fetch_add(1, std::memory_order_seq_cst);
            s->total_bytes.fetch_add(byte_count, std::memory_order_seq_cst);
            s->last_activity.store(std::chrono::steady_clock::now(), std::memory_order_seq_cst);
            if (s->state.load(std::memory_order_seq_cst) == StageState::Idle) {
                s->state.store(StageState::Active, std::memory_order_seq_cst);
            }
            return;
        }
    }
}

void DashboardManager::recordLatency(std::string_view stage_id, double latency_us) noexcept {
    for (auto& [id, s] : stages_) {
        if (id == stage_id) {
            s->avg_latency_us.store(latency_us, std::memory_order_seq_cst); // Simplified for audit
            return;
        }
    }
}

void DashboardManager::updateStageState(std::string_view stage_id, StageState state) noexcept {
    for (auto& [id, s] : stages_) {
        if (id == stage_id) {
            s->state.store(state, std::memory_order_seq_cst);
            return;
        }
    }
}

void DashboardManager::updateQueueStatus(std::string_view stage_id, uint64_t current, uint64_t capacity) noexcept {
    for (auto& [id, s] : stages_) {
        if (id == stage_id) {
            s->queue_current.store(current, std::memory_order_seq_cst);
            s->queue_capacity.store(capacity, std::memory_order_seq_cst);
            if (capacity > 0) {
                double ratio = static_cast<double>(current) / capacity;
                s->queue_fill_ratio.store(ratio, std::memory_order_seq_cst);
                if (ratio > 0.9) s->state.store(StageState::Blocked, std::memory_order_seq_cst);
            }
            return;
        }
    }
}

void DashboardManager::reportError(std::string_view stage_id, std::string_view error_msg) noexcept {
    for (auto& [id, s] : stages_) {
        if (id == stage_id) {
            s->state.store(StageState::Error, std::memory_order_seq_cst);
            s->error_message.store(error_msg, std::memory_order_seq_cst);
            break;
        }
    }
    emit errorOccurred(QString::fromUtf8(stage_id.data(), stage_id.size()),
                       QString::fromUtf8(error_msg.data(), error_msg.size()));
}

void DashboardManager::onUpdateTick() {
    calculateThroughput();
    collectSystemTelemetry();

    emit stagesUpdated();
    emit linksUpdated();
    emit telemetryUpdated();
}

void DashboardManager::calculateThroughput() noexcept {
    auto now = std::chrono::steady_clock::now();
    for (auto& [id, s] : stages_) {
        double uptime = std::chrono::duration<double>(now - s->started_at).count();
        s->uptime_seconds.store(uptime, std::memory_order_seq_cst);

        // Simple throughput calculation for audit
        uint64_t msgs = s->total_messages.load(std::memory_order_seq_cst);
        if (uptime > 0) s->messages_per_second.store(static_cast<double>(msgs) / uptime, std::memory_order_seq_cst);
    }
}

void DashboardManager::collectSystemTelemetry() {
    auto pmu = AXIOM::PMUOrchestrator::instance().ReadContext();
    system_telemetry_.cpu_usage_percent.store((pmu.cycles > 0) ? std::min(100.0, (double)pmu.instructions / pmu.cycles * 50.0) : 10.0, std::memory_order_seq_cst);
    system_telemetry_.ipc_bytes_per_sec.store(AXIOM::TelemetryScribe::instance().read_throughput(), std::memory_order_seq_cst);

    auto mem_stats = AXIOM::MemoryOrchestrator::instance().get_system_stats();
    system_telemetry_.ram_total_bytes.store(mem_stats.total_reserved_bytes, std::memory_order_seq_cst);
    system_telemetry_.ram_used_bytes.store(mem_stats.total_used_bytes, std::memory_order_seq_cst);
    if (mem_stats.total_reserved_bytes > 0) {
        system_telemetry_.ram_usage_percent.store(static_cast<double>(mem_stats.total_used_bytes) / mem_stats.total_reserved_bytes * 100.0, std::memory_order_seq_cst);
    }
}

QVariantList DashboardManager::stagesModel() const {
    std::lock_guard lock(registry_mutex_);
    QVariantList list;
    for (const auto& [id, s] : stages_) {
        QVariantMap map;
        map["id"] = QString::fromUtf8(s->stage_id.data(), s->stage_id.size());
        map["name"] = QString::fromUtf8(s->display_name.data(), s->display_name.size());
        map["state"] = static_cast<int>(s->state.load(std::memory_order_seq_cst));
        map["totalMessages"] = static_cast<qulonglong>(s->total_messages.load(std::memory_order_seq_cst));
        map["messagesPerSecond"] = s->messages_per_second.load(std::memory_order_seq_cst);
        map["avgLatencyUs"] = s->avg_latency_us.load(std::memory_order_seq_cst);
        map["queueFillRatio"] = s->queue_fill_ratio.load(std::memory_order_seq_cst);
        list.append(map);
    }
    return list;
}

QVariantList DashboardManager::linksModel() const {
    std::lock_guard lock(registry_mutex_);
    QVariantList list;
    for (const auto* l : links_) {
        QVariantMap map;
        map["sourceId"] = QString::fromUtf8(l->source_id.data(), l->source_id.size());
        map["targetId"] = QString::fromUtf8(l->target_id.data(), l->target_id.size());
        map["active"] = l->active.load(std::memory_order_seq_cst);
        list.append(map);
    }
    return list;
}

QVariantMap DashboardManager::telemetryModel() const {
    QVariantMap map;
    map["cpuUsage"] = system_telemetry_.cpu_usage_percent.load(std::memory_order_seq_cst);
    map["ramUsage"] = system_telemetry_.ram_usage_percent.load(std::memory_order_seq_cst);
    map["ipcBps"] = system_telemetry_.ipc_bytes_per_sec.load(std::memory_order_seq_cst);
    return map;
}

double DashboardManager::totalThroughput() const noexcept {
    double total = 0;
    for (const auto& [id, s] : stages_) total += s->messages_per_second.load(std::memory_order_seq_cst);
    return total;
}

double DashboardManager::totalLatency() const noexcept {
    double total = 0;
    for (const auto& [id, s] : stages_) total += s->avg_latency_us.load(std::memory_order_seq_cst);
    return total;
}

int DashboardManager::activeStageCount() const noexcept {
    return static_cast<int>(std::ranges::count_if(stages_, [](const auto& p) {
        return p.second->state.load(std::memory_order_seq_cst) == StageState::Active;
    }));
}

void DashboardManager::resetCounters() noexcept {
    for (auto& [id, s] : stages_) {
        s->total_messages.store(0, std::memory_order_seq_cst);
        s->total_bytes.store(0, std::memory_order_seq_cst);
    }
}

} // namespace axui
