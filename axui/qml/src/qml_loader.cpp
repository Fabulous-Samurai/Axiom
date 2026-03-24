#include "axui/qml_loader.h"
#include "axui/compiler.h"
#include "axui/component_registry.h"
#include <QQmlContext>
#include <QQuickWindow>
#include <QFile>
#include <QDir>
#include <sstream>
#include <fstream>
#include <QTextStream>
#include <QColor>
#include <algorithm>

namespace axui {

QmlLoader::QmlLoader(QObject* parent)
    : QObject(parent),
      engine_(std::make_unique<QQmlApplicationEngine>()),
      binding_engine_(std::make_unique<BindingEngine>())
{
    // Register AXUI components
    ComponentRegistry::instance().loadBuiltinComponents();
    ComponentRegistry::instance().registerAllToEngine(engine_.get());
}

QmlLoader::~QmlLoader() = default;

bool QmlLoader::loadFromNode(const UINode& root, const QmlConfig& config) {
    // 1. QML kod üret
    QString qml_code = generateQmlCode(root);

    // 2. Geçici dosyaya yaz (QML engine dosya bekler)
    QDir temp_dir(QDir::temp());
    QString temp_path = temp_dir.filePath("axui_generated.qml");

    QFile temp_file(temp_path);
    if (!temp_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit componentError("Failed to write temp QML file");
        return false;
    }

    QTextStream out(&temp_file);
    out << qml_code;
    temp_file.close();

    // 3. QML engine'e yükle
    QUrl qml_url = QUrl::fromLocalFile(temp_path);
    last_loaded_url_ = qml_url;

    engine_->load(qml_url);

    if (engine_->rootObjects().isEmpty()) {
        emit componentError("QML engine failed to create root object");
        return false;
    }

    // 4. Binding engine'i bağla
    setupBindings(qobject_cast<QQuickItem*>(engine_->rootObjects().first()), root);
    connectBindingEngine();

    emit componentLoaded("root");
    return true;
}

bool QmlLoader::loadFromFile(
    const std::string& axui_path,
    const std::string& theme_path,
    const QmlConfig& config
) {
    // 1. Dosyaları oku
    std::ifstream axui_file(axui_path);
    std::ifstream theme_file(theme_path);

    if (!axui_file.is_open() || !theme_file.is_open()) {
        emit componentError("Failed to open .axui or .axtheme file");
        return false;
    }

    std::stringstream axui_buffer, theme_buffer;
    axui_buffer << axui_file.rdbuf();
    theme_buffer << theme_file.rdbuf();

    // 2. Compile (Parser + Resolver)
    Compiler compiler;
    auto result = compiler.compile(axui_buffer.str(), theme_buffer.str());

    if (!result.success) {
        emit componentError("Compilation failed");
        return false;
    }

    // 3. Binding engine'e bağla
    binding_engine_->bindTree(result.root);

    // 4. QML'e dönüştür
    last_axui_path_ = axui_path;
    last_theme_path_ = theme_path;

    return loadFromNode(result.root, config);
}

void QmlLoader::reload() {
    if (last_axui_path_.empty()) {
        // Node'dan yüklenmiş, hot-reload desteklenmiyor
        return;
    }

    // Cache temizle
    component_cache_.clear();
    binding_engine_->clearSubscriptions();

    // Yeniden yükle
    loadFromFile(last_axui_path_, last_theme_path_);
}

// ═══════════════════════════════════════════════════════════════════
// QML CODE GENERATION
// ═══════════════════════════════════════════════════════════════════

QString QmlLoader::generateQmlCode(const UINode& root) {
    QString code;
    QTextStream stream(&code);

    // QML Header
    stream << R"(
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import AXUI.Components 1.0

ApplicationWindow {
    visible: true
    width: 1280
    height: 720
    title: "AXUI Application"
    
    color: "#1E293B"  // Default background
    
)";

    // Root component
    stream << generateComponentCode(root, 1);

    // QML Footer
    stream << "}\n";

    return code;
}

QString QmlLoader::generateComponentCode(const UINode& node, int depth) {
    QString code;
    QTextStream stream(&code);

    QString indent(depth * 4, ' ');

    // Component type mapping via Registry
    QString qml_type = "Item"; // Default
    
    auto type_name = componentTypeName(node.component_type);
    auto meta = ComponentRegistry::instance().getMeta(std::string(type_name));
    
    if (meta) {
        qml_type = meta->qml_type;
    } else {
        // Fallback for types not in registry or special handling
        switch (node.component_type) {
            case ComponentType::Container: qml_type = "Item"; break;
            case ComponentType::Text:      qml_type = "Text"; break;
            case ComponentType::Image:     qml_type = "Image"; break;
            default:                       qml_type = "Item"; break;
        }
    }

    stream << indent << qml_type << " {\n";

    // Properties
    for (const auto& prop : node.properties) {
        QString qml_prop = convertPropertyToQml(prop, depth + 1);
        if (!qml_prop.isEmpty()) {
            stream << qml_prop;
        }
    }

    // Glass effect (ShaderEffect)
    if (node.glass.enabled) {
        stream << generateBlurShader(node.glass);
    }

    // Hover effect
    if (node.hover.enabled) {
        stream << indent << "    " << "MouseArea {\n";
        stream << indent << "    " << "    anchors.fill: parent\n";
        stream << indent << "    " << "    hoverEnabled: true\n";
        stream << indent << "    " << "    onEntered: parent.scale = " 
               << node.hover.scale << "\n";
        stream << indent << "    " << "    onExited: parent.scale = 1.0\n";
        stream << indent << "    " << "}\n";
    }

    // Children
    for (const auto& child : node.children) {
        stream << generateComponentCode(child, depth + 1);
    }

    stream << indent << "}\n";

    return code;
}

QString QmlLoader::convertPropertyToQml(const Property& prop, int depth) {
    QString indent(depth * 4, ' ');
    QString code;

    std::visit([&](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, double>) {
            code = indent + QString::fromStdString(prop.key) + ": " 
                   + QString::number(value) + "\n";
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            code = indent + QString::fromStdString(prop.key) + ": \"" 
                   + QString::fromStdString(value) + "\"\n";
        }
        else if constexpr (std::is_same_v<T, bool>) {
            code = indent + QString::fromStdString(prop.key) + ": " 
                   + (value ? "true" : "false") + "\n";
        }
        else if constexpr (std::is_same_v<T, Color>) {
            QString color_str = QString("#%1%2%3%4")
                .arg(value.r, 2, 16, QChar('0'))
                .arg(value.g, 2, 16, QChar('0'))
                .arg(value.b, 2, 16, QChar('0'))
                .arg(value.a, 2, 16, QChar('0'));
            code = indent + QString::fromStdString(prop.key) + ": \"" 
                   + color_str + "\"\n";
        }
        else if constexpr (std::is_same_v<T, Binding>) {
            // Runtime binding — QML property binding ile bağlanacak
            // Şimdilik placeholder
            code = indent + QString::fromStdString(prop.key) + ": 0  " 
                   + "// Binding: " + QString::fromStdString(value.path) + "\n";
        }
    }, prop.value);

    return code;
}

QString QmlLoader::generateBlurShader(const GlassParams& glass) {
    // QML ShaderEffect ile blur implementation
    return R"(
    layer.enabled: true
    layer.effect: ShaderEffect {
        property real blurRadius: )" + QString::number(glass.blur_radius) + R"(
        property real bgOpacity: )" + QString::number(glass.bg_opacity) + R"(
        
        fragmentShader: "
            uniform lowp float blurRadius;
            uniform lowp float bgOpacity;
            uniform sampler2D source;
            varying highp vec2 qt_TexCoord0;
            
            void main() {
                vec4 color = vec4(0.0);
                float total = 0.0;
                
                // Simple box blur (9-tap)
                for (float x = -1.0; x <= 1.0; x += 1.0) {
                    for (float y = -1.0; y <= 1.0; y += 1.0) {
                        vec2 offset = vec2(x, y) * (blurRadius / 100.0);
                        color += texture2D(source, qt_TexCoord0 + offset);
                        total += 1.0;
                    }
                }
                
                color /= total;
                color.a *= bgOpacity;
                gl_FragColor = color;
            }
        "
    }
)";
}

QString QmlLoader::generateGlowShader(const HoverParams& hover) {
    return QString("// TODO: Glow shader for radius %1").arg(hover.glow_radius);
}

QString QmlLoader::generateNoiseShader(float strength) {
    return QString("// TODO: Noise shader with strength %1").arg(strength);
}

void QmlLoader::setupBindings(QQuickItem* item, const UINode& node) {
    if (!item) return;

    for (const auto& prop : node.properties) {
        if (auto* binding = std::get_if<Binding>(&prop.value)) {
            if (binding->is_bound) {
                QString qml_prop = QString::fromStdString(prop.key);
                
                connect(this, &QmlLoader::bindingUpdated, item,
                    [item, qml_prop, path = binding->path](
                        const QString& updated_path, 
                        const QVariant& value
                    ) {
                        if (updated_path == QString::fromStdString(path)) {
                            item->setProperty(qml_prop.toUtf8().constData(), value);
                        }
                    }
                );
            }
        }
    }

    auto children = item->childItems();
    int node_child_count = static_cast<int>(node.children.size());
    int qml_child_count = static_cast<int>(children.size());
    int min_count = std::min(node_child_count, qml_child_count);

    for (int i = 0; i < min_count; ++i) {
        setupBindings(children[i], node.children[i]);
    }
}

void QmlLoader::connectBindingEngine() {
}

QVariant propertyToQVariant(const PropValue& value) {
    return std::visit([](auto&& arg) -> QVariant {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, double>) {
            return QVariant(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return QVariant(QString::fromStdString(arg));
        } else if constexpr (std::is_same_v<T, bool>) {
            return QVariant(arg);
        } else if constexpr (std::is_same_v<T, Color>) {
            return QVariant(QColor(arg.r, arg.g, arg.b, arg.a));
        } else {
            return QVariant();
        }
    }, value);
}

PropValue qvariantToProperty(const QVariant& variant) {
    switch (variant.userType()) {
        case QMetaType::Double:
            return variant.toDouble();
        case QMetaType::QString:
            return variant.toString().toStdString();
        case QMetaType::Bool:
            return variant.toBool();
        case QMetaType::QColor: {
            QColor qc = variant.value<QColor>();
            return Color{
                static_cast<uint8_t>(qc.red()),
                static_cast<uint8_t>(qc.green()),
                static_cast<uint8_t>(qc.blue()),
                static_cast<uint8_t>(qc.alpha())
            };
        }
        default:
            return 0.0;
    }
}

} // namespace axui
