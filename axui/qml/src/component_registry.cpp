#include "axui/component_registry.h"
#include <QQmlContext>
#include <QDebug>

namespace axui {

ComponentRegistry& ComponentRegistry::instance() {
    static ComponentRegistry instance;
    return instance;
}

void ComponentRegistry::registerComponent(const std::string& axui_name, ComponentMeta meta) {
    meta_registry_[axui_name] = std::move(meta);
}

void ComponentRegistry::registerComponent(
    const std::string& axui_name, 
    ComponentMeta meta,
    ComponentFactory factory
) {
    meta_registry_[axui_name] = std::move(meta);
    factory_registry_[axui_name] = std::move(factory);
}

std::optional<ComponentMeta> ComponentRegistry::getMeta(const std::string& axui_name) const {
    auto it = meta_registry_.find(axui_name);
    if (it != meta_registry_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<ComponentFactory> ComponentRegistry::getFactory(const std::string& axui_name) const {
    auto it = factory_registry_.find(axui_name);
    if (it != factory_registry_.end()) {
        return it->second;
    }
    return std::nullopt;
}

void ComponentRegistry::registerAllToEngine(QQmlEngine* engine) {
    if (!engine) return;
    
    // QML import path ekle
    engine->addImportPath("qrc:/axui");
    
    // Her component'i QML'e kaydet
    for (const auto& [name, meta] : meta_registry_) {
        qDebug() << "Registering component:" << meta.qml_type 
                 << "from" << meta.qml_module;
    }
}

std::vector<std::string> ComponentRegistry::registeredComponents() const {
    std::vector<std::string> components;
    components.reserve(meta_registry_.size());
    
    for (const auto& [name, _] : meta_registry_) {
        components.push_back(name);
    }
    
    return components;
}

void ComponentRegistry::loadBuiltinComponents() {
    if (builtins_loaded_) return;
    
    // ─── Layout Components ───────────────────────────────────────
    registerComponent("Column", {
        .qml_type = "AXColumn",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/layout/AXColumn.qml",
        .required_props = {},
        .defaults = {{"spacing", 8.0}},
        .supports_glass = false,
        .supports_hover = false
    });
    
    registerComponent("Row", {
        .qml_type = "AXRow",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/layout/AXRow.qml",
        .required_props = {},
        .defaults = {{"spacing", 8.0}},
        .supports_glass = false,
        .supports_hover = false
    });
    
    registerComponent("Grid", {
        .qml_type = "AXGrid",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/layout/AXGrid.qml",
        .required_props = {},
        .defaults = {{"columns", 2.0}, {"spacing", 8.0}},
        .supports_glass = false,
        .supports_hover = false
    });
    
    registerComponent("Stack", {
        .qml_type = "AXStack",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/layout/AXStack.qml",
        .required_props = {},
        .defaults = {},
        .supports_glass = false,
        .supports_hover = false
    });
    
    // ─── Data Display Components ─────────────────────────────────
    registerComponent("KPICard", {
        .qml_type = "AXKPICard",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/AXKPICard.qml",
        .required_props = {"title", "value"},
        .defaults = {
            {"trend", 0.0},
            {"trendDirection", std::string("up")},
            {"icon", std::string("")}
        },
        .supports_glass = true,
        .supports_hover = true
    });
    
    registerComponent("DataGrid", {
        .qml_type = "AXDataGrid",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/AXDataGrid.qml",
        .required_props = {"columns", "data"},
        .defaults = {
            {"sortable", true},
            {"filterable", false},
            {"rowHeight", 48.0}
        },
        .supports_glass = true,
        .supports_hover = true
    });
    
    registerComponent("Chart", {
        .qml_type = "AXChart",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/AXChart.qml",
        .required_props = {"type", "data"},
        .defaults = {
            {"showLegend", true},
            {"showGrid", true},
            {"animated", true}
        },
        .supports_glass = true,
        .supports_hover = false
    });
    
    registerComponent("Gauge", {
        .qml_type = "AXGauge",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/AXGauge.qml",
        .required_props = {"value", "max"},
        .defaults = {
            {"min", 0.0},
            {"unit", std::string("%")},
            {"showValue", true}
        },
        .supports_glass = true,
        .supports_hover = true
    });
    
    // ─── Input Components ────────────────────────────────────────
    registerComponent("Button", {
        .qml_type = "AXButton",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/controls/AXButton.qml",
        .required_props = {"text"},
        .defaults = {
            {"variant", std::string("primary")},
            {"disabled", false}
        },
        .supports_glass = true,
        .supports_hover = true
    });
    
    registerComponent("TextField", {
        .qml_type = "AXTextField",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/controls/AXTextField.qml",
        .required_props = {},
        .defaults = {
            {"placeholder", std::string("")},
            {"disabled", false}
        },
        .supports_glass = true,
        .supports_hover = true
    });
    
    registerComponent("Select", {
        .qml_type = "AXSelect",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/controls/AXSelect.qml",
        .required_props = {"options"},
        .defaults = {
            {"placeholder", std::string("Select...")},
            {"searchable", false}
        },
        .supports_glass = true,
        .supports_hover = true
    });
    
    // ─── Feedback Components ─────────────────────────────────────
    registerComponent("Toast", {
        .qml_type = "AXToast",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/feedback/AXToast.qml",
        .required_props = {"message"},
        .defaults = {
            {"type", std::string("info")},
            {"duration", 3000.0}
        },
        .supports_glass = true,
        .supports_hover = false
    });
    
    registerComponent("Skeleton", {
        .qml_type = "AXSkeleton",
        .qml_module = "AXUI.Components",
        .qml_file = "qrc:/axui/AXUI/Components/feedback/AXSkeleton.qml",
        .required_props = {},
        .defaults = {
            {"variant", std::string("rect")},
            {"animated", true}
        },
        .supports_glass = false,
        .supports_hover = false
    });
    
    builtins_loaded_ = true;
}

} // namespace axui
