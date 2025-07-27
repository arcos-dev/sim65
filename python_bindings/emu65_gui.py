#!/usr/bin/env python3
"""
EMU65 GUI - Interface Gráfica do Emulador 6502
==============================================

Interface gráfica principal do emulador 6502 com painéis
modulares e design responsivo.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-01-06
"""

import sys
import os
import json
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QLabel, QFrame,
    QTabWidget, QTextEdit, QGroupBox, QToolBar, QSplitter, QSizePolicy, QFileDialog,
    QGridLayout, QSpinBox, QMenuBar, QStatusBar, QMessageBox, QComboBox, QToolButton, QStyle,
    QInputDialog
)
from PyQt6.QtCore import Qt, QTimer, pyqtSignal, QSize, QPoint
from PyQt6.QtGui import QAction, QIcon, QFont, QPixmap, QPainter, QColor

# Importar módulos locais - compatibilidade com execução direta e como módulo
try:
    # Quando executado como módulo (pytest, etc.)
    from python_bindings.emu65_core import Emu65Core
    from python_bindings.widgets.lcd_widget import LCD16x2Widget
    from python_bindings.widgets.work_area import WorkAreaWidget
    from python_bindings.widgets.chip_widget import ChipWidget
    from python_bindings.panels.control_panel import ControlPanel
    from python_bindings.panels.memory_panel import MemoryPanel
    from python_bindings.panels.status_panel import StatusPanel
    from python_bindings.programs_6502 import Programs6502
except ImportError:
    # Quando executado diretamente
    from emu65_core import Emu65Core
    from widgets.lcd_widget import LCD16x2Widget
    from widgets.work_area import WorkAreaWidget
    from widgets.chip_widget import ChipWidget
    from panels.control_panel import ControlPanel
    from panels.memory_panel import MemoryPanel
    from panels.status_panel import StatusPanel
    from programs_6502 import Programs6502

class LogPanel(QWidget):
    """Painel de log para a aba lateral"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setProperty("class", "control-panel")  # padronizar cor de fundo
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(16)

        # Título
        title_label = QLabel("Log de Eventos")
        title_label.setProperty("class", "green-label-large")
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title_label.setFont(QFont("Segoe UI Variable", 14, QFont.Weight.Bold))
        title_label.setMinimumHeight(40)
        layout.addWidget(title_label)

        # Área de log
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setFont(QFont("Consolas", 12))
        self.log_text.setProperty("class", "log-area")
        layout.addWidget(self.log_text, 1)

        # Botões
        btns = QHBoxLayout()
        btns.setSpacing(12)
        self.btn_clear = QPushButton("Limpar")
        self.btn_clear.setProperty("class", "green-button")
        self.btn_save = QPushButton("Salvar")
        self.btn_save.setProperty("class", "green-button")
        btns.addWidget(self.btn_clear)
        btns.addWidget(self.btn_save)
        btns.addStretch()
        layout.addLayout(btns)

class Emu65MainWindow(QMainWindow):
    """Janela principal do emulador 6502"""

    def __init__(self):
        super().__init__()
        self.setWindowTitle("EMU65 - Emulador 6502")
        self.setMinimumSize(800, 600)

        # Defina primeiro os atributos usados pelo core
        self.is_running = False
        self.clock_frequency = 100  # <-- Defina aqui ANTES do init_core
        self.cycle_count = 0
        self.instruction_count = 0

        # Inicializar painéis ANTES de init_ui
        self.control_panel = ControlPanel()
        self.status_panel = StatusPanel()
        self.memory_panel = MemoryPanel()
        self.log_panel = LogPanel()

        # Widget LCD para referência
        self.lcd_widget = None

        # Agora sim, inicialize o core
        self.core = None
        self.settings_file = "emu65_settings.json"
        self.load_settings()  # <- Carregar antes de init_core
        self.init_core()

        # Timers
        self.clock_timer = QTimer()
        self.clock_timer.timeout.connect(self.clock_tick)

        # Interface
        self.init_ui()
        self.create_menu_bar()

        # Conectar sinais
        self.connect_signals()

        # Log inicial
        self.status_panel.add_log_entry("EMU65 iniciado")

    def init_core(self):
        """Inicializa o core do emulador"""
        try:
            self.core = Emu65Core({
                'clock_frequency': self.clock_frequency,
                'debug_mode': False,
                'trace_execution': False
            })
        except Exception as e:
            QMessageBox.critical(self, "Erro", f"Falha ao inicializar o core: {e}")
            sys.exit(1)

    def create_white_icon(self, standard_pixmap):
        """Cria um ícone branco a partir de um ícone padrão do sistema"""
        style = self.style()
        if not style:
            return QIcon()

        # Obtém o ícone padrão
        original_icon = style.standardIcon(standard_pixmap)
        if original_icon.isNull():
            return QIcon()

        # Obtém o pixmap do ícone
        pixmap = original_icon.pixmap(24, 24)
        if pixmap.isNull():
            return QIcon()

        # Cria um novo pixmap com fundo transparente
        white_pixmap = QPixmap(pixmap.size())
        white_pixmap.fill(QColor(0, 0, 0, 0))  # Fundo transparente

        # Desenha o ícone em branco
        painter = QPainter(white_pixmap)
        painter.setCompositionMode(QPainter.CompositionMode.CompositionMode_SourceOver)

        # Aplica uma máscara para tornar o ícone branco
        painter.setPen(QColor(255, 255, 255))  # Branco
        painter.setBrush(QColor(255, 255, 255))  # Branco

        # Desenha o ícone original como máscara
        painter.drawPixmap(0, 0, pixmap)
        painter.end()

        return QIcon(white_pixmap)

    def init_ui(self):
        """Inicializa a interface do usuário"""
        # Toolbar superior
        toolbar = QToolBar()
        self.addToolBar(Qt.ToolBarArea.TopToolBarArea, toolbar)
        toolbar.setMovable(False)
        toolbar.setProperty("class", "main-toolbar")

        # Botão carregar binário
        btn_load = QPushButton("Carregar Binário")
        btn_load.setProperty("class", "toolbar-button")
        btn_load.setMinimumHeight(35)
        btn_load.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        btn_load.clicked.connect(self.load_rom)
        toolbar.addWidget(btn_load)

        toolbar.addSeparator()

        # Botão carregar exemplo
        btn_example = QPushButton("Carregar Exemplo")
        btn_example.setProperty("class", "toolbar-button")
        btn_example.setMinimumHeight(35)
        btn_example.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        btn_example.clicked.connect(self.load_example)
        toolbar.addWidget(btn_example)

        toolbar.addSeparator()

        # ComboBox de componentes
        component_label = QLabel("Adicionar componente:")
        component_label.setProperty("class", "toolbar-label")
        component_label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        toolbar.addWidget(component_label)

        self.component_combo = QComboBox()
        self.component_combo.setProperty("class", "toolbar-combo")
        self.component_combo.setMinimumHeight(35)
        self.component_combo.setMinimumWidth(150)
        self.component_combo.addItems(["6502 CPU", "RAM", "ROM", "LCD 16x2", "Botão", "LED", "Resistor", "Capacitor"])
        toolbar.addWidget(self.component_combo)

        btn_add = QPushButton("Adicionar")
        btn_add.setProperty("class", "toolbar-button")
        btn_add.setMinimumHeight(35)
        btn_add.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        btn_add.clicked.connect(self.on_add_component)
        toolbar.addWidget(btn_add)

        toolbar.addSeparator()

        # Controles do grid
        grid_label = QLabel("Grid:")
        grid_label.setProperty("class", "toolbar-label")
        grid_label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        toolbar.addWidget(grid_label)

        # Botão para alternar grid
        self.grid_toggle_button = QPushButton("Grid")
        self.grid_toggle_button.setProperty("class", "toolbar-button")
        self.grid_toggle_button.setMinimumHeight(35)
        self.grid_toggle_button.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.grid_toggle_button.setCheckable(True)
        self.grid_toggle_button.setChecked(True)
        self.grid_toggle_button.clicked.connect(self.toggle_grid)
        toolbar.addWidget(self.grid_toggle_button)

        # Botão para alternar snap-to-grid
        self.snap_toggle_button = QPushButton("Snap")
        self.snap_toggle_button.setProperty("class", "toolbar-button")
        self.snap_toggle_button.setMinimumHeight(35)
        self.snap_toggle_button.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.snap_toggle_button.setCheckable(True)
        self.snap_toggle_button.setChecked(True)
        self.snap_toggle_button.clicked.connect(self.toggle_snap_to_grid)
        toolbar.addWidget(self.snap_toggle_button)

        # SpinBox para tamanho do grid
        self.grid_size_spin = QSpinBox()
        self.grid_size_spin.setProperty("class", "toolbar-combo")
        self.grid_size_spin.setMinimumHeight(35)
        self.grid_size_spin.setRange(10, 50)
        self.grid_size_spin.setValue(20)
        self.grid_size_spin.setSuffix("px")
        self.grid_size_spin.setMinimumWidth(80)
        self.grid_size_spin.setMaximumWidth(100)
        self.grid_size_spin.valueChanged.connect(self.set_grid_size)
        toolbar.addWidget(self.grid_size_spin)

        toolbar.addSeparator()

        # Widget central
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)

        # QSplitter principal
        splitter = QSplitter(Qt.Orientation.Horizontal)
        main_layout.addWidget(splitter, 1)

        # Área central arrastável
        self.work_area = WorkAreaWidget()
        self.work_area.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        splitter.addWidget(self.work_area)
        splitter.setStretchFactor(0, 3)  # Central: mais espaço

        # Conectar sinais do work area (após criar o work_area)
        self.work_area.component_moved.connect(self.on_component_moved)
        self.work_area.component_rotated.connect(self.on_component_rotated)

        # Painel lateral direito (QTabWidget)
        self.side_tabs = QTabWidget()
        self.side_tabs.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self.side_tabs.setMinimumWidth(200)  # Reduzido de ~300 para 200 pixels

        # Configurar políticas de tamanho para todos os painéis
        for panel in [self.control_panel, self.status_panel, self.memory_panel, self.log_panel]:
            panel.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

        self.side_tabs.addTab(self.control_panel, "Controle")
        self.side_tabs.addTab(self.status_panel, "Status")
        self.side_tabs.addTab(self.memory_panel, "Memória")
        self.side_tabs.addTab(self.log_panel, "Log")

        # Adicionar ao splitter principal
        splitter.addWidget(self.side_tabs)
        splitter.setStretchFactor(1, 1)  # Lateral: menos espaço

        # Definir tamanhos padrão
        splitter.setSizes([int(self.width()*0.7), int(self.width()*0.3)])

        # Barra inferior com botões centralizados
        bottom_bar = QHBoxLayout()
        bottom_bar.setSpacing(8)  # Espaçamento entre botões reduzido
        bottom_bar.setContentsMargins(12, 4, 12, 4)  # Margens da barra mais compactas
        bottom_bar.addStretch()

        # Botões com tamanho e estilo padronizados
        self.btn_play = QToolButton()
        self.btn_play.setProperty("class", "toolbar-button")
        self.btn_play.setMinimumSize(44, 44)
        self.btn_play.setMaximumSize(42, 42)
        self.btn_play.setIconSize(QSize(32, 32))
        style = self.style()
        if style:
            self.btn_play.setIcon(style.standardIcon(QStyle.StandardPixmap.SP_MediaPlay))
        self.btn_play.clicked.connect(self.on_run)
        self.btn_play.setToolTip("Executar")
        bottom_bar.addWidget(self.btn_play)

        self.btn_pause = QToolButton()
        self.btn_pause.setProperty("class", "toolbar-button")
        self.btn_pause.setMinimumSize(44, 44)
        self.btn_pause.setMaximumSize(42, 42)
        self.btn_pause.setIconSize(QSize(32, 32))
        if style:
            self.btn_pause.setIcon(style.standardIcon(QStyle.StandardPixmap.SP_MediaPause))
        self.btn_pause.clicked.connect(self.on_stop)
        self.btn_pause.setToolTip("Pausar")
        bottom_bar.addWidget(self.btn_pause)

        self.btn_stop = QToolButton()
        self.btn_stop.setProperty("class", "toolbar-button")
        self.btn_stop.setMinimumSize(44, 44)
        self.btn_stop.setMaximumSize(42, 42)
        self.btn_stop.setIconSize(QSize(28, 28))
        if style:
            self.btn_stop.setIcon(style.standardIcon(QStyle.StandardPixmap.SP_MediaStop))
        self.btn_stop.clicked.connect(self.on_stop)
        self.btn_stop.setToolTip("Parar")
        bottom_bar.addWidget(self.btn_stop)

        self.btn_reset = QToolButton()
        self.btn_reset.setObjectName("reset-button")
        self.btn_reset.setProperty("class", "toolbar-button")
        self.btn_reset.setMinimumSize(44, 44)
        self.btn_reset.setMaximumSize(42, 42)
        self.btn_reset.setIconSize(QSize(36, 36))
        self.btn_reset.setText("↻")
        self.btn_reset.setFont(QFont("Segoe UI Variable", 32, QFont.Weight.Bold))
        self.btn_reset.clicked.connect(self.on_reset)
        self.btn_reset.setToolTip("Resetar")
        bottom_bar.addWidget(self.btn_reset)

        bottom_bar.addStretch()
        main_layout.addLayout(bottom_bar)

        # Barra de status
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)

        # Inicializar status do grid
        self.update_grid_status()

    def create_menu_bar(self):
        """Cria a barra de menu"""
        menubar = self.menuBar()
        if menubar:
            # Menu Arquivo
            file_menu = menubar.addMenu("&Arquivo")
            if file_menu:
                # Ação Carregar ROM
                load_action = QAction("&Carregar ROM...", self)
                load_action.setShortcut("Ctrl+O")
                load_action.triggered.connect(self.load_rom)
                file_menu.addAction(load_action)

                file_menu.addSeparator()

                # Ação Sair
                exit_action = QAction("&Sair", self)
                exit_action.setShortcut("Ctrl+Q")
                exit_action.triggered.connect(self.close)
                file_menu.addAction(exit_action)

            # Menu Emulação
            emu_menu = menubar.addMenu("&Emulação")
            if emu_menu:
                # Ação Reset
                reset_action = QAction("&Reset", self)
                reset_action.setShortcut("F5")
                reset_action.triggered.connect(self.on_reset)
                emu_menu.addAction(reset_action)

                emu_menu.addSeparator()

                # Ação Step
                step_action = QAction("&Step", self)
                step_action.setShortcut("F10")
                step_action.triggered.connect(self.on_step)
                emu_menu.addAction(step_action)

            # Menu Ajuda
            help_menu = menubar.addMenu("&Ajuda")
            if help_menu:
                # Ação Sobre
                about_action = QAction("&Sobre", self)
                about_action.triggered.connect(self.show_about)
                help_menu.addAction(about_action)

    def connect_signals(self):
        """Conecta os sinais dos painéis"""
        # Sinais do painel de controle
        self.control_panel.clock_changed.connect(self.on_clock_changed)
        self.control_panel.reset_triggered.connect(self.on_reset)
        self.control_panel.irq_triggered.connect(self.on_irq)
        self.control_panel.nmi_triggered.connect(self.on_nmi)
        self.control_panel.step_triggered.connect(self.on_step)
        self.control_panel.run_triggered.connect(self.on_run)
        self.control_panel.stop_triggered.connect(self.on_stop)

    def load_settings(self):
        """Carrega as configurações"""
        try:
            if os.path.exists(self.settings_file):
                with open(self.settings_file, 'r') as f:
                    settings = json.load(f)
                # Restaurar tamanho e posição da janela
                if 'window_size' in settings:
                    w = settings['window_size'].get('width', 1000)
                    h = settings['window_size'].get('height', 700)
                    self.resize(w, h)
                if 'window_pos' in settings:
                    x = settings['window_pos'].get('x', 100)
                    y = settings['window_pos'].get('y', 100)
                    self.move(x, y)
                # Restaurar clock_frequency se existir
                if 'clock_frequency' in settings:
                    self.clock_frequency = settings['clock_frequency']
        except Exception as e:
            print(f"Erro ao carregar configurações: {e}")

    def save_settings(self):
        """Salva as configurações"""
        try:
            settings = {}
            # Salvar tamanho e posição da janela
            size = self.size()
            pos = self.pos()
            settings['window_size'] = {'width': size.width(), 'height': size.height()}
            settings['window_pos'] = {'x': pos.x(), 'y': pos.y()}
            settings['clock_frequency'] = self.clock_frequency

            with open(self.settings_file, 'w') as f:
                json.dump(settings, f, indent=2)
        except Exception as e:
            print(f"Erro ao salvar configurações: {e}")

    # Handlers de eventos
    def on_clock_changed(self, frequency):
        """Handler para mudança de frequência do clock"""
        self.clock_frequency = frequency
        if self.is_running:
            self.clock_timer.setInterval(1000 // frequency)
        self.status_panel.add_log_entry(f"Frequência alterada para {frequency} Hz")

    def on_reset(self):
        """Handler para reset"""
        self.stop_emulation()
        if self.core:
            try:
                result = self.core.reset()
                if result != 0:
                    self.status_panel.add_log_entry(f"Erro no reset: {result}")
                else:
                    self.status_panel.add_log_entry("Reset executado com sucesso")
            except Exception as e:
                self.status_panel.add_log_entry(f"Erro no reset: {e}")
        self.cycle_count = 0
        self.instruction_count = 0
        self.update_status()

    def on_irq(self):
        """Handler para IRQ"""
        self.status_panel.add_log_entry("IRQ acionado")

    def on_nmi(self):
        """Handler para NMI"""
        self.status_panel.add_log_entry("NMI acionado")

    def on_step(self):
        """Handler para step"""
        if self.core:
            try:
                self.status_panel.add_log_entry(f"DEBUG: Executando step {self.cycle_count + 1}")
                result = self.core.step()
                self.cycle_count += 1
                self.instruction_count += 1

                # Atualizar estado do barramento
                bus_state = self.core.get_bus_state()
                self.status_panel.update_bus_status(
                    bus_state.address, bus_state.data, bus_state.rw
                )

                # DEBUG: Log antes de atualizar LCD
                self.status_panel.add_log_entry(f"DEBUG: Obtendo estado do LCD...")

                # Atualizar LCD - sempre verificar mudanças
                lcd_state = self.core.get_lcd_state()
                self.status_panel.add_log_entry(f"DEBUG: Estado LCD obtido, chamando update_lcd_display...")
                self.update_lcd_display(lcd_state)

                # Atualizar status da interface
                self.update_status()

                # Log de debug para endereços de I/O
                if 0x6000 <= bus_state.address <= 0x6003:
                    self.status_panel.add_log_entry(
                        f"I/O: addr=0x{bus_state.address:04X} data=0x{bus_state.data:02X} "
                        f"rw={'R' if bus_state.rw else 'W'}"
                    )

            except Exception as e:
                self.status_panel.add_log_entry(f"Erro no step: {e}")
                import traceback
                traceback.print_exc()

    def on_run(self):
        """Handler para run"""
        self.start_emulation()

    def on_stop(self):
        """Handler para stop"""
        self.stop_emulation()

    def start_emulation(self):
        """Inicia a emulação"""
        if not self.is_running:
            self.is_running = True
            self.clock_timer.start(1000 // self.clock_frequency)
            self.control_panel.set_running_state(True)
            self.status_panel.add_log_entry("Emulação iniciada")
            self.status_bar.showMessage("Executando...")

    def stop_emulation(self):
        """Para a emulação de forma robusta"""
        if self.is_running:
            self.is_running = False
            # Parar timer com verificação
            if hasattr(self, 'clock_timer') and self.clock_timer and self.clock_timer.isActive():
                self.clock_timer.stop()

            # Atualizar estado dos controles
            if hasattr(self, 'control_panel') and self.control_panel:
                self.control_panel.set_running_state(False)

            # Log e status
            if hasattr(self, 'status_panel') and self.status_panel:
                self.status_panel.add_log_entry("Emulação parada")

            if hasattr(self, 'status_bar') and self.status_bar:
                self.status_bar.showMessage("Parado")

    def clock_tick(self):
        """Tick do clock"""
        if self.core and self.is_running:
            try:
                # Executa um passo do emulador
                result = self.core.step()
                self.cycle_count += 1
                self.instruction_count += 1

                # Atualiza o estado do LCD
                lcd_state = self.core.get_lcd_state()
                self.update_lcd_display(lcd_state)

                # Atualiza o status
                self.update_status()

            except Exception as e:
                self.status_panel.add_log_entry(f"Erro no clock tick: {e}")
                self.stop_emulation()

    def update_lcd_display(self, lcd_state):
        """Atualiza o display LCD"""
        try:
            # DEBUG: Log do estado recebido
            self.status_panel.add_log_entry(f"DEBUG: update_lcd_display chamado, lcd_state existe: {lcd_state is not None}")

            if not self.lcd_widget:
                # Procurar por widget LCD na área de trabalho se não estiver definido
                lcd_widgets = self.work_area.findChildren(LCD16x2Widget)
                self.status_panel.add_log_entry(f"DEBUG: Encontrados {len(lcd_widgets)} widgets LCD")
                for widget in lcd_widgets:
                    self.lcd_widget = widget
                    self.status_panel.add_log_entry(f"DEBUG: LCD widget definido: {widget}")
                    break

            if not self.lcd_widget:
                self.status_panel.add_log_entry("DEBUG: Nenhum widget LCD encontrado!")
                return

            if lcd_state is None:
                self.status_panel.add_log_entry("DEBUG: lcd_state é None!")
                return

            # DEBUG: Log dos dados brutos
            display_raw = lcd_state.display
            self.status_panel.add_log_entry(f"DEBUG: LCD display raw length: {len(display_raw)}")
            self.status_panel.add_log_entry(f"DEBUG: LCD display raw bytes: {[hex(b) for b in display_raw[:20]]}")

            # Converte o array de bytes para string
            display_bytes = bytes(display_raw)
            display_text = display_bytes.decode('ascii', errors='replace')
            self.status_panel.add_log_entry(f"DEBUG: LCD display text: '{display_text[:32]}' (total: {len(display_text)} chars)")

            # DEBUG: Estados importantes do LCD
            self.status_panel.add_log_entry(f"DEBUG: LCD Estados - display_on={lcd_state.display_on}, cursor_on={lcd_state.cursor_on}, blink_on={lcd_state.blink_on}")
            self.status_panel.add_log_entry(f"DEBUG: LCD Cursor - row={lcd_state.cursor_row}, col={lcd_state.cursor_col}")
            self.status_panel.add_log_entry(f"DEBUG: LCD Busy={lcd_state.busy}, function_set=0x{lcd_state.function_set:02X}")

            # O formato no C é: display[2][17] - 2 linhas de 16 chars + null terminator
            # Então temos 34 bytes total: linha1 (17 bytes) + linha2 (17 bytes)
            # Estrutura: [linha1_16chars][null][linha2_16chars][null]
            if len(display_text) >= 34:
                # Encontra o primeiro null terminator
                null_pos1 = display_text.find('\x00')
                if null_pos1 >= 0:
                    row1 = display_text[:null_pos1]  # Primeiros chars até o null
                    # Pula o null e pega os próximos chars até o segundo null
                    remaining = display_text[null_pos1 + 1:]
                    null_pos2 = remaining.find('\x00')
                    if null_pos2 >= 0:
                        row2 = remaining[:null_pos2]
                    else:
                        row2 = remaining
                else:
                    # Fallback se não encontrar null terminators
                    row1 = display_text[:16]
                    row2 = display_text[17:33] if len(display_text) >= 33 else ""
            else:
                # Fallback se o formato não estiver correto
                row1 = display_text[:16] if len(display_text) >= 16 else ""
                row2 = display_text[16:32] if len(display_text) >= 32 else ""

            self.status_panel.add_log_entry(f"DEBUG: Rows extraídas - row1: '{row1}', row2: '{row2}'")

            # IMPORTANTE: Só atualizar se há mudança real ou se display está ligado
            current_text_row1 = self.lcd_widget.display_text[0].strip() if self.lcd_widget.display_text else ""
            current_text_row2 = self.lcd_widget.display_text[1].strip() if self.lcd_widget.display_text else ""

            if (row1 != current_text_row1 or row2 != current_text_row2 or
                lcd_state.display_on != self.lcd_widget.display_on):

                # Atualizar widget LCD
                self.lcd_widget.set_display_text(row1, row2)
                self.lcd_widget.set_cursor(lcd_state.cursor_row, lcd_state.cursor_col)
                self.lcd_widget.set_display_on(lcd_state.display_on)
                self.lcd_widget.set_cursor_visible(lcd_state.cursor_on)
                self.lcd_widget.set_blink_on(lcd_state.blink_on)

                # Forçar repaint do widget
                self.lcd_widget.update()

                # Log da mudança
                self.status_panel.add_log_entry(f"LCD MUDANÇA DETECTADA: '{row1}' | '{row2}' (ON:{lcd_state.display_on}, CURSOR:{lcd_state.cursor_on}, BLINK:{lcd_state.blink_on})")
            else:
                self.status_panel.add_log_entry(f"LCD SEM MUDANÇA: '{row1}' | '{row2}' (ON:{lcd_state.display_on})")

        except Exception as e:
            self.status_panel.add_log_entry(f"Erro ao atualizar LCD: {e}")
            import traceback
            self.status_panel.add_log_entry(f"Traceback: {traceback.format_exc()}")

    def update_status(self):
        """Atualiza o status da interface"""
        # Atualizar painel de status
        cpu_status = {
            'estado': 'Executando' if self.is_running else 'Parado',
            'ciclos': self.cycle_count,
            'instrucoes': self.instruction_count,
            'tempo': f"{self.cycle_count // 1000000:02d}:{(self.cycle_count // 10000) % 100:02d}:{(self.cycle_count // 100) % 100:02d}"
        }
        self.status_panel.update_cpu_status(cpu_status)

        # Atualizar registradores (simulação)
        registers = {
            'A': 0x00,
            'X': 0x00,
            'Y': 0x00,
            'SP': 0xFF,
            'PC': 0x8000,
            'SR': 0x00
        }
        self.control_panel.update_registers(registers)

        # Atualizar LEDs de status
        status_leds = {
            'interrupt': False,
            'decimal': False,
            'break': False,
            'carry': False
        }
        self.control_panel.update_status_leds(status_leds)

    def load_rom(self):
        """Carrega uma ROM"""
        filename, _ = QFileDialog.getOpenFileName(
            self,
            "Carregar ROM",
            "",
            "Arquivos Binários (*.bin *.rom);;Todos os Arquivos (*)"
        )

        if filename:
            try:
                with open(filename, 'rb') as f:
                    data = f.read()

                # Aqui você implementaria o carregamento no core
                self.status_panel.add_log_entry(f"ROM carregada: {filename}")

            except Exception as e:
                self.status_panel.add_log_entry(f"Erro ao carregar ROM: {e}")

    def load_example(self):
        """Carrega um exemplo completo usando Programs6502"""
        try:
            # Obter todos os exemplos
            examples = Programs6502.get_all_programs()
            # Criar lista de nomes para o diálogo
            example_names = [ex['name'] for ex in examples]
            # Mostrar diálogo de seleção
            name, ok = QInputDialog.getItem(
                self,
                "Selecionar Exemplo",
                "Escolha um exemplo para carregar:",
                example_names,
                0,
                False
            )
            if ok and name:
                # Encontrar o exemplo selecionado
                selected_example = None
                for ex in examples:
                    if ex['name'] == name:
                        selected_example = ex
                        break
                if selected_example and self.core:
                    # Parar emulação se estiver rodando
                    if self.is_running:
                        self.stop_emulation()
                    # Reset do sistema ANTES de carregar o programa
                    self.core.reset()
                    # Carregar o binário diretamente
                    self.core.load_program(selected_example['binary'], selected_example['start_address'])
                    # Reset novamente para garantir que o PC está correto
                    self.core.reset()
                    # Limpar área de trabalho e adicionar componentes necessários
                    self.work_area.clear_components()
                    self.add_required_components(selected_example.get('components', []))

                    # TESTE: Verificar se LCD foi adicionado e testar funcionamento
                    if 'LCD 16x2' in selected_example.get('components', []):
                        self.status_panel.add_log_entry("DEBUG: Verificando LCD após carregar componentes...")
                        lcd_state = self.core.get_lcd_state()
                        self.update_lcd_display(lcd_state)

                        # Verificar se LCD widget foi criado corretamente
                        if self.lcd_widget:
                            self.status_panel.add_log_entry(f"DEBUG: LCD widget válido: {self.lcd_widget}")
                            self.status_panel.add_log_entry(f"DEBUG: LCD widget visível: {self.lcd_widget.isVisible()}")
                            self.status_panel.add_log_entry(f"DEBUG: LCD widget size: {self.lcd_widget.size()}")
                            self.status_panel.add_log_entry(f"DEBUG: LCD widget parent: {self.lcd_widget.parent()}")
                        else:
                            self.status_panel.add_log_entry("DEBUG: LCD widget não encontrado!")

                        # Verificar estado inicial do LCD no core
                        try:
                            lcd_state_after = self.core.get_lcd_state()
                            self.status_panel.add_log_entry(f"DEBUG: Estado do LCD no core: display_on={lcd_state_after.display_on}")
                            self.status_panel.add_log_entry(f"DEBUG: LCD display inicial: {[hex(b) for b in lcd_state_after.display[:16]]}")
                        except Exception as e:
                            self.status_panel.add_log_entry(f"ERRO ao verificar estado do LCD: {e}")

                    # Atualizar status
                    self.update_status()
                    # Logs
                    self.status_panel.add_log_entry(f"Exemplo carregado: {selected_example['name']}")
                    self.status_panel.add_log_entry(f"Descrição: {selected_example['description']}")
                    self.status_panel.add_log_entry(f"Endereço inicial: 0x{selected_example['start_address']:04X}")
                    self.status_panel.add_log_entry(f"Tamanho do binário: {len(selected_example['binary'])} bytes")
                    # Mensagem de sucesso
                    QMessageBox.information(
                        self,
                        "Exemplo Carregado",
                        f"Exemplo '{selected_example['name']}' carregado com sucesso!\n\n"
                        f"Descrição: {selected_example['description']}\n"
                        f"Endereço inicial: 0x{selected_example['start_address']:04X}\n"
                        f"Tamanho: {len(selected_example['binary'])} bytes\n\n"
                        f"Use Step/Run para executar o programa."
                    )
        except Exception as e:
            self.status_panel.add_log_entry(f"Erro ao carregar exemplo: {e}")
            import traceback
            traceback.print_exc()

    def add_basic_components(self):
        """Adiciona componentes básicos à área de trabalho"""
        try:
            # Obter tamanho do grid para posicionamento adequado
            grid_size = self.work_area.grid_size

            # Adicionar CPU 6502 (posição 2,2 no grid)
            cpu_pins = [
                "VCC", "RDY", "IRQ", "NMI", "SYNC", "A0", "A1", "A2",
                "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10",
                "A11", "A12", "A13", "A14", "A15", "D0", "D1", "D2",
                "D3", "D4", "D5", "D6", "D7", "R/W", "PHI2", "RES"
            ]
            cpu = ChipWidget(name="6502 CPU", pin_names=cpu_pins)
            cpu.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
            self.work_area.add_component(cpu, QPoint(grid_size * 2, grid_size * 2))

            # Adicionar LCD (posição 8,2 no grid)
            lcd = LCD16x2Widget()
            lcd.setMinimumSize(180, 60)
            lcd.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
            self.work_area.add_component(lcd, QPoint(grid_size * 8, grid_size * 2))
            self.lcd_widget = lcd

            # Adicionar RAM (posição 5,2 no grid)
            ram_pins = [
                "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
                "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
                "WE", "OE", "CS", "VCC", "GND"
            ]
            ram = ChipWidget(name="RAM", pin_names=ram_pins)
            ram.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
            self.work_area.add_component(ram, QPoint(grid_size * 5, grid_size * 2))

            # Adicionar ROM (posição 5,6 no grid)
            rom_pins = [
                "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
                "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15",
                "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
                "OE", "CE", "VCC", "GND"
            ]
            rom = ChipWidget(name="ROM", pin_names=rom_pins)
            rom.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
            self.work_area.add_component(rom, QPoint(grid_size * 5, grid_size * 6))

            self.status_panel.add_log_entry("Componentes básicos adicionados: CPU, LCD, RAM, ROM")

        except Exception as e:
            self.status_panel.add_log_entry(f"Erro ao adicionar componentes: {e}")

    def show_about(self):
        """Mostra a janela sobre"""
        QMessageBox.about(
            self,
            "Sobre EMU65",
            "EMU65 - Emulador 6502\n\n"
            "Versão 2.0.0\n"
            "Autor: Anderson Costa\n\n"
            "Interface gráfica modular para emulação do processador 6502."
        )

    def closeEvent(self, event):
        """Evento de fechamento da janela com cleanup completo"""
        try:
            # 1. Parar emulação se estiver rodando
            if self.is_running:
                self.stop_emulation()

            # 2. Parar explicitamente o timer principal
            if hasattr(self, 'clock_timer') and self.clock_timer:
                self.clock_timer.stop()
                self.clock_timer.timeout.disconnect()
                self.clock_timer = None

            # 3. Limpar todos os widgets LCD (que têm blink_timer)
            lcd_widgets = self.findChildren(LCD16x2Widget)
            for lcd in lcd_widgets:
                if hasattr(lcd, 'blink_timer') and lcd.blink_timer:
                    lcd.blink_timer.stop()
                    lcd.blink_timer.timeout.disconnect()
                    lcd.blink_timer = None

            # 4. Limpar componentes da work area
            if hasattr(self, 'work_area') and self.work_area:
                self.work_area.clear_components()

            # 5. Salvar configurações
            self.save_settings()

            # 6. Destruir core do emulador (com timeout)
            if hasattr(self, 'core') and self.core:
                try:
                    # Usar QTimer.singleShot para timeout na destruição
                    import threading

                    def destroy_core():
                        try:
                            self.core.destroy()
                        except Exception as e:
                            print(f"Erro na destruição do core: {e}")

                    # Executar destruição em thread separada com timeout
                    destroy_thread = threading.Thread(target=destroy_core)
                    destroy_thread.daemon = True  # Thread daemon para não bloquear o fechamento
                    destroy_thread.start()
                    destroy_thread.join(timeout=2.0)  # Timeout de 2 segundos

                    if destroy_thread.is_alive():
                        print("Warning: Core cleanup timeout - forçando fechamento")

                except Exception as e:
                    print(f"Erro no cleanup do core: {e}")
                finally:
                    self.core = None

            # 7. Forçar processamento de eventos pendentes
            if hasattr(self, 'app') and self.app:
                self.app.processEvents()

        except Exception as e:
            print(f"Erro durante closeEvent: {e}")
        finally:
            # Sempre aceitar o evento de fechamento
            event.accept()

    def on_component_moved(self, widget, new_position):
        """Handler para movimento de componente"""
        self.status_panel.add_log_entry(f"Componente movido para ({new_position.x()}, {new_position.y()})")

    def on_component_rotated(self, widget, angle):
        """Handler para rotação de componente"""
        self.status_panel.add_log_entry(f"Componente rotacionado para {angle}°")

    def toggle_grid(self):
        """Alterna a visibilidade do grid"""
        self.work_area.toggle_grid()
        self.status_panel.add_log_entry("Grid " + ("ativado" if self.grid_toggle_button.isChecked() else "desativado"))
        self.update_grid_status()

    def toggle_snap_to_grid(self):
        """Alterna o snap-to-grid"""
        self.work_area.toggle_snap_to_grid()
        self.status_panel.add_log_entry("Snap-to-grid " + ("ativado" if self.snap_toggle_button.isChecked() else "desativado"))
        self.update_grid_status()

    def set_grid_size(self, size):
        """Define o tamanho do grid"""
        self.work_area.set_grid_size(size)
        self.status_panel.add_log_entry(f"Tamanho do grid alterado para {size}px")
        self.update_grid_status()

    def update_grid_status(self):
        """Atualiza o status do grid na barra de status"""
        grid_status = f"Grid: {self.grid_size_spin.value()}px"
        if self.grid_toggle_button.isChecked():
            grid_status += " | Visível"
        else:
            grid_status += " | Oculto"

        if self.snap_toggle_button.isChecked():
            grid_status += " | Snap ativo"
        else:
            grid_status += " | Snap inativo"

        self.status_bar.showMessage(f"{grid_status} - Clique esquerdo para mover, Ctrl+clique ou clique direito para rotacionar")

    def on_add_component(self):
        comp_name = self.component_combo.currentText()

        # Obter tamanho do grid para posicionamento adequado
        grid_size = self.work_area.grid_size

        # Encontrar posição livre no grid
        occupied_positions = set()
        for comp in self.work_area.get_all_components():
            grid_x = comp['pos'].x() // grid_size
            grid_y = comp['pos'].y() // grid_size
            occupied_positions.add((grid_x, grid_y))

        # Encontrar próxima posição livre
        new_x, new_y = 2, 2
        while (new_x, new_y) in occupied_positions:
            new_x += 1
            if new_x > 15:  # Limite horizontal
                new_x = 2
                new_y += 1
                if new_y > 10:  # Limite vertical
                    new_y = 2

        new_pos = QPoint(new_x * grid_size, new_y * grid_size)

        try:
            if comp_name == "6502 CPU":
                pin_names = [
                    "VCC", "RDY", "IRQ", "NMI", "SYNC", "A0", "A1", "A2",
                    "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10",
                    "A11", "A12", "A13", "A14", "A15", "D0", "D1", "D2",
                    "D3", "D4", "D5", "D6", "D7", "R/W", "PHI2", "RES"
                ]
                chip = ChipWidget(name="6502 CPU", pin_names=pin_names)
                chip.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
                self.work_area.add_component(chip, new_pos)
                self.status_panel.add_log_entry("CPU 6502 adicionada")

            elif comp_name == "LCD 16x2":
                lcd = LCD16x2Widget()
                lcd.setMinimumSize(180, 60)
                lcd.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
                self.work_area.add_component(lcd, new_pos)
                # Guardar referência para o LCD widget
                self.lcd_widget = lcd
                self.status_panel.add_log_entry("LCD 16x2 adicionado")

            elif comp_name == "ROM":
                rom_pin_names = [
                    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
                    "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15",
                    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
                    "OE", "CE", "VCC", "GND"
                ]
                rom = ChipWidget(name="ROM", pin_names=rom_pin_names)
                rom.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
                self.work_area.add_component(rom, new_pos)
                self.status_panel.add_log_entry("ROM adicionada")

            elif comp_name == "RAM":
                ram_pin_names = [
                    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
                    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
                    "WE", "OE", "CS", "VCC", "GND"
                ]
                ram = ChipWidget(name="RAM", pin_names=ram_pin_names)
                ram.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
                self.work_area.add_component(ram, new_pos)
                self.status_panel.add_log_entry("RAM adicionada")

            else:
                # Para outros componentes, usar ChipWidget genérico
                chip = ChipWidget(name=comp_name)
                chip.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
                self.work_area.add_component(chip, new_pos)
                self.status_panel.add_log_entry(f"Componente {comp_name} adicionado")

        except Exception as e:
            self.status_panel.add_log_entry(f"Erro ao adicionar componente {comp_name}: {e}")
            QMessageBox.warning(self, "Erro", f"Erro ao adicionar componente {comp_name}: {e}")

    def add_required_components(self, components):
        """Adiciona automaticamente os componentes necessários para o exemplo"""
        try:
            # Importação local para evitar problemas circulares
            from widgets.chip_widget import ChipWidget
            from widgets.lcd_widget import LCD16x2Widget
            try:
                from widgets.control_widgets import LEDWidget, SwitchWidget
            except ImportError:
                # Fallback se control_widgets não existir
                LEDWidget = ChipWidget
                SwitchWidget = ChipWidget

            self.status_panel.add_log_entry(f"DEBUG: Adicionando componentes: {components}")

            positions = {
                '6502 CPU': QPoint(50, 50),
                'LCD 16x2': QPoint(300, 50),
                'LED': QPoint(300, 150),
                'Botão': QPoint(300, 200),
                'RAM': QPoint(150, 50),
                'ROM': QPoint(150, 150),
                'Resistor': QPoint(400, 50),
                'Capacitor': QPoint(400, 150),
            }

            for comp in components:
                pos = positions.get(comp, QPoint(100, 100))
                self.status_panel.add_log_entry(f"DEBUG: Criando componente: {comp} na posição {pos}")

                # Cria o widget correto para cada componente
                try:
                    if comp == '6502 CPU':
                        pin_names = [
                            "VCC", "RDY", "IRQ", "NMI", "SYNC", "A0", "A1", "A2",
                            "A3", "A4", "A5", "A6", "A7", "A8", "A9", "A10",
                            "A11", "A12", "A13", "A14", "A15", "D0", "D1", "D2",
                            "D3", "D4", "D5", "D6", "D7", "R/W", "PHI2", "RES"
                        ]
                        widget = ChipWidget(name=comp, pin_names=pin_names)
                        self.status_panel.add_log_entry(f"DEBUG: CPU widget criado: {widget}")
                    elif comp == 'RAM':
                        pin_names = [
                            "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
                            "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
                            "WE", "OE", "CS", "VCC", "GND"
                        ]
                        widget = ChipWidget(name=comp, pin_names=pin_names)
                        self.status_panel.add_log_entry(f"DEBUG: RAM widget criado: {widget}")
                    elif comp == 'ROM':
                        pin_names = [
                            "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
                            "A8", "A9", "A10", "A11", "A12", "A13", "A14", "A15",
                            "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
                            "OE", "CE", "VCC", "GND"
                        ]
                        widget = ChipWidget(name=comp, pin_names=pin_names)
                        self.status_panel.add_log_entry(f"DEBUG: ROM widget criado: {widget}")
                    elif comp == 'Resistor' or comp == 'Capacitor':
                        widget = ChipWidget(name=comp)
                        self.status_panel.add_log_entry(f"DEBUG: {comp} widget criado: {widget}")
                    elif comp == 'LCD 16x2':
                        widget = LCD16x2Widget()
                        self.lcd_widget = widget
                        self.status_panel.add_log_entry(f"DEBUG: LCD widget criado e definido como self.lcd_widget: {widget}")

                        # LCD começa vazio - sem texto de teste
                        widget.set_display_text("", "")
                        widget.set_display_on(False)  # Começa desligado até ser inicializado pelo programa
                        widget.update()
                        self.status_panel.add_log_entry("DEBUG: LCD inicializado vazio")

                    elif comp == 'LED':
                        widget = LEDWidget()
                        self.status_panel.add_log_entry(f"DEBUG: LED widget criado: {widget}")
                    elif comp == 'Botão':
                        widget = SwitchWidget()
                        self.status_panel.add_log_entry(f"DEBUG: Botão widget criado: {widget}")
                    else:
                        widget = ChipWidget(name=comp)
                        self.status_panel.add_log_entry(f"DEBUG: Widget genérico criado para {comp}: {widget}")

                    self.work_area.add_component(widget, pos)
                    self.status_panel.add_log_entry(f"DEBUG: Componente {comp} adicionado à work_area na posição {pos}")

                except Exception as e:
                    self.status_panel.add_log_entry(f"ERRO ao adicionar componente {comp}: {e}")
                    import traceback
                    self.status_panel.add_log_entry(f"Traceback: {traceback.format_exc()}")
                    # Fallback - adicionar como chip genérico
                    widget = ChipWidget(name=comp)
                    self.work_area.add_component(widget, pos)

        except Exception as e:
            self.status_panel.add_log_entry(f"Erro geral ao adicionar componentes: {e}")
            import traceback
            self.status_panel.add_log_entry(f"Traceback: {traceback.format_exc()}")

def main():
    """Função principal"""
    app = QApplication(sys.argv)

    # Carregar arquivo de estilos
    try:
        with open('style.qss', 'r') as f:
            app.setStyleSheet(f.read())
    except FileNotFoundError:
        print("Arquivo style.qss não encontrado")

    # Criar e mostrar janela principal
    window = Emu65MainWindow()
    window.app = app  # Adicionar referência da aplicação para cleanup
    window.show()

    # Executar aplicação
    try:
        exit_code = app.exec()
    except Exception as e:
        print(f"Erro durante execução da aplicação: {e}")
        exit_code = 1
    finally:
        # Cleanup final forçado
        try:
            if hasattr(window, 'core') and window.core:
                window.core.destroy()
        except:
            pass

    sys.exit(exit_code)

if __name__ == "__main__":
    main()