#pragma once

#include <QObject>
#include <QString>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <numeric>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "axui/node.h"

namespace axui {

// Runtime veri kaynağı tipi
using BindingValue = std::variant<double, int64_t, std::string, bool, Color>;

// Batch update için tek yapı
struct SourceUpdate {
  std::string path;
  BindingValue value;
};

// İstatistik yapısı
struct BindingStats {
  size_t total_updates = 0;
  size_t active_subscriptions = 0;
  size_t source_count = 0;
  size_t failed_updates = 0;
  double avg_update_ns = 0;
};

class BindingEngine : public QObject {
  Q_OBJECT

 public:
  BindingEngine();
  ~BindingEngine() override;

  // Tek kaynak güncelle
  void updateSource(const std::string& path, BindingValue value);
  void updateSource(const std::string& path, double value);
  void updateSource(const std::string& path, int64_t value);
  void updateSource(const std::string& path, const std::string& value);
  void updateSource(const std::string& path, bool value);
  void updateSource(const std::string& path, const Color& value);

  // Çoklu kaynak güncelle
  void updateBatch(const std::vector<SourceUpdate>& updates);

  // UINode Tree Binding
  void bindTree(UINode& root);
  void clearSubscriptions();

  // Sorgulama
  bool hasSource(const std::string& path) const;
  BindingValue getSource(const std::string& path) const;
  size_t subscriptionCount() const;
  size_t sourceCount() const;
  BindingStats statistics() const;

 signals:
  // Emitted when a data source updates
  void sourceUpdated(const QString& path, const BindingValue& value);

 private:
  mutable std::mutex subs_mutex_;
  std::unordered_map<std::string, std::vector<Property*>> subscriptions_;

  mutable std::mutex cache_mutex_;
  std::unordered_map<std::string, BindingValue> source_cache_;

  mutable std::mutex stats_mutex_;
  BindingStats stats_;

  void scanNode(UINode& node);
  bool applyValue(Property& prop, const BindingValue& value);
  void notifyPath(const std::string& path, const BindingValue& value);
  void recordUpdate(double duration_ns, bool success);
};

// Global singleton erişimi
class BindingContext {
 public:
  static BindingEngine& instance();

 private:
  BindingContext() = default;
};

}  // namespace axui
