from PySide6.QtCore import Qt
from PySide6.QtWidgets import QDockWidget, QWidget, QVBoxLayout, QHBoxLayout, QPlainTextEdit, QLineEdit, QPushButton, QTabWidget

class ConsolePanel(QDockWidget):
    def __init__(self, parent=None):
        super().__init__("Surgical Console", parent)
        self.setObjectName("ConsolePanel")
        self.setAllowedAreas(Qt.DockWidgetArea.AllDockWidgetAreas)

        self.widget = QWidget()
        self.layout = QVBoxLayout(self.widget)
        self.layout.setContentsMargins(4, 4, 4, 4)

        self.tabs = QTabWidget()
        self.cmd_tab = QWidget()
        self.cmd_layout = QVBoxLayout(self.cmd_tab)

        self.output = QPlainTextEdit()
        self.output.setReadOnly(True)
        self.output.setPlainText("AXIOM PRO Surgical shell ready")

        row = QHBoxLayout()
        self.input = QLineEdit()
        self.input.setPlaceholderText("Enter expression or command (e.g. 2+3, solve, derive)...")
        self.run_btn = QPushButton("Execute")
        self.run_btn.setFixedWidth(80)

        row.addWidget(self.input)
        row.addWidget(self.run_btn)

        self.cmd_layout.addWidget(self.output, 1)
        self.cmd_layout.addLayout(row)

        self.tabs.addTab(self.cmd_tab, "Command")
        self.layout.addWidget(self.tabs)
        self.setWidget(self.widget)
