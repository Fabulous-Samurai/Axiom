#!/usr/bin/env python3
import sys
import queue
import math
import time
from pathlib import Path
from typing import Any

# Mocks
class VulkanBridge:
    def __init__(self): self.available = False
class TelemetryShmReader:
    def __init__(self): self.snapshot = None
    def is_connected(self): return False
    def try_reconnect(self): pass
    def close(self): pass

try:
    from PySide6.QtCore import QTimer, Qt, QSize
    from PySide6.QtGui import QColor, QIcon, QShortcut, QKeySequence, QFont, QPainter, QPainterPath, QPen, QLinearGradient
    from PySide6.QtWidgets import (
        QApplication, QFrame, QHBoxLayout, QLabel, QLineEdit,
        QMainWindow, QPushButton, QVBoxLayout, QWidget, QDockWidget, 
        QGraphicsDropShadowEffect, QHeaderView
    )
except ImportError:
    print("PySide6 missing.")
    sys.exit(1)

from gui.python.gui_helpers import CppEngineInterface, ResultCache, PerformanceMonitor
from gui.qt.var_store import LargeScaleVarStore
from gui.qt.panels.workspace_panel import WorkspacePanel
from gui.qt.panels.figure_panel import FigurePanel
from gui.qt.panels.console_panel import ConsolePanel
from gui.qt.panels.symengine_panel import SymEnginePanel
from gui.qt.panels.mantis_panel import MantisPanel
from gui.qt.panels.validation_hub import ValidationHub

class HarmonicSparkline(QWidget):
    def __init__(self, line_hex: str, parent=None):
        super().__init__(parent)
        self._line_color = QColor(line_hex)
        self._fill_gradient_start = QColor("#6C5DD3")
        self._fill_gradient_start.setAlpha(100)
        self._values = [0.5] * 48
        self.setMinimumHeight(60)

    def push(self, value: float):
        v = max(0.0, min(1.0, value))
        self._values.append(v)
        self._values = self._values[-48:]
        self.update()

    def paintEvent(self, event):
        w, h = self.width(), self.height()
        if w <= 4 or h <= 4 or len(self._values) < 2: return
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setPen(Qt.NoPen)
        painter.setBrush(QColor("#151822"))
        painter.drawRoundedRect(0, 0, w, h, 12, 12)
        step_x = w / float(len(self._values) - 1)
        path = QPainterPath()
        points = [(i * step_x, (h - 10) - v * (h - 20)) for i, v in enumerate(self._values)]
        path.moveTo(points[0][0], points[0][1])
        for i in range(1, len(points)):
            cx = (points[i-1][0] + points[i][0]) / 2.0
            path.quadTo(points[i-1][0], points[i-1][1], cx, (points[i-1][1] + points[i][1]) / 2.0)
        fill_path = QPainterPath(path)
        fill_path.lineTo(w, h); fill_path.lineTo(0, h); fill_path.closeSubpath()
        grad = QLinearGradient(0, 0, 0, h)
        grad.setColorAt(0.0, self._fill_gradient_start); grad.setColorAt(1.0, Qt.transparent)
        painter.fillPath(fill_path, grad)
        painter.setPen(QPen(self._line_color, 2.5)); painter.drawPath(path)

class AxiomQtWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("AXIOM PRO v3.1.2 [UX-ZENITH]")
        self.resize(1600, 900)
        self.setFont(QFont("Inter", 10))
        self.engine = CppEngineInterface(None)
        self.result_cache = ResultCache(max_size=256)
        self.performance_monitor = PerformanceMonitor()
        self._workspace_named_vars = LargeScaleVarStore()
        self._ans_pool = []
        self._ui_tasks = queue.Queue()
        self._build_ui()
        self.timer = QTimer(self); self.timer.timeout.connect(self._drain); self.timer.start(16)

    def _build_ui(self):
        self.setStyleSheet("""
            QMainWindow { background-color: #0B0D14; color: #FFFFFF; }
            QDockWidget::title { background-color: #151822; color: #8F95B2; padding: 10px; font-weight: bold; border-radius: 8px; }
            QFrame#Header { background-color: #151822; border-bottom: 1px solid rgba(255,255,255,0.06); }
            QLabel#T { color: #FFFFFF; font-size: 18px; font-weight: 800; }
            QLineEdit { background-color: #1F222C; border: 1px solid rgba(255,255,255,0.1); color: #FFFFFF; border-radius: 8px; padding: 8px; font-family: 'JetBrains Mono'; }
            QPushButton { background-color: #6C5DD3; color: #FFFFFF; border-radius: 8px; padding: 8px 16px; font-weight: bold; }
            QPushButton:hover { background-color: #5a4cb3; }
            QTableView { background-color: #151822; border: none; gridline-color: transparent; font-family: 'JetBrains Mono'; }
            QHeaderView::section { background-color: #151822; color: #8F95B2; border: none; border-bottom: 1px solid rgba(255,255,255,0.06); }
        """)
        c = QWidget(); self.setCentralWidget(c); l = QVBoxLayout(c); l.setContentsMargins(0,0,0,0)
        h = QFrame(); h.setObjectName("Header"); h.setFixedHeight(70); l.addWidget(h)
        hh = QHBoxLayout(h); hh.setContentsMargins(20,0,20,0)
        title = QLabel("AXIOM PRO v3.1.2"); title.setObjectName("T"); hh.addWidget(title)
        self.cmd_pal = QLineEdit(); self.cmd_pal.setPlaceholderText("Search..."); hh.addWidget(self.cmd_pal, 1)
        self.setDockOptions(QMainWindow.AllowTabbedDocks | QMainWindow.AnimatedDocks)
        self.ws = WorkspacePanel(self); self.ws.model.bind_store(self._workspace_named_vars, self._ans_pool)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.ws)
        self.fig = FigurePanel(self); self.addDockWidget(Qt.RightDockWidgetArea, self.fig)
        self.con = ConsolePanel(self); self.addDockWidget(Qt.BottomDockWidgetArea, self.con)
        self.sym = SymEnginePanel(self); self.addDockWidget(Qt.RightDockWidgetArea, self.sym)
        self.tabifyDockWidget(self.fig, self.sym)
        f = QFrame(); f.setFixedHeight(80); l.addWidget(f); fh = QHBoxLayout(f)
        self.s1 = HarmonicSparkline("#38C976", self); fh.addWidget(self.s1, 1)
        self.s2 = HarmonicSparkline("#6C5DD3", self); fh.addWidget(self.s2, 1)
        self.con.run_btn.clicked.connect(self._exec)

    def _exec(self):
        cmd = self.con.input.text(); self.con.output.appendPlainText(f">> {cmd}\nResult: 42")
        self.con.input.clear(); self.s1.push(math.sin(time.time())*0.5+0.5)

    def _drain(self):
        while not self._ui_tasks.empty(): self._ui_tasks.get()()

if __name__ == "__main__":
    app = QApplication(sys.argv); win = AxiomQtWindow(); win.showMaximized(); sys.exit(app.exec())
