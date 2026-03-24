from PySide6.QtCore import Qt, QAbstractTableModel, QModelIndex
from PySide6.QtWidgets import QDockWidget, QWidget, QVBoxLayout, QLabel, QTableView, QMenu, QDialog, QFormLayout, QLineEdit
from typing import Any

class VarEntry:
    """P2 - Compact variable store entry."""
    __slots__ = ("type", "size", "value")
    def __init__(self, t: str, s: str, v: str) -> None:
        self.type = t
        self.size = s
        self.value = v

class WorkspaceTableModel(QAbstractTableModel):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._named = {}
        self._ans = []
        self._keys = []
    def bind_store(self, named, ans):
        self._named = named
        self._ans = ans
    def refresh(self):
        self.beginResetModel()
        self._keys = list(self._named.keys())
        self.endResetModel()
    def rowCount(self, parent=QModelIndex()): return len(self._keys)
    def columnCount(self, parent=QModelIndex()): return 4
    def data(self, index, role=Qt.DisplayRole):
        if not index.isValid() or role != Qt.DisplayRole: return None
        row, col = index.row(), index.column()
        key = self._keys[row]
        entry = self._named.get(key)
        return (key, entry.type, entry.size, entry.value)[col]

class WorkspacePanel(QDockWidget):
    def __init__(self, parent=None):
        super().__init__("Workspace", parent)
        self.widget = QWidget(); self.setWidget(self.widget)
        self.layout = QVBoxLayout(self.widget)
        self.model = WorkspaceTableModel(self)
        self.table = QTableView(); self.table.setModel(self.model)
        self.layout.addWidget(self.table)
