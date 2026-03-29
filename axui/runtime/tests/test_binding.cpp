#include "axui/binding_engine.h"
#include "axui/compiler.h"
#include <cassert>
#include <cmath>
#include <string>
#include <thread>
#include <vector>
#include <iostream>

// ─── Yardımcı ────────────────────────────────────────────────────

static const char* MINI_THEME = R"({
    "name": "test",
    "colors": { "primary": "#7DD3FC", "success": "#6EE7B7" },
    "typography": { "body": 13 },
    "spacing": { "medium": 16 },
    "radius": {},
    "glass": {},
    "animation": {}
})";

static bool near(double a, double b, double eps = 0.001) {
    return std::abs(a - b) < eps;
}

// ═══════════════════════════════════════════════════════════════════
// TEST 1: Tek binding — veri gelince property güncellenir
// ═══════════════════════════════════════════════════════════════════

void test_binding_single_update() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "KPICard",
            "props": {
                "title": "Throughput",
                "value": " @engine.throughput"
            }
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    // value hâlâ Binding tipinde (veri gelmedi)
    auto* prop = result.root.getProperty("value");
    assert(prop != nullptr);
    assert(std::holds_alternative<axui::Binding>(*prop));

    // Veri gönder
    engine.updateSource(" @engine.throughput", 2500000.0);

    // Artık double olmalı
    assert(std::holds_alternative<double>(*prop));
    assert(near(std::get<double>(*prop), 2500000.0));
}

// ═══════════════════════════════════════════════════════════════════
// TEST 2: Birden fazla child'da farklı binding path'leri
// ═══════════════════════════════════════════════════════════════════

void test_binding_multiple_children() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Dashboard",
            "children": [
                {
                    "component": "Card",
                    "props": { "value": " @engine.cpu" }
                },
                {
                    "component": "Card",
                    "props": { "value": "@engine.ram" }
                },
                {
                    "component": "Card",
                    "props": { "value": "@engine.disk" }
                }
            ]
        }
    })", MINI_THEME);

    assert(result.success);
    assert(result.root.children.size() == 3);

    engine.bindTree(result.root);

    engine.updateSource(" @engine.cpu", 34.5);
    engine.updateSource("@engine.ram", 62.1);
    engine.updateSource("@engine.disk", 45.8);

    auto* cpu = result.root.children[0].getProperty("value");
    auto* ram = result.root.children[1].getProperty("value");
    auto* disk = result.root.children[2].getProperty("value");

    assert(near(std::get<double>(*cpu), 34.5));
    assert(near(std::get<double>(*ram), 62.1));
    assert(near(std::get<double>(*disk), 45.8));
}

// ═══════════════════════════════════════════════════════════════════
// TEST 3: Aynı path'e birden fazla property bağlı
// ═══════════════════════════════════════════════════════════════════

void test_binding_shared_path() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Dashboard",
            "children": [
                {
                    "component": "Header",
                    "props": { "metric": " @engine.throughput" }
                },
                {
                    "component": "Chart",
                    "props": { "dataPoint": "@engine.throughput" }
                }
            ]
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    // Tek update, iki property güncellenmeli
    engine.updateSource(" @engine.throughput", 9999.0);

    auto* p1 = result.root.children[0].getProperty("metric");
    auto* p2 = result.root.children[1].getProperty("dataPoint");

    assert(near(std::get<double>(*p1), 9999.0));
    assert(near(std::get<double>(*p2), 9999.0));
}

// ═══════════════════════════════════════════════════════════════════
// TEST 4: Batch update — tek seferde çoklu güncelleme
// ═══════════════════════════════════════════════════════════════════

void test_binding_batch_update() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Panel",
            "children": [
                { "component": "A", "props": { "v": " @engine.m1" } },
                { "component": "B", "props": { "v": "@engine.m2" } },
                { "component": "C", "props": { "v": "@engine.m3" } }
            ]
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    std::vector<axui::SourceUpdate> batch = {
        {" @engine.m1", 100.0},
        {"@engine.m2", 200.0},
        {"@engine.m3", 300.0}
    };
    engine.updateBatch(batch);

    assert(near(std::get<double>(*result.root.children[0].getProperty("v")), 100.0));
    assert(near(std::get<double>(*result.root.children[1].getProperty("v")), 200.0));
    assert(near(std::get<double>(*result.root.children[2].getProperty("v")), 300.0));
}

// ═══════════════════════════════════════════════════════════════════
// TEST 5: String tipi veri binding
// ═══════════════════════════════════════════════════════════════════

void test_binding_string_value() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "StatusBar",
            "props": { "status": " @engine.status" }
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    engine.updateSource(" @engine.status", std::string("RUNNING"));

    auto* prop = result.root.getProperty("status");
    assert(std::holds_alternative<std::string>(*prop));
    assert(std::get<std::string>(*prop) == "RUNNING");

    // Değeri değiştir
    engine.updateSource("@engine.status", std::string("IDLE"));
    assert(std::get<std::string>(*prop) == "IDLE");
}

// ═══════════════════════════════════════════════════════════════════
// TEST 6: Bool tipi veri binding
// ═══════════════════════════════════════════════════════════════════

void test_binding_bool_value() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Toggle",
            "props": { "active": " @engine.jit_enabled" }
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    engine.updateSource(" @engine.jit_enabled", true);

    auto* prop = result.root.getProperty("active");
    assert(std::holds_alternative<bool>(*prop));
    assert(std::get<bool>(*prop) == true);

    engine.updateSource("@engine.jit_enabled", false);
    assert(std::get<bool>(*prop) == false);
}

// ═══════════════════════════════════════════════════════════════════
// TEST 7: Color tipi veri binding
// ═══════════════════════════════════════════════════════════════════

void test_binding_color_value() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Indicator",
            "props": { "indicatorColor": " @engine.statusColor" }
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    axui::Color green{110, 231, 183, 255};
    engine.updateSource(" @engine.statusColor", green);

    auto* prop = result.root.getProperty("indicatorColor");
    assert(std::holds_alternative<axui::Color>(*prop));
    auto& c = std::get<axui::Color>(*prop);
    assert(c.r == 110 && c.g == 231 && c.b == 183 && c.a == 255);
}

// ═══════════════════════════════════════════════════════════════════
// TEST 8: int64_t → double promotion
// ═══════════════════════════════════════════════════════════════════

void test_binding_int64_promotion() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Counter",
            "props": { "count": " @engine.query_count" }
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    engine.updateSource(" @engine.query_count", int64_t(1234567));

    auto* prop = result.root.getProperty("count");
    // int64 → double promotion yapılmış olmalı
    assert(std::holds_alternative<double>(*prop));
    assert(near(std::get<double>(*prop), 1234567.0));
}

// ═══════════════════════════════════════════════════════════════════
// TEST 9: Late binding — önce veri gel, sonra bind et
// ═══════════════════════════════════════════════════════════════════

void test_binding_late_binding() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Widget",
            "props": { "data": " @engine.delayed" }
        }
    })", MINI_THEME);

    assert(result.success);

    // ÖNCE veriyi gönder (tree henüz bind edilmedi)
    engine.updateSource(" @engine.delayed", 42.0);

    // SONRA tree'yi bind et
    engine.bindTree(result.root);

    // Property hemen güncellenmiş olmalı (cache'den)
    auto* prop = result.root.getProperty("data");
    assert(std::holds_alternative<double>(*prop));
    assert(near(std::get<double>(*prop), 42.0));
}

// ═══════════════════════════════════════════════════════════════════
// TEST 10: Tema token'ları binding'den BAĞIMSIZ kalmalı
// ═══════════════════════════════════════════════════════════════════

void test_binding_theme_tokens_untouched() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Card",
            "props": {
                "background": " @colors.primary",
                "value": "@engine.metric"
            }
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    // @colors.primary zaten tema resolver tarafından Color'a dönüşmüş
    auto* bg = result.root.getProperty("background");
    assert(std::holds_alternative<axui::Color>(*bg));
    assert(std::get<axui::Color>(*bg).r == 125); // #7DD3FC

    // @engine.metric hâlâ Binding (veri gelmedi)
    auto* val = result.root.getProperty("value");
    assert(std::holds_alternative<axui::Binding>(*val));

    // Veri gelsin
    engine.updateSource(" @engine.metric", 99.9);
    assert(std::holds_alternative<double>(*val));
    assert(near(std::get<double>(*val), 99.9));

    // Tema rengi DEĞİŞMEMİŞ olmalı
    assert(std::holds_alternative<axui::Color>(*bg));
    assert(std::get<axui::Color>(*bg).r == 125);
}

// ═══════════════════════════════════════════════════════════════════
// TEST 11: İstatistikler doğru çalışıyor mu
// ═══════════════════════════════════════════════════════════════════

void test_binding_statistics() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Panel",
            "children": [
                { "component": "A", "props": { "v": " @engine.s1" } },
                { "component": "B", "props": { "v": "@engine.s2" } }
            ]
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    auto s1 = engine.statistics();
    assert(s1.active_subscriptions == 2);
    assert(s1.source_count == 0);
    assert(s1.total_updates == 0);

    engine.updateSource(" @engine.s1", 10.0);
    engine.updateSource("@engine.s2", 20.0);

    auto s2 = engine.statistics();
    assert(s2.total_updates == 2);
    assert(s2.source_count == 2);
    assert(s2.failed_updates == 0);
    assert(s2.avg_update_ns > 0); // Sıfırdan büyük olmalı
}

// ═══════════════════════════════════════════════════════════════════
// TEST 12: hasSource ve getSource
// ═══════════════════════════════════════════════════════════════════

void test_binding_source_query() {
    axui::BindingEngine engine;

    assert(!engine.hasSource("@engine.test"));
    assert(engine.sourceCount() == 0);

    engine.updateSource("@engine.test", 123.45);

    assert(engine.hasSource("@engine.test"));
    assert(!engine.hasSource("@engine.nonexistent"));
    assert(engine.sourceCount() == 1);

    auto val = engine.getSource("@engine.test");
    assert(std::holds_alternative<double>(val));
    assert(near(std::get<double>(val), 123.45));

    // Var olmayan path → default (0.0)
    auto def = engine.getSource("@engine.nonexistent");
    assert(std::holds_alternative<double>(def));
    assert(near(std::get<double>(def), 0.0));
}

// ═══════════════════════════════════════════════════════════════════
// TEST 13: clearSubscriptions — rebind senaryosu
// ═══════════════════════════════════════════════════════════════════

void test_binding_clear_and_rebind() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Widget",
            "props": { "v": " @engine.x" }
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);
    assert(engine.subscriptionCount() == 1);

    engine.updateSource(" @engine.x", 10.0);
    assert(near(std::get<double>(*result.root.getProperty("v")), 10.0));

    // Subscription temizle
    engine.clearSubscriptions();
    assert(engine.subscriptionCount() == 0);

    // Şimdi update gönder — property DEĞİŞMEMELİ (abone yok)
    engine.updateSource("@engine.x", 99.0);
    assert(near(std::get<double>(*result.root.getProperty("v")), 10.0)); // Hâlâ 10

    // Yeniden bind et
    // NOT: Property zaten 10.0 (double), artık Binding değil
    // Bu yüzden rebind için yeni bir tree lazım
    auto result2 = compiler.compile(R"({
        "root": {
            "component": "Widget",
            "props": { "v": " @engine.x" }
        }
    })", MINI_THEME);

    engine.bindTree(result2.root);
    assert(engine.subscriptionCount() == 1);

    // Cache'de 99.0 var, hemen uygulanmalı (late binding)
    assert(near(std::get<double>(*result2.root.getProperty("v")), 99.0));
}

// ═══════════════════════════════════════════════════════════════════
// TEST 14: Birden fazla güncelleme — son değer kalır
// ═══════════════════════════════════════════════════════════════════

void test_binding_rapid_updates() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Meter",
            "props": { "reading": " @engine.sensor" }
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    // 100 ardışık güncelleme
    for (int i = 0; i < 100; i++) {
        engine.updateSource(" @engine.sensor", static_cast<double>(i));
    }

    auto* prop = result.root.getProperty("reading");
    assert(near(std::get<double>(*prop), 99.0)); // Son değer

    auto stats = engine.statistics();
    assert(stats.total_updates == 100);
}

// ═══════════════════════════════════════════════════════════════════
// TEST 15: Thread safety — çoklu thread'den eşzamanlı güncelleme
// ═══════════════════════════════════════════════════════════════════

void test_binding_thread_safety() {
    axui::BindingEngine engine;
    axui::Compiler compiler;

    auto result = compiler.compile(R"({
        "root": {
            "component": "Monitor",
            "props": { "counter": " @engine.counter" }
        }
    })", MINI_THEME);

    assert(result.success);
    engine.bindTree(result.root);

    const int thread_count = 4;
    const int updates_per_thread = 250;

    std::vector<std::jthread> threads;
    for (int t = 0; t < thread_count; t++) {
        threads.emplace_back([&engine, t, updates_per_thread]() {
            for (int i = 0; i < updates_per_thread; i++) {
                double val = static_cast<double>(t * 1000 + i);
                engine.updateSource(" @engine.counter", val);
            }
        });
    }

    // Property'de bir double değer olmalı (hangisi olduğu önemli değil, race)
    auto* prop = result.root.getProperty("counter");
    assert(std::holds_alternative<double>(*prop));

    // Toplam update sayısı doğru olmalı
    auto stats = engine.statistics();
    assert(stats.total_updates == thread_count * updates_per_thread);
    assert(stats.failed_updates == 0);
}

// ═══════════════════════════════════════════════════════════════════
// TEST 16: Singleton erişimi
// ═══════════════════════════════════════════════════════════════════

void test_binding_singleton() {
    auto& e1 = axui::BindingContext::instance();
    auto& e2 = axui::BindingContext::instance();

    // Aynı instance
    assert(&e1 == &e2);

    e1.updateSource("@engine.singleton_test", 777.0);
    assert(e2.hasSource("@engine.singleton_test"));

    auto val = e2.getSource("@engine.singleton_test");
    assert(near(std::get<double>(val), 777.0));
}
