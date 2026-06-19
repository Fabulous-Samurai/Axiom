from PySide6.QtCore import Qt
from PySide6.QtWidgets import QDockWidget, QWidget, QVBoxLayout, QLabel, QTreeWidget, QPushButton

class SymEnginePanel(QDockWidget):
    def __init__(self, parent=None):
        super().__init__("SymEngine Unit", parent)
        self.setObjectName("SymEnginePanel")

        self.widget = QWidget()
        self.layout = QVBoxLayout(self.widget)

        self.tree = QTreeWidget()
        self.tree.setHeaderLabels(["Expression Tree", "Details"])

        self.simplify_btn = QPushButton("Simplify Expression")

        self.layout.addWidget(QLabel("Live Symbolic Tree:"))
        self.layout.addWidget(self.tree)
        self.layout.addWidget(self.simplify_btn)

        self.setWidget(self.widget)
