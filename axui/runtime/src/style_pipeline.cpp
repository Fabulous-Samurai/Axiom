#include "axui/style_pipeline.h"
#include <simdjson.h>
#include <chrono>
#include <iostream>

namespace axui {

StylePipeline::StylePipeline(QObject* parent) : QObject(parent) {
    poll_timer_ = new QTimer(this);
    connect(poll_timer_, &QTimer::timeout, this, &StylePipeline::processQueue);
    poll_timer_->start(POLL_INTERVAL_MS);
}

bool StylePipeline::pushUpdate(std::string json) {
    StyleUpdate update;
    update.sequence_id = next_sequence_id_.fetch_add(1, std::memory_order_relaxed);
    update.json_data = std::move(json);
    update.timestamp = std::chrono::steady_clock::now();
    return queue_.push(std::move(update));
}

void StylePipeline::processQueue() {
    StyleUpdate update;
    simdjson::ondemand::parser parser;

    while (queue_.pop(update)) {
        try {
            simdjson::padded_string padded(update.json_data);
            auto doc = parser.iterate(padded);
            auto obj = doc.get_object().value();

            QVariantMap style;
            for (auto field : obj) {
                std::string_view key = field.unescaped_key().value();
                auto val = field.value();
                auto type = val.type().value();

                QString qKey = QString::fromUtf8(key.data(), static_cast<int>(key.size()));

                if (type == simdjson::ondemand::json_type::string) {
                    std::string_view sv = val.get_string().value();
                    style[qKey] = QString::fromUtf8(sv.data(), static_cast<int>(sv.size()));
                } else if (type == simdjson::ondemand::json_type::number) {
                    style[qKey] = val.get_double().value();
                } else if (type == simdjson::ondemand::json_type::boolean) {
                    style[qKey] = val.get_bool().value();
                }
            }

            auto now = std::chrono::steady_clock::now();
            last_latency_ms_ = std::chrono::duration_cast<std::chrono::microseconds>(now - update.timestamp).count() / 1000.0;
            
            emit styleChanged(style);
            emit pipelineUpdated();

        } catch (std::exception& e) {
            std::cerr << "[AXUI] Simdjson Style Error: " << e.what() << std::endl;
        }
    }
}

} // namespace axui
