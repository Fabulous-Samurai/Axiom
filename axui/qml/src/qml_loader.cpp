// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file qml_loader.cpp
 * @brief Phase G: Adaptive UI Loader (Operation VARIANT SHIFT)
 */

#include "axui/qml_loader.h"
#include <QQmlContext>
#include <QQuickItem>
#include <QStringBuilder>
#include <QFile>
#include <QTextStream>
#include <iostream>

namespace axui {

QmlLoader::QmlLoader(QObject* parent)
    : QObject(parent), binding_engine_(&BindingContext::instance()) {}

bool QmlLoader::loadFromNode(const UINode& root, const QmlConfig& config) {
    QString qml_code = "import QtQuick\nimport QtQuick.Controls\n\n";
    qml_code += generateComponentCode(root, 0);

    if (config.debug_output) {
        std::cout << "[AXIOM UI] Generated QML:\n" << qml_code.toStdString() << std::endl;
    }

    QQmlComponent component(&qml_engine_);
    component.setData(qml_code.toUtf8(), QUrl());

    if (component.isError()) {
        std::cerr << "[AXIOM UI] QML Component Error: " << component.errorString().toStdString() << std::endl;
        return false;
    }

    QObject* obj = component.create();
    if (!obj) return false;

    QQuickItem* item = qobject_cast<QQuickItem*>(obj);
    if (!item) {
        delete obj;
        return false;
    }

    if (config.parent_window) {
        item->setParentItem(config.parent_window->contentItem());
    }

    setupBindings(item, root);
    return true;
}

bool QmlLoader::loadFromFile(const std::string& axui_json_path, const std::string& theme_json_path, const QmlConfig& config) {
    QFile axui_file(QString::fromStdString(axui_json_path));
    if (!axui_file.open(QIODevice::ReadOnly)) return false;
    std::string axui_json = axui_file.readAll().toStdString();

    std::string theme_json;
    if (!theme_json_path.empty()) {
        QFile theme_file(QString::fromStdString(theme_json_path));
        if (theme_file.open(QIODevice::ReadOnly)) {
            theme_json = theme_file.readAll().toStdString();
        }
    }

    CompileResult result = compiler_.compile(axui_json, theme_json);
    if (!result.success || !result.root) return false;

    binding_engine_->bindTree(*result.root);

    return loadFromNode(*result.root, config);
}

QString QmlLoader::generateComponentCode(const UINode& node, int depth) {
    QString indent = QString(depth * 4, ' ');
    QString code = indent;

    switch (node.component_type) {
        case ComponentType::Container: code += "Item {\n"; break;
        case ComponentType::Column:    code += "Column {\n"; break;
        case ComponentType::Row:       code += "Row {\n"; break;
        case ComponentType::Text:      code += "Text {\n"; break;
        case ComponentType::Button:    code += "Button {\n"; break;
        case ComponentType::KPICard:   code += "Rectangle {\n"; break;
        case ComponentType::GlassPanel: code += "Rectangle {\n"; break;
        default: code += "Item {\n"; break;
    }

    if (!node.id.empty()) {
        QString id_str = QString::fromUtf8(node.id.data(), static_cast<int>(node.id.size()));
        code += indent + "    id: " + id_str + "\n";
    }

    // Properties
    for (const auto& prop : node.properties) {
        code += convertPropertyToQml(prop, depth + 1);
    }

    // Children
    if (!node.children.empty()) {
        QString children_code;
        QTextStream stream(&children_code);
        for (auto* child : node.children) {
            if (child) stream << generateComponentCode(*child, depth + 1);
        }
        code += children_code;
    }

    code += indent + "}\n";
    return code;
}

QString QmlLoader::convertPropertyToQml(const Property& prop, int depth) {
    QString indent = QString(depth * 4, ' ');
    QString code;
    QString key = QString::fromUtf8(prop.key.data(), static_cast<int>(prop.key.size()));

    std::visit([&](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, double>) {
            code = indent + key + ": " + QString::number(value) + "\n";
        }
        else if constexpr (std::is_same_v<T, std::string_view>) {
            QString v = QString::fromUtf8(value.data(), static_cast<int>(value.size()));
            code = indent + key + ": \"" + v + "\"\n";
        }
        else if constexpr (std::is_same_v<T, bool>) {
            code = indent + key + ": " + (value ? "true" : "false") + "\n";
        }
        else if constexpr (std::is_same_v<T, Color>) {
            QString color_str = QString("#%1%2%3%4")
                .arg(value.r, 2, 16, QChar('0'))
                .arg(value.g, 2, 16, QChar('0'))
                .arg(value.b, 2, 16, QChar('0'))
                .arg(value.a, 2, 16, QChar('0'));
            code = indent + key + ": \"" + color_str + "\"\n";
        }
        else if constexpr (std::is_same_v<T, Binding>) {
            QString path = QString::fromUtf8(value.path.data(), static_cast<int>(value.path.size()));
            code = indent + key + ": 0  // Binding: " + path + "\n";
        }
    }, prop.value);

    return code;
}

void QmlLoader::setupBindings(QQuickItem* item, const UINode& node) {
    if (!item) return;

    for (const auto& prop : node.properties) {
        if (auto* binding = std::get_if<Binding>(&prop.value)) {
            if (binding->is_bound) {
                QString qml_prop = QString::fromUtf8(prop.key.data(), static_cast<int>(prop.key.size()));
                QString path = QString::fromUtf8(binding->path.data(), static_cast<int>(binding->path.size()));

                // [MANDATORY PATH]: Register QML callback for data flow
                connect(binding_engine_, &BindingEngine::sourceUpdated, this,
                    [item, qml_prop, path](const QString& updated_path, const BindingValue& val) {
                        if (updated_path == path) {
                            // Update QML property dynamically
                            // [MANDATORY]: Use QVariant::fromValue for complex types in Zenith
                            item->setProperty(qml_prop.toUtf8().constData(), QVariant::fromValue(val));
                        }
                    });
            }
        }
    }

    // Recurse into children
    auto children = item->childItems();
    for (int i = 0; i < std::min((int)children.size(), (int)node.children.size()); ++i) {
        if (node.children[i]) setupBindings(children[i], *node.children[i]);
    }
}

} // namespace axui
