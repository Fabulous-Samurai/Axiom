#pragma once

#include "axui/node.h"
#include <QQmlEngine>
#include <QString>
#include <functional>
#include <unordered_map>
#include <memory>
#include <optional>

namespace axui {

// Component metadata
struct ComponentMeta {
    QString qml_type;           // QML type name (e.g., "AXKPICard")
    QString qml_module;         // QML module (e.g., "AXUI.Components")
    QString qml_file;           // QML file path (e.g., "qrc:/axui/components/KPICard.qml")
    int version_major = 1;
    int version_minor = 0;

    // Required properties
    std::vector<std::string> required_props;

    // Default property values
    std::unordered_map<std::string, PropValue> defaults;

    // Supported effects
    bool supports_glass = true;
    bool supports_hover = true;
    bool supports_animation = true;
};

// Component factory function type
using ComponentFactory = std::function<QString(const UINode&, int depth)>;

// Component Registry - Singleton
class ComponentRegistry {
public:
    static ComponentRegistry& instance();

    // Component kaydı
    void registerComponent(const std::string& axui_name, ComponentMeta meta);
    void registerComponent(const std::string& axui_name, ComponentMeta meta,
                           ComponentFactory factory);

    // Component lookup
    std::optional<ComponentMeta> getMeta(const std::string& axui_name) const;
    std::optional<ComponentFactory> getFactory(const std::string& axui_name) const;

    // QML engine'e tüm component'leri kaydet
    void registerAllToEngine(QQmlEngine* engine);

    // Kayıtlı component listesi
    std::vector<std::string> registeredComponents() const;

    // Built-in component'leri yükle
    void loadBuiltinComponents();

private:
    ComponentRegistry() = default;
    ~ComponentRegistry() = default;

    ComponentRegistry(const ComponentRegistry&) = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;

    std::unordered_map<std::string, ComponentMeta> meta_registry_;
    std::unordered_map<std::string, ComponentFactory> factory_registry_;
    bool builtins_loaded_ = false;
};

// Macro: Component registration helper
#define AXUI_REGISTER_COMPONENT(axui_name, qml_type, qml_module) \
    static bool _reg_##axui_name = []() { \
        ComponentRegistry::instance().registerComponent(#axui_name, { \
            .qml_type = #qml_type, \
            .qml_module = #qml_module, \
            .qml_file = "qrc:/axui/components/" #qml_type ".qml" \
        }); \
        return true; \
    }()

} // namespace axui
