import sys
import os
from PySide6.QtGui import QGuiApplication, QFontDatabase
from PySide6.QtQml import QQmlApplicationEngine
from PySide6.QtCore import QUrl

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from axiom_studio.backend.bridge.engine_wrapper import EngineWrapper
from axiom_studio.backend.bridge.qml_bridge import ThemeManager
from axiom_studio.config import load_config
from axiom_studio.version import AXIOM_STUDIO_VERSION, AXIOM_ENGINE_VERSION

def main():
    app = QGuiApplication(sys.argv)
    app.setApplicationName("AXIOM Studio")
    app.setApplicationVersion(AXIOM_STUDIO_VERSION)

    # Fontları script'in bulunduğu dizine göre yükle
    base_dir = os.path.dirname(__file__)
    font_dir = os.path.join(base_dir, "resources", "fonts")
    fonts = ["Inter-Regular.ttf", "Inter-Medium.ttf", "Inter-SemiBold.ttf", "Inter-Bold.ttf", "JetBrainsMono-Regular.ttf", "JetBrainsMono-Bold.ttf"]
    for font in fonts:
        f_path = os.path.join(font_dir, font)
        if os.path.exists(f_path):
            QFontDatabase.addApplicationFont(f_path)

    engine = QQmlApplicationEngine()

    engine_bridge = EngineWrapper()
    engine.rootContext().setContextProperty("engineBridge", engine_bridge)

    theme_manager = ThemeManager()
    config = load_config()
    theme_manager.setTheme(config.get("theme", "shadow_soft"))
    engine.rootContext().setContextProperty("themeManager", theme_manager)

    frontend_path = os.path.join(os.path.dirname(__file__), 'frontend')
    engine.addImportPath(frontend_path)

    qml_file = os.path.join(frontend_path, 'main.qml')
    engine.load(QUrl.fromLocalFile(qml_file))

    if not engine.rootObjects():
        print("ERROR: QML failed to load")
        return -1

    print(f"AXIOM Studio v{AXIOM_STUDIO_VERSION}")
    print(f"Powered by AXIOM Engine v{AXIOM_ENGINE_VERSION}")
    print("Ready.")

    return app.exec()

if __name__ == "__main__":
    sys.exit(main())