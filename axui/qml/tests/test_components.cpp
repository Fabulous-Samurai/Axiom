#include "axui/component_registry.h"
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QTest>
#include <QResource>
#include <cassert>
#include <iostream>

using namespace axui;

// ═══════════════════════════════════════════════════════════════════
// TEST HELPERS
// ═══════════════════════════════════════════════════════════════════

class ComponentTestFixture {
public:
    ComponentTestFixture() {
        // Manual resource initialization for static library
        Q_INIT_RESOURCE(axui_components);

        // Create minimal Qt application
        static int argc = 1;
        static char* argv[] = { const_cast<char*>("test") };
        if (!QGuiApplication::instance()) {
            app_ = std::make_unique<QGuiApplication>(argc, argv);
        }
        engine_ = std::make_unique<QQmlEngine>();
        
        // Register AXUI components
        ComponentRegistry::instance().loadBuiltinComponents();
        ComponentRegistry::instance().registerAllToEngine(engine_.get());
    }
    
    QQmlEngine* engine() { return engine_.get(); }
    
    QQuickItem* createComponent(const QString& qml) {
        QQmlComponent component(engine_.get());
        component.setData(qml.toUtf8(), QUrl());
        
        if (component.isError()) {
            for (const auto& error : component.errors()) {
                std::cerr << "QML Error: " << error.toString().toStdString() << "\n";
            }
            return nullptr;
        }
        
        return qobject_cast<QQuickItem*>(component.create());
    }
    
private:
    std::unique_ptr<QGuiApplication> app_;
    std::unique_ptr<QQmlEngine> engine_;
};

// ═══════════════════════════════════════════════════════════════════
// REGISTRY TESTS
// ═══════════════════════════════════════════════════════════════════

void test_builtin_components_registered() {
    ComponentRegistry::instance().loadBuiltinComponents();
    
    auto components = ComponentRegistry::instance().registeredComponents();
    
    // Check essential components are registered
    auto hasComponent = [&](const std::string& name) {
        return std::find(components.begin(), components.end(), name) != components.end();
    };
    
    assert(hasComponent("KPICard"));
    assert(hasComponent("DataGrid"));
    assert(hasComponent("Chart"));
    assert(hasComponent("Gauge"));
    assert(hasComponent("Column"));
    assert(hasComponent("Row"));
    assert(hasComponent("Grid"));
    assert(hasComponent("Button"));
    
    std::cout << "✓ test_builtin_components_registered PASSED\n";
}

void test_component_meta_retrieval() {
    ComponentRegistry::instance().loadBuiltinComponents();
    
    auto meta = ComponentRegistry::instance().getMeta("KPICard");
    assert(meta.has_value());
    assert(meta->qml_type == "AXKPICard");
    assert(meta->qml_module == "AXUI.Components");
    assert(meta->supports_glass == true);
    assert(meta->supports_hover == true);
    
    // Check required props
    auto& required = meta->required_props;
    assert(std::find(required.begin(), required.end(), "title") != required.end());
    assert(std::find(required.begin(), required.end(), "value") != required.end());
    
    std::cout << "✓ test_component_meta_retrieval PASSED\n";
}

void test_unknown_component_returns_nullopt() {
    auto meta = ComponentRegistry::instance().getMeta("NonExistentComponent");
    assert(!meta.has_value());
    
    std::cout << "✓ test_unknown_component_returns_nullopt PASSED\n";
}

// ═══════════════════════════════════════════════════════════════════
// KPICard TESTS
// ═══════════════════════════════════════════════════════════════════

void test_kpicard_creation() {
    ComponentTestFixture fixture;
    
    auto item = fixture.createComponent(R"(
        import QtQuick 2.15
        import AXUI.Components 1.0
        
        AXKPICard {
            title: "Revenue"
            value: 125000
            unit: "$"
            trend: 12.5
            trendDirection: "up"
        }
    )");
    
    assert(item != nullptr);
    assert(item->property("title").toString() == "Revenue");
    assert(item->property("value").toDouble() == 125000);
    assert(item->property("unit").toString() == "$");
    assert(item->property("trend").toDouble() == 12.5);
    assert(item->property("trendDirection").toString() == "up");
    
    delete item;
    std::cout << "✓ test_kpicard_creation PASSED\n";
}

void test_kpicard_trend_colors() {
    ComponentTestFixture fixture;
    
    // Test "up" direction
    auto itemUp = fixture.createComponent(R"(
        import QtQuick 2.15
        import AXUI.Components 1.0
        
        AXKPICard {
            title: "Test"
            value: 100
            trendDirection: "up"
        }
    )");
    
    assert(itemUp != nullptr);
    QColor upColor = itemUp->property("currentTrendColor").value<QColor>();
    assert(upColor == QColor("#22C55E"));  // Green
    
    // Test "down" direction
    auto itemDown = fixture.createComponent(R"(
        import QtQuick 2.15
        import AXUI.Components 1.0
        
        AXKPICard {
            title: "Test"
            value: 100
            trendDirection: "down"
        }
    )");
    
    assert(itemDown != nullptr);
    QColor downColor = itemDown->property("currentTrendColor").value<QColor>();
    assert(downColor == QColor("#EF4444"));  // Red
    
    delete itemUp;
    delete itemDown;
    std::cout << "✓ test_kpicard_trend_colors PASSED\n";
}

int main(int argc, char *argv[]) {
    test_builtin_components_registered();
    test_component_meta_retrieval();
    test_unknown_component_returns_nullopt();
    test_kpicard_creation();
    test_kpicard_trend_colors();
    
    std::cout << "\nALL COMPONENT TESTS PASSED\n";
    return 0;
}
