#!/usr/bin/env python3
"""
Control Panel - Painel de Controle do Emulador 6502
==================================================

Widget com switches, LEDs, registradores e controles de clock
para interface didática do emulador.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-01-06
"""

from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QGridLayout, QLabel, QPushButton,
    QSlider, QCheckBox, QGroupBox, QFrame, QSizePolicy, QTabWidget
)
from PyQt6.QtCore import Qt, QTimer, pyqtSignal
from PyQt6.QtGui import QColor, QFont

# Imports inteligentes para compatibilidade
try:
    from python_bindings.widgets.control_widgets import LEDWidget, SwitchWidget, RegisterDisplay, StatusLabel
except ImportError:
    from widgets.control_widgets import LEDWidget, SwitchWidget, RegisterDisplay, StatusLabel

class ControlPanel(QWidget):
    """Painel de controle principal com abas organizadas"""

    # Sinais para comunicação com o core
    clock_changed = pyqtSignal(int)  # Frequência do clock
    reset_triggered = pyqtSignal()
    irq_triggered = pyqtSignal()
    nmi_triggered = pyqtSignal()
    step_triggered = pyqtSignal()
    run_triggered = pyqtSignal()
    stop_triggered = pyqtSignal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("control-panel")
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self.init_ui()

    def init_ui(self):
        """Inicializa a interface do usuário com abas"""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(16)

        # Título principal
        title_label = QLabel("Painel de Controle")
        title_label.setProperty("class", "green-label-large")
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title_label.setFont(QFont("Segoe UI Variable", 14, QFont.Weight.Bold))
        title_label.setMinimumHeight(40)
        layout.addWidget(title_label)

        # QTabWidget principal
        self.tab_widget = QTabWidget()
        self.tab_widget.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

        # Criar abas
        self.create_clock_tab()
        self.create_registers_tab()
        self.create_status_tab()

        layout.addWidget(self.tab_widget)

    def create_clock_tab(self):
        """Cria a aba de controles de clock"""
        clock_widget = QWidget()
        clock_layout = QVBoxLayout(clock_widget)
        clock_layout.setContentsMargins(16, 16, 16, 16)
        clock_layout.setSpacing(20)

        # Grupo de frequência
        freq_group = QGroupBox("Frequência do Clock")
        freq_group.setProperty("class", "control-group")
        freq_layout = QVBoxLayout(freq_group)
        freq_layout.setSpacing(16)
        freq_layout.setContentsMargins(16, 20, 16, 16)

        # Slider de frequência
        freq_slider_layout = QHBoxLayout()
        freq_slider_layout.setSpacing(12)

        freq_label = QLabel("Frequência:")
        freq_label.setProperty("class", "green-label")
        freq_label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        freq_label.setMinimumWidth(80)
        freq_slider_layout.addWidget(freq_label)

        self.freq_slider = QSlider(Qt.Orientation.Horizontal)
        self.freq_slider.setRange(1, 1000)  # 1Hz a 1MHz
        self.freq_slider.setValue(100)  # 100Hz default
        self.freq_slider.valueChanged.connect(self.on_freq_changed)
        self.freq_slider.setMinimumHeight(25)
        freq_slider_layout.addWidget(self.freq_slider)

        # Label do valor da frequência
        self.freq_label = QLabel("100 Hz")
        self.freq_label.setProperty("class", "green-label-large")
        self.freq_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.freq_label.setFont(QFont("Courier", 12, QFont.Weight.Bold))
        self.freq_label.setMinimumWidth(80)
        freq_slider_layout.addWidget(self.freq_label)

        freq_layout.addLayout(freq_slider_layout)
        clock_layout.addWidget(freq_group)

        # Grupo de botões principais
        main_buttons_group = QGroupBox("Controles Principais")
        main_buttons_group.setProperty("class", "control-group")
        main_buttons_layout = QVBoxLayout(main_buttons_group)
        main_buttons_layout.setSpacing(16)
        main_buttons_layout.setContentsMargins(16, 20, 16, 16)

        # Botões de controle em grid
        btn_grid = QGridLayout()
        btn_grid.setSpacing(12)

        self.btn_step = QPushButton("Step")
        self.btn_step.setProperty("class", "green-button")
        self.btn_step.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.btn_step.setMinimumHeight(35)
        self.btn_step.clicked.connect(self.on_step)
        btn_grid.addWidget(self.btn_step, 0, 0)

        self.btn_run = QPushButton("Run")
        self.btn_run.setProperty("class", "green-button")
        self.btn_run.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.btn_run.setMinimumHeight(35)
        self.btn_run.setCheckable(True)
        self.btn_run.clicked.connect(self.on_run)
        btn_grid.addWidget(self.btn_run, 0, 1)

        self.btn_stop = QPushButton("Stop")
        self.btn_stop.setProperty("class", "green-button")
        self.btn_stop.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.btn_stop.setMinimumHeight(35)
        self.btn_stop.clicked.connect(self.on_stop)
        btn_grid.addWidget(self.btn_stop, 1, 0)

        # Botão de reset na segunda linha
        self.btn_reset_exec = QPushButton("Reset")
        self.btn_reset_exec.setProperty("class", "green-button")
        self.btn_reset_exec.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.btn_reset_exec.setMinimumHeight(35)
        self.btn_reset_exec.clicked.connect(self.on_reset)
        btn_grid.addWidget(self.btn_reset_exec, 1, 1)

        main_buttons_layout.addLayout(btn_grid)
        clock_layout.addWidget(main_buttons_group)

        # Espaçador
        clock_layout.addStretch()

        self.tab_widget.addTab(clock_widget, "Clock")



    def create_registers_tab(self):
        """Cria a aba de registradores"""
        reg_widget = QWidget()
        reg_layout = QVBoxLayout(reg_widget)
        reg_layout.setContentsMargins(16, 16, 16, 16)
        reg_layout.setSpacing(20)

        # Grupo de registradores principais
        reg_group = QGroupBox("Registradores Principais")
        reg_group.setProperty("class", "control-group")
        reg_group_layout = QVBoxLayout(reg_group)
        reg_group_layout.setSpacing(16)
        reg_group_layout.setContentsMargins(16, 20, 16, 16)

        # Grid de registradores
        reg_grid = QGridLayout()
        reg_grid.setSpacing(16)

        # Registradores principais
        self.reg_a = RegisterDisplay("A")
        self.reg_x = RegisterDisplay("X")
        self.reg_y = RegisterDisplay("Y")
        self.reg_sp = RegisterDisplay("SP")
        self.reg_pc = RegisterDisplay("PC")
        self.reg_sr = RegisterDisplay("SR")

        # Layout em grid 2x3 com mais espaçamento
        reg_grid.addWidget(self.reg_a, 0, 0)
        reg_grid.addWidget(self.reg_x, 0, 1)
        reg_grid.addWidget(self.reg_y, 0, 2)
        reg_grid.addWidget(self.reg_sp, 1, 0)
        reg_grid.addWidget(self.reg_pc, 1, 1)
        reg_grid.addWidget(self.reg_sr, 1, 2)

        reg_group_layout.addLayout(reg_grid)
        reg_layout.addWidget(reg_group)

        # Grupo de interrupções
        irq_group = QGroupBox("Interrupções do Sistema")
        irq_group.setProperty("class", "control-group")
        irq_group_layout = QVBoxLayout(irq_group)
        irq_group_layout.setSpacing(16)
        irq_group_layout.setContentsMargins(16, 20, 16, 16)

        # Grid de botões de interrupção
        irq_grid = QGridLayout()
        irq_grid.setSpacing(16)

        # Botões de interrupção
        self.btn_reset_irq = QPushButton("Reset")
        self.btn_reset_irq.setProperty("class", "green-button")
        self.btn_reset_irq.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.btn_reset_irq.setMinimumHeight(35)
        self.btn_reset_irq.clicked.connect(self.on_reset)
        irq_grid.addWidget(self.btn_reset_irq, 0, 0)

        self.btn_irq = QPushButton("IRQ")
        self.btn_irq.setProperty("class", "green-button")
        self.btn_irq.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.btn_irq.setMinimumHeight(35)
        self.btn_irq.clicked.connect(self.on_irq)
        irq_grid.addWidget(self.btn_irq, 0, 1)

        self.btn_nmi = QPushButton("NMI")
        self.btn_nmi.setProperty("class", "green-button")
        self.btn_nmi.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.btn_nmi.setMinimumHeight(35)
        self.btn_nmi.clicked.connect(self.on_nmi)
        irq_grid.addWidget(self.btn_nmi, 1, 0)

        irq_group_layout.addLayout(irq_grid)
        reg_layout.addWidget(irq_group)

        # Espaçador
        reg_layout.addStretch()

        self.tab_widget.addTab(reg_widget, "Registradores")

    def create_status_tab(self):
        """Cria a aba de LEDs de status"""
        status_widget = QWidget()
        status_layout = QVBoxLayout(status_widget)
        status_layout.setContentsMargins(16, 16, 16, 16)
        status_layout.setSpacing(20)

        # Grupo de LEDs
        led_group = QGroupBox("Indicadores de Status")
        led_group.setProperty("class", "control-group")
        led_layout = QVBoxLayout(led_group)
        led_layout.setSpacing(16)
        led_layout.setContentsMargins(16, 20, 16, 16)

        # Grid de LEDs com mais espaçamento
        led_grid = QGridLayout()
        led_grid.setSpacing(20)

        # LEDs de status
        self.led_interrupt = LEDWidget(QColor(255, 0, 0))
        self.led_decimal = LEDWidget(QColor(0, 255, 0))
        self.led_break = LEDWidget(QColor(255, 255, 0))
        self.led_carry = LEDWidget(QColor(0, 0, 255))

        # Labels para os LEDs com melhor formatação
        irq_label = QLabel("IRQ (Interrupt)")
        irq_label.setProperty("class", "green-label")
        irq_label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        led_grid.addWidget(irq_label, 0, 0)
        led_grid.addWidget(self.led_interrupt, 0, 1)

        dec_label = QLabel("DEC (Decimal)")
        dec_label.setProperty("class", "green-label")
        dec_label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        led_grid.addWidget(dec_label, 0, 2)
        led_grid.addWidget(self.led_decimal, 0, 3)

        brk_label = QLabel("BRK (Break)")
        brk_label.setProperty("class", "green-label")
        brk_label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        led_grid.addWidget(brk_label, 1, 0)
        led_grid.addWidget(self.led_break, 1, 1)

        carry_label = QLabel("C (Carry)")
        carry_label.setProperty("class", "green-label")
        carry_label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        led_grid.addWidget(carry_label, 1, 2)
        led_grid.addWidget(self.led_carry, 1, 3)

        led_layout.addLayout(led_grid)
        status_layout.addWidget(led_group)

        # Espaçador
        status_layout.addStretch()

        self.tab_widget.addTab(status_widget, "Status")



    # Handlers de eventos
    def on_freq_changed(self, value):
        """Handler para mudança de frequência"""
        self.freq_label.setText(f"{value} Hz")
        self.clock_changed.emit(value)

    def on_step(self):
        """Handler para botão Step"""
        self.step_triggered.emit()

    def on_run(self, checked):
        """Handler para botão Run"""
        if checked:
            self.run_triggered.emit()
        else:
            self.stop_triggered.emit()

    def on_stop(self):
        """Handler para botão Stop"""
        self.btn_run.setChecked(False)
        self.stop_triggered.emit()

    def on_reset(self):
        """Handler para botão Reset"""
        self.reset_triggered.emit()

    def on_irq(self):
        """Handler para botão IRQ"""
        self.irq_triggered.emit()

    def on_nmi(self):
        """Handler para botão NMI"""
        self.nmi_triggered.emit()

    # Métodos públicos para atualização
    def update_registers(self, registers: dict):
        """Atualiza os valores dos registradores"""
        if 'A' in registers:
            self.reg_a.set_value(registers['A'])
        if 'X' in registers:
            self.reg_x.set_value(registers['X'])
        if 'Y' in registers:
            self.reg_y.set_value(registers['Y'])
        if 'SP' in registers:
            self.reg_sp.set_value(registers['SP'])
        if 'PC' in registers:
            self.reg_pc.set_value(registers['PC'])
        if 'SR' in registers:
            self.reg_sr.set_value(registers['SR'])

    def update_status_leds(self, status: dict):
        """Atualiza os LEDs de status"""
        if 'interrupt' in status:
            self.led_interrupt.set_state(status['interrupt'])
        if 'decimal' in status:
            self.led_decimal.set_state(status['decimal'])
        if 'break' in status:
            self.led_break.set_state(status['break'])
        if 'carry' in status:
            self.led_carry.set_state(status['carry'])

    def set_running_state(self, running: bool):
        """Define o estado de execução"""
        self.btn_run.setChecked(running)