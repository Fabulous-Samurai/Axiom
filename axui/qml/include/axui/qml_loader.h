#pragma once

#include "axui/node.h"
#include "axui/binding_engine.h"
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QUrl>
#include <memory>
#include <unordered_map>
#include <QVariant>

#include <QQuickWindow>
#include "axui/compiler.h"

namespace axui {

struct QmlConfig {
    bool enable_hot_reload = true;
    bool enable_shader_debug = false;
    bool debug_output = false;
    int max_fps = 60;
    std::string window_title = "AXUI Application";
    int window_width = 1280;
    int window_height = 720;
    QQuickWindow* parent_window = nullptr;
};

// UINode → QML Component dönüşümü
class QmlLoader : public QObject {
    Q_OBJECT

public:
    explicit QmlLoader(QObject* parent = nullptr);
    ~QmlLoader() override = default;

    // UINode tree'yi QML'e dönüştür ve göster
    bool loadFromNode(const UINode& root, const QmlConfig& config = QmlConfig());

    // .axui dosyasından direkt yükle (parser + resolver + binding + render)
    bool loadFromFile(const std::string& axui_path, 
                      const std::string& theme_path,
                      const QmlConfig& config = QmlConfig());

private:
    QQmlApplicationEngine qml_engine_;
    BindingEngine* binding_engine_;
    Compiler compiler_;

    // Dinamik QML code generator
    QString generateComponentCode(const UINode& node, int depth = 0);
    
    // Property binding (UINode property → QML property)
    void setupBindings(QQuickItem* item, const UINode& node);

    // Helper for property to QML conversion
    QString convertPropertyToQml(const Property& prop, int depth);
};

// Global utility: QVariant ↔ PropValue dönüşümü
QVariant propertyToQVariant(const PropValue& value);
PropValue qvariantToProperty(const QVariant& variant);

} // namespace axui
