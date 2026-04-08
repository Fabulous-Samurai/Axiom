#!/usr/bin/env python3
"""
AXIOM Studio v1.1.0 (Engine v3.1.1)
Dockable, Functional, Engine-Integrated AXIOM Dashboard.
Theme: SHADOW SOFT (Alacakaranlık)
"""

import sys
import random
import queue
import time
import collections
from pathlib import Path

try:
    from PySide6.QtCore import Qt, QTimer, QRectF, QSize
    from PySide6.QtGui import (QColor, QFont, QPainter, QPainterPath, QPen, QBrush, 
                               QLinearGradient, QIcon)
    from PySide6.QtWidgets import (
        QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
        QLabel, QPushButton, QFrame, QTableWidget, QTableWidgetItem,
        QHeaderView, QScrollArea, QSizePolicy, QDockWidget, QPlainTextEdit,
        QLineEdit, QSplitter, QStackedWidget
    )
except ImportError:
    print("PySide6 is not installed.")
    sys.exit(1)

# --- CORE INTEGRATION ---
# Ensure project root is in sys.path for local imports
PROJECT_ROOT = Path(__file__).resolve().parents[2]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from gui.python.gui_helpers import CppEngineInterface

# --- SHADOW SOFT (Alacakaranlık) PALETİ ---
SHADOW_SOFT = {
    "surface":          "#1E293B",
    "canvas":           "#334155",
    "backdrop":         "#0F172A",
    "primary":          "#7DD3FC",
    "secondary":        "#C4B5FD",
    "accent":           "#67E8F9",
    "success":          "#6EE7B7",
    "warning":          "#FCD34D",
    "error":            "#FCA5A5",
    "info":             "#93C5FD",
    "text-primary":     "#E2E8F0",
    "text-secondary":   "#94A3B8",
    "text-tertiary":    "#64748B",
    "text-inverse":     "#1E293B",
    "border":           "#475569",
    "border-focus":     "#7DD3FC",
    "hover":            "#334155",
    "active":           "#155E75"
}

AXIOM_SEMANTIC = {
    "mode-algebraic":   "#8B5CF6",
    "mode-linear":      "#3B82F6",
    "mode-stats":       "#14B8A6",
    "mode-symbolic":    "#F59E0B",
    "mode-units":       "#EC4899",
    "mode-plot":        "#10B981"
}

AXIOM_QSS = f"""
QMainWindow, QWidget#MainContainer {{ background-color: {SHADOW_SOFT["backdrop"]}; color: {SHADOW_SOFT["text-primary"]}; }}

/* Sidebar & Navbar */
QFrame#Sidebar {{ background-color: {SHADOW_SOFT["surface"]}; border-right: 1px solid {SHADOW_SOFT["border"]}; }}
QFrame#Navbar {{ background-color: {SHADOW_SOFT["surface"]}; border-bottom: 1px solid {SHADOW_SOFT["border"]}; }}

/* Dock Widgets */
QDockWidget {{
    color: {SHADOW_SOFT["text-secondary"]};
    border: none;
}}
QDockWidget::title {{
    background-color: {SHADOW_SOFT["surface"]};
    padding: 10px;
    border-radius: 6px;
    font-weight: bold;
    font-size: 11px;
    text-transform: uppercase;
    color: {SHADOW_SOFT["text-tertiary"]};
    margin: 4px;
}}

/* Butonlar */
QPushButton#NavBtn {{
    background-color: transparent; color: {SHADOW_SOFT["text-secondary"]};
    text-align: left; padding: 12px 16px; border: none; border-radius: 6px;
    font-size: 14px; font-weight: 500;
}}
QPushButton#NavBtn:hover {{ background-color: {SHADOW_SOFT["hover"]}; color: {SHADOW_SOFT["text-primary"]}; }}
QPushButton#NavBtn:checked {{
    background-color: rgba(125, 211, 252, 0.1);
    color: {SHADOW_SOFT["primary"]}; border-left: 3px solid {SHADOW_SOFT["primary"]};
    font-weight: bold; border-radius: 0px 6px 6px 0px;
}}
QPushButton#SubNavBtn {{
    background-color: transparent; color: {SHADOW_SOFT["text-tertiary"]};
    text-align: left; padding: 8px 16px 8px 40px; border: none;
    font-size: 13px;
}}
QPushButton#SubNavBtn:hover {{ color: {SHADOW_SOFT["text-primary"]}; }}

/* Cards */
QFrame#MetricCard, QFrame#ChartCard, QFrame#TableCard {{
    background-color: {SHADOW_SOFT["canvas"]}; border: 1px solid {SHADOW_SOFT["border"]};
    border-radius: 8px;
}}

/* Input / Output */
QLineEdit {{
    background-color: {SHADOW_SOFT["surface"]};
    border: 1px solid {SHADOW_SOFT["border"]};
    color: {SHADOW_SOFT["text-primary"]};
    padding: 10px;
    border-radius: 6px;
    font-family: 'JetBrains Mono';
    font-size: 14px;
}}
QPlainTextEdit {{
    background-color: {SHADOW_SOFT["surface"]};
    border: none;
    color: {SHADOW_SOFT["text-primary"]};
    font-family: 'JetBrains Mono';
    font-size: 13px;
    padding: 8px;
}}

/* Tables */
QTableWidget {{
    background-color: {SHADOW_SOFT["canvas"]}; alternate-background-color: {SHADOW_SOFT["surface"]};
    color: {SHADOW_SOFT["text-primary"]}; border: none; gridline-color: transparent; 
    font-family: 'JetBrains Mono';
}}
QHeaderView::section {{
    background-color: {SHADOW_SOFT["surface"]}; color: {SHADOW_SOFT["text-secondary"]};
    border: none; border-bottom: 2px solid {SHADOW_SOFT["border"]}; padding: 12px 8px;
    font-weight: bold; font-size: 11px;
}}
"""

class MinimalSparkline(QWidget):
    def __init__(self, color_hex, parent=None):
        super().__init__(parent)
        self.color = QColor(color_hex)
        # ⚡ Bolt: Using deque with maxlen provides O(1) appends and evictions
        # compared to O(N) list.pop(0) shifts for real-time charting.
        self.data = collections.deque((random.uniform(0.2, 0.8) for _ in range(40)), maxlen=40)
        self.setFixedHeight(30)

    def update_data(self, val):
        self.data.append(val)
        self.update() 

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        w, h = self.width(), self.height()
        if w < 2: return
        path = QPainterPath()
        step = w / (len(self.data) - 1)
        for i, val in enumerate(self.data):
            x = i * step
            y = h - (val * h)
            if i == 0: path.moveTo(x, y)
            else: path.lineTo(x, y)
        painter.setPen(QPen(self.color, 2))
        painter.drawPath(path)

class CustomChart(QWidget):
    def __init__(self, title, color_hex, is_bar=False, parent=None):
        super().__init__(parent)
        self.color = QColor(color_hex)
        self.is_bar = is_bar
        self.data = [random.uniform(10, 80) for _ in range(25)]
        self.setMinimumHeight(180)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        w, h = self.width(), self.height()
        painter.setPen(QPen(QColor(SHADOW_SOFT["border"]), 1, Qt.DashLine))
        for i in range(1, 4):
            painter.drawLine(0, i * (h / 4), w, i * (h / 4))
        if self.is_bar:
            bar_w = (w / len(self.data)) - 4
            painter.setBrush(QBrush(self.color))
            painter.setPen(Qt.NoPen)
            for i, val in enumerate(self.data):
                bar_h = (val / 100.0) * h
                painter.drawRect(QRectF(i * (bar_w + 4) + 2, h - bar_h, bar_w, bar_h))
        else:
            path = QPainterPath()
            step = w / (len(self.data) - 1)
            for i, val in enumerate(self.data):
                x = i * step
                y = h - ((val / 100.0) * h)
                if i == 0: path.moveTo(x, y)
                else: path.lineTo(x, y)
            painter.setPen(QPen(self.color, 2.5))
            painter.drawPath(path)

class AxiomStudio(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("AXIOM Studio v1.1.0 [SHADOW SOFT]")
        self.resize(1440, 900)
        self.setStyleSheet(AXIOM_QSS)
        
        # Engine Initialization
        self.engine_exe = self._locate_engine()
        self.engine = CppEngineInterface(self.engine_exe)
        self._ui_tasks = queue.Queue()
        
        self._setup_main_ui()
        self._create_docks()
        
        self.timer = QTimer(self)
        self.timer.timeout.connect(self._pulse)
        self.timer.start(500)

    def _locate_engine(self):
        root = Path(__file__).resolve().parents[2]
        for c in ["build/axiom.exe", "axiom.exe", "build/axiom", "axiom"]:
            if (root / c).exists(): return str(root / c)
        return None

    def _setup_main_ui(self):
        central = QWidget()
        central.setObjectName("MainContainer")
        self.setCentralWidget(central)
        main_l = QVBoxLayout(central)
        main_l.setContentsMargins(0, 0, 0, 0)
        main_l.setSpacing(0)
        
        # Navbar
        nav = QFrame(); nav.setObjectName("Navbar"); nav.setFixedHeight(64)
        nh = QHBoxLayout(nav); nh.setContentsMargins(24, 0, 24, 0)
        logo = QLabel("●"); logo.setStyleSheet(f"color: {SHADOW_SOFT['accent']}; font-size: 20px;")
        title = QLabel("AXIOM Studio"); title.setStyleSheet("font-weight: 800; font-size: 18px;")
        nh.addWidget(logo); nh.addWidget(title); nh.addStretch()
        self.status_lbl = QLabel("Engine: IDLE"); self.status_lbl.setStyleSheet("font-family: 'JetBrains Mono';")
        nh.addWidget(self.status_lbl)
        main_l.addWidget(nav)
        
        # Body
        body = QHBoxLayout(); body.setSpacing(0)
        self.sidebar = self._build_sidebar()
        body.addWidget(self.sidebar)
        
        # Dashboard Content (Middle)
        self.stack = QStackedWidget()
        self.dashboard_view = self._build_dashboard_view()
        self.stack.addWidget(self.dashboard_view)
        body.addWidget(self.stack, 1)
        
        main_l.addLayout(body)
        self.setDockOptions(QMainWindow.AllowTabbedDocks | QMainWindow.AnimatedDocks)

    def _build_sidebar(self):
        side = QFrame(); side.setObjectName("Sidebar"); side.setFixedWidth(220)
        sl = QVBoxLayout(side); sl.setContentsMargins(8, 24, 8, 24); sl.setSpacing(4)
        
        self.btn_dash = QPushButton("  ⚡ Dashboard"); self.btn_dash.setObjectName("NavBtn")
        self.btn_dash.setCheckable(True); self.btn_dash.setChecked(True)
        sl.addWidget(self.btn_dash)
        
        modes_lbl = QLabel("ENGINE MODES"); modes_lbl.setStyleSheet("margin-top: 20px; color: #64748B; font-size: 11px; font-weight: 800; padding-left: 16px;")
        sl.addWidget(modes_lbl)
        
        for m, color in AXIOM_SEMANTIC.items():
            mode_name = m.split('-')[1]
            btn = QPushButton(f"  ● {mode_name.title()}")
            btn.setObjectName("SubNavBtn")
            btn.clicked.connect(lambda _=False, name=mode_name: self._switch_mode(name))
            sl.addWidget(btn)
            
        sl.addStretch()
        return side

    def _build_dashboard_view(self):
        scroll = QScrollArea(); scroll.setWidgetResizable(True); scroll.setFrameShape(QFrame.NoFrame)
        container = QWidget(); l = QVBoxLayout(container); l.setContentsMargins(32, 32, 32, 32); l.setSpacing(24)
        
        # KPI Row
        kpi_l = QHBoxLayout(); kpi_l.setSpacing(16)
        self.tp_lbl = QLabel("2.50M ops/sec")
        self.tp_spark = MinimalSparkline(SHADOW_SOFT["success"])
        kpi_l.addWidget(self._card("PARSER THROUGHPUT", self.tp_lbl, self.tp_spark))
        
        self.mem_lbl = QLabel("61.1 MB")
        self.mem_spark = MinimalSparkline(SHADOW_SOFT["info"])
        kpi_l.addWidget(self._card("TRACE MEMORY", self.mem_lbl, self.mem_spark))
        l.addLayout(kpi_l)
        
        # Charts Row
        chart_l = QHBoxLayout(); chart_l.setSpacing(16)
        c1 = QFrame(); c1.setObjectName("ChartCard"); c1v = QVBoxLayout(c1)
        c1v.setContentsMargins(20, 20, 20, 20)
        c1v.addWidget(QLabel("LATENCY SPECTRUM (p50/p99)")); c1v.addWidget(CustomChart("Lat", SHADOW_SOFT["primary"]))
        chart_l.addWidget(c1)
        l.addLayout(chart_l)
        
        scroll.setWidget(container)
        return scroll

    def _card(self, title, val_lbl, spark):
        f = QFrame(); f.setObjectName("MetricCard"); v = QVBoxLayout(f)
        v.setContentsMargins(20, 20, 20, 20)
        t = QLabel(title); t.setStyleSheet("color: #94A3B8; font-size: 11px; font-weight: 800;")
        val_lbl.setStyleSheet("font-size: 28px; font-weight: 800; font-family: 'JetBrains Mono'; color: #E2E8F0;")
        v.addWidget(t); v.addWidget(val_lbl); v.addWidget(spark)
        return f

    def _create_docks(self):
        # 1. Console Dock
        self.console_dock = QDockWidget("Surgical Console", self)
        self.console_dock.setObjectName("ConsoleDock")
        cw = QWidget(); cl = QVBoxLayout(cw)
        cl.setContentsMargins(10, 10, 10, 10)
        self.output = QPlainTextEdit()
        self.output.setReadOnly(True)
        self.output.setPlainText("AXIOM Studio Engine link established.\nType expression and press Enter.")
        self.input = QLineEdit(); self.input.setPlaceholderText(">> Enter expression (e.g. 2 + 3 * 4)")
        self.input.returnPressed.connect(self._execute)
        cl.addWidget(self.output, 1); cl.addWidget(self.input)
        self.console_dock.setWidget(cw)
        self.addDockWidget(Qt.BottomDockWidgetArea, self.console_dock)

        # 2. Session Dock
        self.session_dock = QDockWidget("History & Variables", self)
        self.session_dock.setObjectName("SessionDock")
        self.session_table = QTableWidget(0, 3)
        self.session_table.setHorizontalHeaderLabels(["Expression", "Result", "Latency"])
        self.session_table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.session_table.setAlternatingRowColors(True)
        self.session_dock.setWidget(self.session_table)
        self.addDockWidget(Qt.RightDockWidgetArea, self.session_dock)

    def _execute(self):
        cmd = self.input.text().strip()
        if not cmd: return
        self.input.clear()
        self.output.appendPlainText(f">> {cmd}")
        
        self.status_lbl.setText("Engine: BUSY")
        start_t = time.perf_counter()
        
        def on_res(res):
            lat = (time.perf_counter() - start_t) * 1000000
            self._ui_tasks.put(lambda: self._handle_result(cmd, res, lat))
        
        if self.engine:
            self.engine.execute_command_async(cmd, on_res)
        else:
            self.output.appendPlainText("Error: Engine binary not found in build/ directory.")

    def _handle_result(self, cmd, res, lat):
        self.status_lbl.setText("Engine: IDLE")
        if res.get("success"):
            val = res.get("result", "")
            self.output.appendPlainText(f"Result: {val}")
            
            row = self.session_table.rowCount()
            self.session_table.insertRow(row)
            self.session_table.setItem(row, 0, QTableWidgetItem(cmd))
            self.session_table.setItem(row, 1, QTableWidgetItem(str(val)))
            self.session_table.setItem(row, 2, QTableWidgetItem(f"{lat:.1f}µs"))
            self.session_table.scrollToBottom()
        else:
            self.output.appendPlainText(f"Error: {res.get('error')}")

    def _switch_mode(self, mode):
        if self.engine:
            self.engine.set_mode(mode)
            self.output.appendPlainText(f"\n--- SYSTEM: MODE SET TO [{mode.upper()}] ---")

    def _pulse(self):
        while not self._ui_tasks.empty():
            try:
                task = self._ui_tasks.get_nowait()
                if task: task()
            except Exception as e:
                print(f"[Studio] Deferred task error: {e}")
            
        # Real-time metrics simulation
        tp = 2.5 + random.uniform(-0.1, 0.2)
        self.tp_lbl.setText(f"{tp:.2f}M ops/sec")
        self.tp_spark.update_data(random.uniform(0.4, 0.8))
        self.mem_spark.update_data(random.uniform(0.2, 0.3))

def main():
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    win = AxiomStudio()
    win.show()
    return app.exec()

if __name__ == "__main__":
    sys.exit(main())
