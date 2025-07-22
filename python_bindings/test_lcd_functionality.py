#!/usr/bin/env python3
"""
Teste de Funcionalidade do LCD - EMU65
======================================

Agora carrega exemplos binários externos gerados pelo VASM.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-07-16
"""

import sys
import os
import time
from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QLabel, QTextEdit, QComboBox, QGroupBox, QFrame, QTabWidget
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtGui import QFont

# Importar módulos locais
try:
    from emu65_core import Emu65Core
    from widgets.lcd_widget import LCD16x2Widget
    from simple_examples import SimpleExamples
except ImportError:
    print("Erro: Não foi possível importar os módulos necessários")
    sys.exit(1)

class LCDTestWidget(QWidget):
    """Widget de teste para o LCD"""
    
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Teste de Funcionalidade do LCD - EMU65")
        self.setMinimumSize(800, 500)
        self.resize(1000, 600)
        
        # Inicializar componentes
        self.core = None
        self.lcd_widget = LCD16x2Widget()
        # LCD com tamanho mais apropriado para 16x2
        self.lcd_widget.setMinimumSize(320, 120)
        self.lcd_widget.setMaximumSize(400, 150)
        
        # Lista de exemplos
        self.examples = SimpleExamples.get_all_examples()
        self.selected_example = self.examples[0]
        
        # Interface de teste
        self.init_ui()
        
        # Timer para atualização
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.update_lcd)
        self.update_timer.start(100)  # 100ms
    
    def init_ui(self):
        """Inicializa a interface de teste"""
        layout = QVBoxLayout(self)
        layout.setSpacing(10)
        layout.setContentsMargins(15, 15, 15, 15)
        
        # Seleção de exemplo - Combo box maior
        example_frame = QFrame()
        example_frame.setObjectName("example-frame")
        example_layout = QHBoxLayout(example_frame)
        example_layout.setContentsMargins(15, 10, 15, 10)
        
        example_label = QLabel("Exemplo:")
        example_label.setFont(QFont("Segoe UI", 12, QFont.Weight.Medium))
        
        self.example_combo = QComboBox()
        self.example_combo.setFont(QFont("Segoe UI", 11))
        self.example_combo.setMinimumHeight(35)
        self.example_combo.setMinimumWidth(300)
        
        for ex in self.examples:
            self.example_combo.addItem(ex['name'])
        self.example_combo.currentIndexChanged.connect(self.on_example_changed)
        
        example_layout.addWidget(example_label)
        example_layout.addWidget(self.example_combo)
        example_layout.addStretch()
        layout.addWidget(example_frame)
        
        # Botões de controle - Menores e mais elegantes
        btn_frame = QFrame()
        btn_frame.setObjectName("button-frame")
        btn_layout = QHBoxLayout(btn_frame)
        btn_layout.setContentsMargins(15, 10, 15, 10)
        btn_layout.setSpacing(10)
        
        btn_font = QFont("Segoe UI", 10, QFont.Weight.Medium)
        
        self.btn_init_core = QPushButton("Inicializar Core")
        self.btn_init_core.setFont(btn_font)
        self.btn_init_core.setMinimumHeight(35)
        self.btn_init_core.clicked.connect(self.init_core)
        btn_layout.addWidget(self.btn_init_core)
        
        self.btn_test_example = QPushButton("Carregar Exemplo")
        self.btn_test_example.setFont(btn_font)
        self.btn_test_example.setMinimumHeight(35)
        self.btn_test_example.clicked.connect(self.test_selected_example)
        self.btn_test_example.setEnabled(False)
        btn_layout.addWidget(self.btn_test_example)
        
        self.btn_step = QPushButton("Step")
        self.btn_step.setFont(btn_font)
        self.btn_step.setMinimumHeight(35)
        self.btn_step.clicked.connect(self.step_execution)
        self.btn_step.setEnabled(False)
        btn_layout.addWidget(self.btn_step)
        
        self.btn_run = QPushButton("Run")
        self.btn_run.setFont(btn_font)
        self.btn_run.setMinimumHeight(35)
        self.btn_run.clicked.connect(self.run_execution)
        self.btn_run.setEnabled(False)
        btn_layout.addWidget(self.btn_run)
        
        self.btn_stop = QPushButton("Stop")
        self.btn_stop.setFont(btn_font)
        self.btn_stop.setMinimumHeight(35)
        self.btn_stop.clicked.connect(self.stop_execution)
        self.btn_stop.setEnabled(False)
        btn_layout.addWidget(self.btn_stop)
        
        self.btn_reset = QPushButton("Reset")
        self.btn_reset.setFont(btn_font)
        self.btn_reset.setMinimumHeight(35)
        self.btn_reset.clicked.connect(self.reset_execution)
        self.btn_reset.setEnabled(False)
        btn_layout.addWidget(self.btn_reset)
        
        layout.addWidget(btn_frame)
        
        # Área principal: LCD à esquerda, abas à direita
        main_layout = QHBoxLayout()
        main_layout.setSpacing(15)
        
        # Área do LCD (esquerda)
        lcd_frame = QFrame()
        lcd_frame.setObjectName("lcd-frame")
        lcd_layout = QVBoxLayout(lcd_frame)
        lcd_layout.setContentsMargins(15, 15, 15, 15)
        
        lcd_title = QLabel("LCD 16x2")
        lcd_title.setFont(QFont("Segoe UI", 12, QFont.Weight.Medium))
        lcd_title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        lcd_layout.addWidget(lcd_title)
        
        lcd_layout.addWidget(self.lcd_widget, alignment=Qt.AlignmentFlag.AlignCenter)
        lcd_layout.addStretch()
        
        main_layout.addWidget(lcd_frame)
        
        # Painel de abas (direita) - Log e Debug
        tab_widget = QTabWidget()
        tab_widget.setMinimumWidth(400)
        
        # Aba 1: Log de Execução
        log_tab = QWidget()
        log_layout = QVBoxLayout(log_tab)
        log_layout.setContentsMargins(10, 10, 10, 10)
        
        log_label = QLabel("Log de Execução:")
        log_label.setFont(QFont("Segoe UI", 11, QFont.Weight.Medium))
        log_layout.addWidget(log_label)
        
        self.log_text = QTextEdit()
        self.log_text.setFont(QFont("Consolas", 9))
        log_layout.addWidget(self.log_text)
        
        tab_widget.addTab(log_tab, "Log de Execução")
        
        # Aba 2: Depuração VIA/LCD
        debug_tab = QWidget()
        debug_layout = QVBoxLayout(debug_tab)
        debug_layout.setContentsMargins(10, 10, 10, 10)
        debug_layout.setSpacing(8)
        
        # Layout horizontal para VIA e LCD lado a lado
        debug_columns_layout = QHBoxLayout()
        debug_columns_layout.setSpacing(15)
        
        # Coluna esquerda - Registradores VIA
        via_column = QVBoxLayout()
        via_column.setSpacing(5)
        
        via_label = QLabel("Registradores VIA 6522:")
        via_label.setFont(QFont("Segoe UI", 10, QFont.Weight.Medium))
        via_column.addWidget(via_label)
        
        self.via_text = QTextEdit()
        self.via_text.setFont(QFont("Consolas", 9))
        self.via_text.setMinimumHeight(120)
        self.via_text.setMaximumHeight(150)
        self.via_text.setReadOnly(True)
        via_column.addWidget(self.via_text)
        
        debug_columns_layout.addLayout(via_column)
        
        # Coluna direita - LCD bytes brutos
        lcd_column = QVBoxLayout()
        lcd_column.setSpacing(5)
        
        lcd_raw_label = QLabel("LCD (bytes brutos):")
        lcd_raw_label.setFont(QFont("Segoe UI", 10, QFont.Weight.Medium))
        lcd_column.addWidget(lcd_raw_label)
        
        self.lcd_raw_text = QTextEdit()
        self.lcd_raw_text.setFont(QFont("Consolas", 9))
        self.lcd_raw_text.setMinimumHeight(120)
        self.lcd_raw_text.setMaximumHeight(150)
        self.lcd_raw_text.setReadOnly(True)
        lcd_column.addWidget(self.lcd_raw_text)
        
        debug_columns_layout.addLayout(lcd_column)
        
        debug_layout.addLayout(debug_columns_layout)
        
        # Botões dump
        dump_layout = QHBoxLayout()
        
        self.btn_dump_via = QPushButton("Dump VIA")
        self.btn_dump_via.setFont(QFont("Segoe UI", 9, QFont.Weight.Medium))
        self.btn_dump_via.setProperty("class", "compact-button")
        self.btn_dump_via.clicked.connect(self.dump_via)
        dump_layout.addWidget(self.btn_dump_via)
        
        self.btn_dump_lcd = QPushButton("Dump LCD")
        self.btn_dump_lcd.setFont(QFont("Segoe UI", 9, QFont.Weight.Medium))
        self.btn_dump_lcd.setProperty("class", "compact-button")
        self.btn_dump_lcd.clicked.connect(self.dump_lcd)
        dump_layout.addWidget(self.btn_dump_lcd)
        
        debug_layout.addLayout(dump_layout)
        tab_widget.addTab(debug_tab, "Depuração VIA/LCD")
        
        main_layout.addWidget(tab_widget)
        layout.addLayout(main_layout)
        
        # Status - Mais elegante
        self.status_label = QLabel("Pronto para testes")
        self.status_label.setFont(QFont("Segoe UI", 10, QFont.Weight.Medium))
        self.status_label.setObjectName("status-label")
        layout.addWidget(self.status_label)
        
        # Estado de execução
        self.is_running = False
        self.step_count = 0
    
    def log(self, message):
        self.log_text.append(f"[{time.strftime('%H:%M:%S')}] {message}")
        self.log_text.ensureCursorVisible()
        
    def on_example_changed(self, idx):
        self.selected_example = self.examples[idx]
        self.log(f"Exemplo selecionado: {self.selected_example['name']}")
        self.btn_test_example.setEnabled(self.core is not None)
        self.btn_step.setEnabled(False)
        self.btn_run.setEnabled(False)
        self.btn_reset.setEnabled(False)
        self.status_label.setText("Exemplo alterado - Carregue para testar")
        self.step_count = 0
        
    def init_core(self):
        try:
            self.core = Emu65Core({
                'clock_frequency': 1000000.0,
                'debug_mode': False,
                'trace_execution': False
            })
            self.log("Core inicializado com sucesso")
            self.status_label.setText("Core inicializado")
            self.btn_test_example.setEnabled(True)
            self.btn_init_core.setEnabled(False)
        except Exception as e:
            self.log(f"Erro ao inicializar core: {e}")
            self.status_label.setText(f"Erro: {e}")
    
    def test_selected_example(self):
        try:
            if not self.core:
                self.log("Core não inicializado")
                return
            example = self.selected_example
            if not os.path.exists(example['bin_path']):
                self.log(f"Arquivo binário não encontrado: {example['bin_path']}")
                self.status_label.setText("Arquivo binário não encontrado!")
                return
            with open(example['bin_path'], 'rb') as f:
                code = f.read()
            self.core.reset()
            self.core.load_program(code, example['start_address'])
            self.core.reset()
            self.log(f"Exemplo carregado: {example['name']}")
            self.log(f"Descrição: {example['description']}")
            self.log(f"Endereço inicial: 0x{example['start_address']:04X}")
            self.btn_step.setEnabled(True)
            self.btn_run.setEnabled(True)
            self.btn_reset.setEnabled(True)
            self.btn_test_example.setEnabled(False)
            self.step_count = 0
            self.status_label.setText("Exemplo carregado - Use Step/Run para executar")
        except Exception as e:
            self.log(f"Erro ao carregar exemplo: {e}")
            self.status_label.setText(f"Erro: {e}")
    
    def step_execution(self):
        try:
            if not self.core:
                return
            result = self.core.step()
            self.step_count += 1
            bus_state = self.core.get_bus_state()
            if 0x6000 <= bus_state.address <= 0x6003:
                self.log(f"Step {self.step_count}: I/O addr=0x{bus_state.address:04X} data=0x{bus_state.data:02X} rw={'R' if bus_state.rw else 'W'}")
            self.status_label.setText(f"Step {self.step_count} - PC: 0x{bus_state.address:04X}")
        except Exception as e:
            self.log(f"Erro no step: {e}")
    
    def run_execution(self):
        if not self.core:
            return
        self.is_running = True
        self.btn_run.setEnabled(False)
        self.btn_stop.setEnabled(True)
        self.btn_step.setEnabled(False)
        self.run_timer = QTimer()
        self.run_timer.timeout.connect(self.run_step)
        self.run_timer.start(50)
        self.log("Execução iniciada")
    
    def run_step(self):
        try:
            if not self.core or not self.is_running:
                self.run_timer.stop()
                return
            result = self.core.step()
            self.step_count += 1
            bus_state = self.core.get_bus_state()
            if 0x6000 <= bus_state.address <= 0x6003:
                self.log(f"Run {self.step_count}: I/O addr=0x{bus_state.address:04X} data=0x{bus_state.data:02X} rw={'R' if bus_state.rw else 'W'}")
            self.status_label.setText(f"Run {self.step_count} - PC: 0x{bus_state.address:04X}")
        except Exception as e:
            self.log(f"Erro no run: {e}")
            self.stop_execution()
    
    def stop_execution(self):
        self.is_running = False
        if hasattr(self, 'run_timer'):
            self.run_timer.stop()
        self.btn_run.setEnabled(True)
        self.btn_stop.setEnabled(False)
        self.btn_step.setEnabled(True)
        self.log("Execução parada")
    
    def reset_execution(self):
        try:
            if self.core:
                self.core.reset()
                self.step_count = 0
                self.log("Execução resetada")
                self.status_label.setText("Resetado")
                if self.is_running:
                    self.stop_execution()
        except Exception as e:
            self.log(f"Erro no reset: {e}")
    
    def update_lcd(self):
        try:
            if not self.core:
                return
            lcd_state = self.core.get_lcd_state()
            
            # Converte o array de bytes para string
            display_bytes = bytes(lcd_state.display)
            display_text = display_bytes.decode('ascii', errors='ignore')
            
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
            
            # Atualizar widget LCD
            self.lcd_widget.set_display_text(row1, row2)
            self.lcd_widget.set_cursor(lcd_state.cursor_row, lcd_state.cursor_col)
            self.lcd_widget.set_display_on(lcd_state.display_on)
            self.lcd_widget.set_cursor_visible(lcd_state.cursor_on)
            self.lcd_widget.set_blink_on(lcd_state.blink_on)
            
            # Log de debug (apenas se houver mudanças)
            if row1.strip() or row2.strip():
                self.log(f"LCD: '{row1}' | '{row2}' (ON:{lcd_state.display_on}, CURSOR:{lcd_state.cursor_on}, BLINK:{lcd_state.blink_on})")
        except Exception as e:
            self.log(f"Erro ao atualizar LCD: {e}")
            import traceback
            traceback.print_exc()

    def dump_via(self):
        if not self.core:
            self.via_text.setPlainText("Core não inicializado")
            return
        state = self.core.get_via_state()
        if 'error' in state:
            self.via_text.setPlainText(f"Erro: {state['error']}")
            return
        lines = []
        for reg in ["ORB","ORA","DDRB","DDRA","T1CL","T1CH","T1LL","T1LH","T2CL","T2CH","SR","ACR","PCR","IFR","IER","ORANH"]:
            val = state.get(reg, 0)
            lines.append(f"{reg}: 0x{val:02X}")
        self.via_text.setPlainText("\n".join(lines))
    
    def dump_lcd(self):
        if not self.core:
            self.lcd_raw_text.setPlainText("Core não inicializado")
            return
        lcd_state = self.core.get_lcd_state()
        display_bytes = bytes(lcd_state.display)
        hex_str = ' '.join(f'{b:02X}' for b in display_bytes)
        self.lcd_raw_text.setPlainText(hex_str)

def main():
    app = QApplication(sys.argv)
    
    # Carregar arquivo de estilo
    try:
        style_file = os.path.join(os.path.dirname(__file__), 'style.qss')
        with open(style_file, 'r', encoding='utf-8') as f:
            app.setStyleSheet(f.read())
    except Exception as e:
        print(f"Erro ao carregar style.qss: {e}")
    
    test_widget = LCDTestWidget()
    test_widget.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main() 