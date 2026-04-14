#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTest>
#include <cassert>

#include "axui/compiler.h"
#include "axui/qml_loader.h"

using namespace axui;

// ═══════════════════════════════════════════════════════════════════
// TEST: QML kod üretimi
// ═══════════════════════════════════════════════════════════════════

void test_qml_code_generation() {
  int argc = 0;
  char** argv = nullptr;
  QGuiApplication app(argc, argv);  // Qt requires QGuiApplication
  QmlLoader loader;

  Compiler compiler;
  auto result = compiler.compile(R"({
        "root": {
            "component": "Column",
            "props": {
                "spacing": 16
            },
            "children": [
                {
                    "component": "Text",
                    "props": {
                        "text": "Hello AXUI",
                        "color": "#FFFFFF"
                    }
                }
            ]
        }
    })",
                                 R"({
        "name": "test",
        "colors": {},
        "typography": {},
        "spacing": {},
        "radius": {},
        "glass": {},
        "animation": {}
    })");

  assert(result.success);
  std::cout << "QML Code Generation Test: PASS" << std::endl;

  // Test direct loading from node
  // QmlLoader loader2;
  // loader2.loadFromNode(result.root);
  // std::cout << "QML Load From Node Test: PASS" << std::endl;
}

int main(int argc, char** argv) {
  std::cout << "Running AXUI QML Tests..." << std::endl;
  // Basic test run
  test_qml_code_generation();
  std::cout << "All AXUI QML Tests PASSED!" << std::endl;
  return 0;
}
