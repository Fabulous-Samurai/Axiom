from PySide6.QtCore import Qt
from PySide6.QtWidgets import QDockWidget, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton, QPlainTextEdit
try:
    from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg as FigureCanvas
    from matplotlib.figure import Figure
    MATPLOTLIB_AVAILABLE = True
except Exception:
    MATPLOTLIB_AVAILABLE = False

class FigurePanel(QDockWidget):
    def __init__(self, parent=None):
        super().__init__("Figure Viewport", parent)
        self.setObjectName("FigurePanel")
        self.setAllowedAreas(Qt.DockWidgetArea.AllDockWidgetAreas)

        self.widget = QWidget()
        self.layout = QVBoxLayout(self.widget)
        self.layout.setContentsMargins(4, 4, 4, 4)

        header_row = QHBoxLayout()
        self.memory_info = QLabel("0/0 | 3D off")
        self.prev_btn = QPushButton("<")
        self.prev_btn.setFixedSize(24, 22)
        self.next_btn = QPushButton(">")
        self.next_btn.setFixedSize(24, 22)

        header_row.addStretch(1)
        header_row.addWidget(self.memory_info)
        header_row.addWidget(self.prev_btn)
        header_row.addWidget(self.next_btn)
        self.layout.addLayout(header_row)

        if MATPLOTLIB_AVAILABLE:
            self.figure = Figure(figsize=(5, 3), tight_layout=True)
            self.axes = self.figure.add_subplot(111)
            self.axes.grid(alpha=0.25)
            self.canvas = FigureCanvas(self.figure)
            self.layout.addWidget(self.canvas, 1)
            self.figure_stub = None
        else:
            self.figure_stub = QPlainTextEdit()
            self.figure_stub.setReadOnly(True)
            self.figure_stub.setPlainText("Matplotlib not available.")
            self.layout.addWidget(self.figure_stub, 1)
            self.figure = None
            self.axes = None
            self.canvas = None

        self.setWidget(self.widget)
