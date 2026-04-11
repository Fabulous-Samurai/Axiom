from PySide6.QtCore import QObject, Signal, Slot
from axiom_studio.config import save_config

class ThemeManager(QObject):
    themeChanged = Signal(str)

    def __init__(self):
        super().__init__()
        self._current_theme = "shadow_soft"

    @Slot(str)
    def setTheme(self, theme_name: str):
        self._current_theme = theme_name
        self.themeChanged.emit(theme_name)
        save_config({"theme": theme_name})

    @Slot(result=str)
    def autoTheme(self) -> str:
        import datetime
        hour = datetime.datetime.now().hour
        if 6 <= hour < 18:
            return "reflection_soft"
        else:
            return "shadow_soft"