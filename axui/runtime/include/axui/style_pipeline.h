#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantMap>
#include <memory>
#include <string>

#include "lock_free_ring_buffer.h"

namespace axui {

/**
 * @brief Zero-Lag Style Update Structure
 * Contains JSON string or pre-parsed data for QML consumption.
 */
struct StyleUpdate {
  uint64_t sequence_id;
  std::string json_data;
  std::chrono::steady_clock::time_point timestamp;
};

/**
 * @brief StylePipeline: High-performance bridge for JSON-to-QML style updates.
 *
 * Uses SPSCQueue (Lock-Free) to ensure the C++ producer (e.g., from network or
 * logic) never blocks on the Qt Main Thread / Event Loop.
 */
class StylePipeline : public QObject {
  Q_OBJECT
  Q_PROPERTY(int queueSize READ queueSize NOTIFY pipelineUpdated)
  Q_PROPERTY(double lastLatencyMs READ lastLatencyMs NOTIFY pipelineUpdated)

 public:
  explicit StylePipeline(QObject* parent = nullptr);
  ~StylePipeline() override = default;

  /**
   * @brief Push a new style update from any thread.
   * @param json Raw JSON style definition.
   * @return true if successfully queued, false if queue is full (backpressure).
   */
  bool pushUpdate(std::string json);

  int queueSize() const { return static_cast<int>(queue_.size()); }
  double lastLatencyMs() const { return last_latency_ms_; }

 signals:
  /**
   * @brief Emitted when a new style update is successfully processed on the UI
   * thread.
   * @param style Pre-parsed QVariantMap for direct QML property binding.
   */
  void styleChanged(const QVariantMap& style);
  void pipelineUpdated();

 private slots:
  /**
   * @brief Performed on the UI thread to drain the queue.
   */
  void processQueue();

 private:
  // SPSCQueue: Lock-free SPSC buffer (64 entries to balance memory and burst
  // capacity)
  AXIOM::SPSCQueue<StyleUpdate, 64> queue_;

  QTimer* poll_timer_ = nullptr;
  std::atomic<uint64_t> next_sequence_id_{1};
  double last_latency_ms_ = 0.0;

  static constexpr int POLL_INTERVAL_MS =
      8;  // ~120Hz polling for sub-frame response
};

}  // namespace axui
