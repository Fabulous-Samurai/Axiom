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

namespace axui {

struct QmlConfig {
    bool enable_hot_reload = true;
    bool enable_shader_debug = false;
    int max_fps = 60;
    std::string window_title = "AXUI Application";
    int window_width = 1280;
    int window_height = 720;
};

// UINode → QML Component dönüşümü
class QmlLoader : public QObject {
    Q_OBJECT

public:
    explicit QmlLoader(QObject* parent = nullptr);
    ~QmlLoader() override;

    // UINode tree'yi QML'e dönüştür ve göster
    bool loadFromNode(const UINode& root, const QmlConfig& config = QmlConfig());

    // .axui dosyasından direkt yükle (parser + resolver + binding + render)
    bool loadFromFile(const std::string& axui_path, 
                      const std::string& theme_path,
                      const QmlConfig& config = QmlConfig());

    // Binding engine erişimi (runtime veri güncellemesi için)
    BindingEngine* bindingEngine() { return binding_engine_.get(); }

    // Hot-reload tetikle (F5 veya file watcher)
    void reload();

    // QML engine erişimi (debugging için)
    QQmlApplicationEngine* engine() { return engine_.get(); }

signals:
    // QML tarafından emit edilecek sinyaller
    void componentLoaded(const QString& component_name);
    void componentError(const QString& error);
    void bindingUpdated(const QString& path, const QVariant& value);

private:
    std::unique_ptr<QQmlApplicationEngine> engine_;
    std::unique_ptr<BindingEngine> binding_engine_;

    // UINode → QML Component map (cache)
    std::unordered_map<std::string, QUrl> component_cache_;

    // Dinamik QML code generator
    QString generateQmlCode(const UINode& node);
    QString generateComponentCode(const UINode& node, int depth = 0);
    
    // Shader generator (blur, glow, noise için)
    QString generateBlurShader(const GlassParams& glass);
    QString generateGlowShader(const HoverParams& hover);
    QString generateNoiseShader(float strength);

    // Property binding (UINode property → QML property)
    void setupBindings(QQuickItem* item, const UINode& node);
    void connectBindingEngine();

    // Hot-reload internals
    QUrl last_loaded_url_;
    std::string last_axui_path_;
    std::string last_theme_path_;

    // Helper for property to QML conversion
    QString convertPropertyToQml(const Property& prop, int depth);
};

// Global utility: QVariant ↔ PropValue dönüşümü
QVariant propertyToQVariant(const PropValue& value);
PropValue qvariantToProperty(const QVariant& variant);

} // namespace axui
