#pragma once

#include "axui/node.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <memory>
#include <variant>
#include <vector>
#include <chrono>
#include <numeric>

namespace axui {

// Runtime veri kaynağı tipi
// Property variant'ından BAĞIMSIZ — dışarıdan gelen ham veri
using BindingValue = std::variant<double, int64_t, std::string, bool, Color>;

// Batch update için tek yapı
struct SourceUpdate {
    std::string path;
    BindingValue value;
};

// İstatistik yapısı
struct BindingStats {
    size_t total_updates = 0;          // updateSource çağrı sayısı
    size_t active_subscriptions = 0;   // Bağlı property sayısı
    size_t source_count = 0;           // Kayıtlı veri kaynağı sayısı
    size_t failed_updates = 0;         // Başarısız güncelleme sayısı
    double avg_update_ns = 0;          // Ortalama güncelleme süresi (ns)
};

class BindingEngine {
public:
    BindingEngine();
    ~BindingEngine();

    // ─── Veri Kaynağı Güncelleme (Thread-Safe) ────────────────────

    // Tek kaynak güncelle
    void updateSource(const std::string& path, BindingValue value);

    // Tip-spesifik kısayollar
    void updateSource(const std::string& path, double value);
    void updateSource(const std::string& path, int64_t value);
    void updateSource(const std::string& path, const std::string& value);
    void updateSource(const std::string& path, bool value);
    void updateSource(const std::string& path, const Color& value);

    // Çoklu kaynak güncelle — TEK lock alır (batch optimizasyon)
    void updateBatch(const std::vector<SourceUpdate>& updates);

    // ─── UINode Tree Binding ──────────────────────────────────────

    // Tüm tree'yi tara, @src\daemon_engine.cpp Binding'leri subscribe et
    void bindTree(UINode& root);

    // Tüm subscription'ları temizle (tree yeniden bind edilecekse)
    void clearSubscriptions();

    // ─── Sorgulama ────────────────────────────────────────────────

    // Kayıtlı veri kaynağı var mı?
    bool hasSource(const std::string& path) const;

    // Kayıtlı veri kaynağını oku
    BindingValue getSource(const std::string& path) const;

    // Aktif subscription sayısı (toplam bağlı property)
    size_t subscriptionCount() const;

    // Kayıtlı veri kaynağı sayısı
    size_t sourceCount() const;

    // İstatistikler
    BindingStats statistics() const;

private:
    // Path → bağlı Property pointer listesi
    mutable std::mutex subs_mutex_;
    std::unordered_map<std::string, std::vector<Property*>> subscriptions_;

    // Path → son bilinen değer (cache)
    mutable std::mutex cache_mutex_;
    std::unordered_map<std::string, BindingValue> source_cache_;

    // İstatistik (atomic olmasa da mutex altında güncellenir)
    mutable std::mutex stats_mutex_;
    BindingStats stats_;

    // Recursive tree scan
    void scanNode(UINode& node);

    // BindingValue → Property::value dönüşümü
    bool applyValue(Property& prop, const BindingValue& value);

    // Belirli path'e abone olan property'leri güncelle (lock DIŞINDA çağrılır)
    void notifyPath(const std::string& path, const BindingValue& value);

    // Stats helper
    void recordUpdate(double duration_ns, bool success);
};

// Global singleton erişimi
class BindingContext {
public:
    static BindingEngine& instance();
private:
    BindingContext() = default;
};

} // namespace axui
