#!/usr/bin/env python3
"""
Status Panel - Painel de Status do Emulador
===========================================

Painel que exibe informações de status do emulador,
incluindo estado da CPU, barramento e log de eventos.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-01-06
"""

from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QTextEdit,
    QPushButton, QGroupBox, QFrame, QSplitter, QGridLayout, QTabWidget
)
from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtGui import QFont

# Imports inteligentes para compatibilidade
try:
    from python_bindings.widgets.control_widgets import StatusLabel
except ImportError:
    from widgets.control_widgets import StatusLabel

class StatusPanel(QWidget):
    """Painel de status com abas: CPU, Barramento e Log"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("status-panel")
        self.setMinimumSize(200, 200)  # Reduzido de 300 para 200 pixels
        self.setProperty("class", "control-panel")  # padronizar cor de fundo
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(16)

        # Título principal
        title_label = QLabel("Painel de Status")
        title_label.setProperty("class", "green-label-large")
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title_label.setFont(QFont("Segoe UI Variable", 14, QFont.Weight.Bold))
        title_label.setMinimumHeight(40)
        layout.addWidget(title_label)

        # QTabWidget principal
        self.tab_widget = QTabWidget()
        self.tab_widget.setSizePolicy(self.sizePolicy())

        self.create_cpu_tab()
        self.create_bus_tab()
        self.create_log_tab()

        layout.addWidget(self.tab_widget)

    def create_cpu_tab(self):
        cpu_widget = QWidget()
        cpu_layout = QVBoxLayout(cpu_widget)
        cpu_layout.setContentsMargins(16, 16, 16, 16)
        cpu_layout.setSpacing(20)



        cpu_group = QGroupBox("Status da CPU")
        cpu_group.setProperty("class", "control-group")
        cpu_group_layout = QVBoxLayout(cpu_group)
        cpu_group_layout.setContentsMargins(16, 20, 16, 16)
        cpu_group_layout.setSpacing(16)

        cpu_grid = QGridLayout()
        cpu_grid.setSpacing(16)
        self.cpu_status_labels = {}
        status_items = [
            ("Estado:", "Parado"),
            ("Ciclos:", "0"),
            ("Instruções:", "0"),
            ("Tempo:", "00:00:00")
        ]
        for i, (label_text, value_text) in enumerate(status_items):
            label = QLabel(label_text)
            label.setProperty("class", "green-label-small")
            label.setFont(QFont("Consolas", 12, QFont.Weight.Bold))
            cpu_grid.addWidget(label, i, 0)
            value_label = StatusLabel(value_text)
            value_label.setFont(QFont("Consolas", 12, QFont.Weight.Bold))
            cpu_grid.addWidget(value_label, i, 1)
            self.cpu_status_labels[label_text] = value_label
        cpu_group_layout.addLayout(cpu_grid)
        cpu_layout.addWidget(cpu_group)
        cpu_layout.addStretch()
        self.tab_widget.addTab(cpu_widget, "CPU")

    def create_bus_tab(self):
        bus_widget = QWidget()
        bus_layout = QVBoxLayout(bus_widget)
        bus_layout.setContentsMargins(16, 16, 16, 16)
        bus_layout.setSpacing(20)



        bus_group = QGroupBox("Status do Barramento")
        bus_group.setProperty("class", "control-group")
        bus_group_layout = QVBoxLayout(bus_group)
        bus_group_layout.setContentsMargins(16, 20, 16, 16)
        bus_group_layout.setSpacing(16)

        bus_grid = QGridLayout()
        bus_grid.setSpacing(16)
        self.bus_status_labels = {}
        bus_items = [
            ("Endereço:", "0x0000"),
            ("Dados:", "0x00"),
            ("R/W:", "Read"),
            ("Ciclo:", "0")
        ]
        for i, (label_text, value_text) in enumerate(bus_items):
            label = QLabel(label_text)
            label.setProperty("class", "green-label-small")
            label.setFont(QFont("Consolas", 12, QFont.Weight.Bold))
            bus_grid.addWidget(label, i, 0)
            value_label = StatusLabel(value_text)
            value_label.setFont(QFont("Consolas", 12, QFont.Weight.Bold))
            bus_grid.addWidget(value_label, i, 1)
            self.bus_status_labels[label_text] = value_label
        bus_group_layout.addLayout(bus_grid)
        bus_layout.addWidget(bus_group)
        bus_layout.addStretch()
        self.tab_widget.addTab(bus_widget, "Barramento")

    def create_log_tab(self):
        log_widget = QWidget()
        log_layout = QVBoxLayout(log_widget)
        log_layout.setContentsMargins(16, 16, 16, 16)
        log_layout.setSpacing(20)



        # Campo de texto maior para o log
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setFont(QFont("Consolas", 12))
        self.log_text.setProperty("class", "log-area")
        self.log_text.setMinimumHeight(220)
        log_layout.addWidget(self.log_text, 1)

        # Botões de controle do log
        log_buttons = QHBoxLayout()
        log_buttons.setSpacing(12)
        self.btn_clear_log = QPushButton("Limpar")
        self.btn_clear_log.setProperty("class", "green-button")
        self.btn_clear_log.clicked.connect(self.clear_log)
        log_buttons.addWidget(self.btn_clear_log)
        self.btn_save_log = QPushButton("Salvar")
        self.btn_save_log.setProperty("class", "green-button")
        self.btn_save_log.clicked.connect(self.save_log)
        log_buttons.addWidget(self.btn_save_log)
        log_buttons.addStretch()
        log_layout.addLayout(log_buttons)
        log_layout.addStretch()
        self.tab_widget.addTab(log_widget, "Log")

    def update_cpu_status(self, status: dict):
        if 'estado' in status:
            self.cpu_status_labels["Estado:"].setText(status['estado'])
        if 'ciclos' in status:
            self.cpu_status_labels["Ciclos:"].setText(str(status['ciclos']))
        if 'instrucoes' in status:
            self.cpu_status_labels["Instruções:"].setText(str(status['instrucoes']))
        if 'tempo' in status:
            self.cpu_status_labels["Tempo:"].setText(status['tempo'])

    def update_bus_status(self, address: int, data: int, rw: bool):
        self.bus_status_labels["Endereço:"].setText(f"0x{address:04X}")
        self.bus_status_labels["Dados:"].setText(f"0x{data:02X}")
        self.bus_status_labels["R/W:"].setText("Write" if rw else "Read")

    def add_log_entry(self, message: str):
        from datetime import datetime
        timestamp = datetime.now().strftime("%H:%M:%S")
        log_entry = f"[{timestamp}] {message}"
        self.log_text.append(log_entry)
        cursor = self.log_text.textCursor()
        cursor.movePosition(cursor.MoveOperation.End)
        self.log_text.setTextCursor(cursor)

    def clear_log(self):
        self.log_text.clear()
        self.add_log_entry("Log limpo")

    def save_log(self):
        from PyQt6.QtWidgets import QFileDialog
        from datetime import datetime
        filename, _ = QFileDialog.getSaveFileName(
            self,
            "Salvar Log",
            f"emu65_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt",
            "Arquivos de Texto (*.txt);;Todos os Arquivos (*)"
        )
        if filename:
            try:
                with open(filename, 'w', encoding='utf-8') as f:
                    f.write(self.log_text.toPlainText())
                self.add_log_entry(f"Log salvo em: {filename}")
            except Exception as e:
                self.add_log_entry(f"Erro ao salvar log: {e}")

    def get_log_content(self) -> str:
        return self.log_text.toPlainText()