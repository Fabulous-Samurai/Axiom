from PySide6.QtCore import Qt
from PySide6.QtWidgets import QDockWidget, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QSlider, QProgressBar

class MantisPanel(QDockWidget):
    def __init__(self, parent=None):
        super().__init__("Mantis Unit", parent)
        self.setObjectName("MantisPanel")
        
        self.widget = QWidget()
        self.layout = QVBoxLayout(self.widget)
        
        self.layout.addWidget(QLabel("Heuristic Weight (A*):"))
        self.weight_slider = QSlider(Qt.Orientation.Horizontal)
        self.weight_slider.setRange(0, 200)
        self.weight_slider.setValue(100)
        self.layout.addWidget(self.weight_slider)
        
        self.layout.addWidget(QLabel("Search Tree Depth:"))
        self.depth_bar = QProgressBar()
        self.depth_bar.setRange(0, 1000)
        self.layout.addWidget(self.depth_bar)
        
        self.setWidget(self.widget)
