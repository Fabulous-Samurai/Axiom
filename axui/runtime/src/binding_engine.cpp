#include "axui/binding_engine.h"
#include <chrono>

namespace axui {

// ─── Constructor / Destructor ────────────────────────────────────

BindingEngine::BindingEngine() = default;
BindingEngine::~BindingEngine() = default;

// ─── Tek Kaynak Güncelleme ───────────────────────────────────────

void BindingEngine::updateSource(const std::string& path, BindingValue value) {
    auto start = std::chrono::high_resolution_clock::now();
    bool success = true;

    // 1. Cache'e yaz
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        source_cache_[path] = value;
    }

    // 2. Aboneleri bilgilendir
    {
        std::lock_guard<std::mutex> lock(subs_mutex_);
        auto it = subscriptions_.find(path);
        if (it != subscriptions_.end()) {
            for (auto* prop : it->second) {
                if (!applyValue(*prop, value)) {
                    success = false;
                }
            }
        }
    }

    // 3. İstatistik
    auto end = std::chrono::high_resolution_clock::now();
    double ns = static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    );
    recordUpdate(ns, success);
}

void BindingEngine::updateSource(const std::string& path, double value) {
    updateSource(path, BindingValue{value});
}

void BindingEngine::updateSource(const std::string& path, int64_t value) {
    updateSource(path, BindingValue{value});
}

void BindingEngine::updateSource(const std::string& path, const std::string& value) {
    updateSource(path, BindingValue{value});
}

void BindingEngine::updateSource(const std::string& path, bool value) {
    updateSource(path, BindingValue{value});
}

void BindingEngine::updateSource(const std::string& path, const Color& value) {
    updateSource(path, BindingValue{value});
}

// ─── Batch Güncelleme ────────────────────────────────────────────

void BindingEngine::updateBatch(const std::vector<SourceUpdate>& updates) {
    auto start = std::chrono::high_resolution_clock::now();
    bool all_success = true;

    // 1. Tüm cache güncellemelerini tek lock altında yap
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        for (const auto& u : updates) {
            source_cache_[u.path] = u.value;
        }
    }

    // 2. Tüm aboneleri tek lock altında bilgilendir
    {
        std::lock_guard<std::mutex> lock(subs_mutex_);
        for (const auto& u : updates) {
            auto it = subscriptions_.find(u.path);
            if (it != subscriptions_.end()) {
                for (auto* prop : it->second) {
                    if (!applyValue(*prop, u.value)) {
                        all_success = false;
                    }
                }
            }
        }
    }

    // 3. İstatistik — batch'i tek update olarak say
    auto end = std::chrono::high_resolution_clock::now();
    double ns = static_cast<double>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    );

    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.total_updates += updates.size();
        if (!all_success) stats_.failed_updates++;
        // Ortalama: batch toplam süresini eleman sayısına böl
        double per_item_ns = updates.empty() ? 0 : ns / static_cast<double>(updates.size());
        if (stats_.total_updates <= updates.size()) {
            stats_.avg_update_ns = per_item_ns;
        } else {
            stats_.avg_update_ns = stats_.avg_update_ns * 0.9 + per_item_ns * 0.1;
        }
    }
}

// ─── UINode Tree Binding ─────────────────────────────────────────

void BindingEngine::bindTree(UINode& root) {
    std::lock_guard<std::mutex> lock(subs_mutex_);
    scanNode(root);

    // Aktif subscription sayısını güncelle
    size_t total = 0;
    for (const auto& [path, props] : subscriptions_) {
        total += props.size();
    }
    std::lock_guard<std::mutex> slock(stats_mutex_);
    stats_.active_subscriptions = total;
}

void BindingEngine::scanNode(UINode& node) {
    for (auto& prop : node.properties) {
        if (auto* binding = std::get_if<Binding>(&prop.value)) {
            if (binding->is_bound) {
                // Bu property'yi, binding path'ine abone et
                subscriptions_[std::string(binding->path)].push_back(&prop);

                // Eğer cache'de veri varsa hemen uygula (late binding desteği)
                std::lock_guard<std::mutex> clock(cache_mutex_);
                auto cache_it = source_cache_.find(std::string(binding->path));
                if (cache_it != source_cache_.end()) {
                    applyValue(prop, cache_it->second);
                }
            }
        }
    }

    for (auto* child : node.children) {
        if (child) scanNode(*child);
    }
}

void BindingEngine::clearSubscriptions() {
    std::lock_guard<std::mutex> lock(subs_mutex_);
    subscriptions_.clear();

    std::lock_guard<std::mutex> slock(stats_mutex_);
    stats_.active_subscriptions = 0;
}

// ─── Sorgulama ───────────────────────────────────────────────────

bool BindingEngine::hasSource(const std::string& path) const {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    return source_cache_.find(path) != source_cache_.end();
}

BindingValue BindingEngine::getSource(const std::string& path) const {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    auto it = source_cache_.find(path);
    if (it != source_cache_.end()) {
        return it->second;
    }
    return BindingValue{0.0}; // Default: double 0
}

size_t BindingEngine::subscriptionCount() const {
    std::lock_guard<std::mutex> lock(subs_mutex_);
    size_t total = 0;
    for (const auto& [path, props] : subscriptions_) {
        total += props.size();
    }
    return total;
}

size_t BindingEngine::sourceCount() const {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    return source_cache_.size();
}

BindingStats BindingEngine::statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    BindingStats s = stats_;
    s.source_count = sourceCount();
    s.active_subscriptions = subscriptionCount();
    return s;
}

// ─── Internal Helpers ────────────────────────────────────────────

bool BindingEngine::applyValue(Property& prop, const BindingValue& value) {
    // BindingValue variant'ını Property::value variant'ına dönüştür
    // int64_t → double promotion yapılır (Property variant'ında int64 yok)

    return std::visit([&prop](auto&& arg) -> bool {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, double>) {
            prop.value = arg;
            return true;
        }
        else if constexpr (std::is_same_v<T, int64_t>) {
            prop.value = static_cast<double>(arg);
            return true;
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            prop.value = arg;
            return true;
        }
        else if constexpr (std::is_same_v<T, bool>) {
            prop.value = arg;
            return true;
        }
        else if constexpr (std::is_same_v<T, Color>) {
            prop.value = arg;
            return true;
        }
        else {
            return false;
        }
    }, value);
}

void BindingEngine::recordUpdate(double duration_ns, bool success) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.total_updates++;
    if (!success) stats_.failed_updates++;

    // Exponential moving average
    if (stats_.total_updates == 1) {
        stats_.avg_update_ns = duration_ns;
    } else {
        stats_.avg_update_ns = stats_.avg_update_ns * 0.95 + duration_ns * 0.05;
    }
}

// ─── Global Singleton ────────────────────────────────────────────

BindingEngine& BindingContext::instance() {
    static BindingEngine engine;
    return engine;
}

} // namespace axui
