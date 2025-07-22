#!/usr/bin/env python3
"""
Memory Panel - Painel de Visualização de Memória
================================================

Widget para visualizar o conteúdo da memória RAM/ROM
com dump hexadecimal, endereços e valores.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-01-06
"""

from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QSpinBox, QComboBox, QTableWidget, QTableWidgetItem, QHeaderView, QFrame, QGridLayout, QTabWidget, QSizePolicy, QGroupBox
)
from PyQt6.QtCore import Qt, QTimer, pyqtSignal
from PyQt6.QtGui import QFont

class MemoryPanel(QWidget):
    """Painel de visualização de memória com abas organizadas"""

    memory_updated = pyqtSignal(int, int, int)  # endereço, valor, tipo_acesso

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("memory-viewer")
        self.setMinimumSize(300, 300)  # Reduzido de 400 para 300 pixels
        self.setProperty("class", "control-panel")  # padronizar cor de fundo
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

        # Dados de memória
        self.memory_data = bytearray(65536)  # 64KB de memória
        self.current_address = 0x0000
        self.bytes_per_line = 16
        self.lines_visible = 20
        self.highlighted_address = None
        self.last_access_address = None
        self.last_access_type = None  # 'read', 'write'

        self.init_ui()
        self.load_sample_data()

    def init_ui(self):
        """Inicializa a interface do usuário com abas"""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(16)

        # Título principal
        title_label = QLabel("Visualizador de Memória")
        title_label.setProperty("class", "green-label-large")
        title_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title_label.setFont(QFont("Segoe UI Variable", 14, QFont.Weight.Bold))
        title_label.setMinimumHeight(40)
        layout.addWidget(title_label)

        # QTabWidget principal
        self.tab_widget = QTabWidget()
        self.tab_widget.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

        # Criar abas
        self.create_controls_tab()
        self.create_viewer_tab()
        self.create_data_tab()

        layout.addWidget(self.tab_widget)

    def create_controls_tab(self):
        """Cria a aba de controles"""
        controls_widget = QWidget()
        controls_layout = QVBoxLayout(controls_widget)
        controls_layout.setContentsMargins(16, 16, 16, 16)
        controls_layout.setSpacing(20)



        # Grupo de controles
        controls_group = QGroupBox("Navegação")
        controls_group.setProperty("class", "control-group")
        controls_group_layout = QVBoxLayout(controls_group)
        controls_group_layout.setSpacing(16)
        controls_group_layout.setContentsMargins(16, 20, 16, 16)

        # Grid de controles
        controls_grid = QGridLayout()
        controls_grid.setSpacing(16)
        controls_grid.setColumnStretch(1, 1)  # Segunda coluna expande

        # Linha 1: Região
        region_label = QLabel("Região:")
        region_label.setProperty("class", "green-label-small")
        region_label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        controls_grid.addWidget(region_label, 0, 0)

        self.region_combo = QComboBox()
        self.region_combo.addItems([
            "RAM (0x0000-0x00FF)",
            "Stack (0x0100-0x01FF)",
            "RAM (0x0200-0x7FFF)",
            "LCD (0x6000-0x600F)",
            "ROM (0x8000-0xFFFF)"
        ])
        self.region_combo.setProperty("class", "green-combo")
        self.region_combo.currentIndexChanged.connect(self.on_region_changed)
        controls_grid.addWidget(self.region_combo, 0, 1)

        # Linha 2: Endereço
        address_label = QLabel("Endereço:")
        address_label.setProperty("class", "green-label-small")
        address_label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        controls_grid.addWidget(address_label, 1, 0)

        self.address_spin = QSpinBox()
        self.address_spin.setRange(0, 65535)
        self.address_spin.setPrefix("0x")
        self.address_spin.setDisplayIntegerBase(16)
        self.address_spin.setValue(0x0000)
        self.address_spin.setProperty("class", "green-spin")
        self.address_spin.valueChanged.connect(self.on_address_changed)
        controls_grid.addWidget(self.address_spin, 1, 1)

        controls_group_layout.addLayout(controls_grid)
        controls_layout.addWidget(controls_group)

        # Grupo de botões
        buttons_group = QGroupBox("Ações")
        buttons_group.setProperty("class", "control-group")
        buttons_group_layout = QVBoxLayout(buttons_group)
        buttons_group_layout.setSpacing(16)
        buttons_group_layout.setContentsMargins(16, 20, 16, 16)

        # Botões em grid
        buttons_grid = QGridLayout()
        buttons_grid.setSpacing(12)

        # Botão de atualização
        self.refresh_btn = QPushButton("🔄 Atualizar")
        self.refresh_btn.setToolTip("Atualizar visualização")
        self.refresh_btn.setProperty("class", "green-button")
        self.refresh_btn.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.refresh_btn.setMinimumHeight(35)
        self.refresh_btn.clicked.connect(self.refresh_display)
        buttons_grid.addWidget(self.refresh_btn, 0, 0)

        # Botão de anterior
        self.prev_btn = QPushButton("◀ Anterior")
        self.prev_btn.setToolTip("Página anterior")
        self.prev_btn.setProperty("class", "green-button")
        self.prev_btn.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.prev_btn.setMinimumHeight(35)
        self.prev_btn.clicked.connect(self.prev_page)
        buttons_grid.addWidget(self.prev_btn, 0, 1)

        # Botão de próximo
        self.next_btn = QPushButton("▶ Próximo")
        self.next_btn.setToolTip("Próxima página")
        self.next_btn.setProperty("class", "green-button")
        self.next_btn.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.next_btn.setMinimumHeight(35)
        self.next_btn.clicked.connect(self.next_page)
        buttons_grid.addWidget(self.next_btn, 1, 0)

        # Botão de busca
        self.search_btn = QPushButton("🔍 Buscar")
        self.search_btn.setToolTip("Buscar valor na memória")
        self.search_btn.setProperty("class", "green-button")
        self.search_btn.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        self.search_btn.setMinimumHeight(35)
        buttons_grid.addWidget(self.search_btn, 1, 1)

        buttons_group_layout.addLayout(buttons_grid)
        controls_layout.addWidget(buttons_group)

        # Espaçador
        controls_layout.addStretch()

        self.tab_widget.addTab(controls_widget, "Controles")

    def create_viewer_tab(self):
        """Cria a aba de visualização da memória"""
        viewer_widget = QWidget()
        viewer_layout = QVBoxLayout(viewer_widget)
        viewer_layout.setContentsMargins(8, 8, 8, 8)
        viewer_layout.setSpacing(8)

        # Tabela de memória (sem painéis internos para maximizar espaço)
        self.memory_table = QTableWidget()
        self.memory_table.setRowCount(self.lines_visible)
        self.memory_table.setColumnCount(18)  # Endereço + 16 bytes + ASCII
        self.memory_table.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

        # Cabeçalhos
        headers = ["Addr"]
        for i in range(16):
            headers.append(f"{i:02X}")
        headers.append("ASCII")

        self.memory_table.setHorizontalHeaderLabels(headers)

        # Configuração da tabela
        self.memory_table.setAlternatingRowColors(True)
        self.memory_table.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)
        self.memory_table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)

        # Larguras das colunas
        header = self.memory_table.horizontalHeader()
        if header:
            header.setSectionResizeMode(0, QHeaderView.ResizeMode.Fixed)  # Endereço
            header.setSectionResizeMode(17, QHeaderView.ResizeMode.Fixed)  # ASCII
            self.memory_table.setColumnWidth(0, 60)  # Endereço
            self.memory_table.setColumnWidth(17, 120)  # ASCII

            # Larguras das colunas de bytes
            for i in range(1, 17):
                header.setSectionResizeMode(i, QHeaderView.ResizeMode.Fixed)
                self.memory_table.setColumnWidth(i, 30)

        # Altura das linhas
        vheader = self.memory_table.verticalHeader()
        if vheader:
            vheader.setDefaultSectionSize(20)
            vheader.setVisible(False)

        viewer_layout.addWidget(self.memory_table)

        # Atualizar display inicial
        self.refresh_display()

        self.tab_widget.addTab(viewer_widget, "Visualização")

    def create_data_tab(self):
        """Cria a aba de dados e estatísticas"""
        data_widget = QWidget()
        data_layout = QVBoxLayout(data_widget)
        data_layout.setContentsMargins(16, 16, 16, 16)
        data_layout.setSpacing(20)



        # Grupo de estatísticas
        stats_group = QGroupBox("Estatísticas da Memória")
        stats_group.setProperty("class", "control-group")
        stats_group_layout = QVBoxLayout(stats_group)
        stats_group_layout.setSpacing(16)
        stats_group_layout.setContentsMargins(16, 20, 16, 16)

        # Grid de estatísticas
        stats_grid = QGridLayout()
        stats_grid.setSpacing(16)

        # Labels de estatísticas
        self.stats_labels = {}
        stats_items = [
            ("Total de Bytes:", "65536"),
            ("Bytes Preenchidos:", "0"),
            ("Bytes Vazios:", "65536"),
            ("Último Acesso:", "Nenhum"),
            ("Região Ativa:", "RAM (0x0000-0x00FF)")
        ]

        for i, (label_text, value_text) in enumerate(stats_items):
            label = QLabel(label_text)
            label.setProperty("class", "green-label-small")
            label.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
            stats_grid.addWidget(label, i, 0)

            value_label = QLabel(value_text)
            value_label.setProperty("class", "green-label")
            value_label.setFont(QFont("Consolas", 10, QFont.Weight.Bold))
            stats_grid.addWidget(value_label, i, 1)
            self.stats_labels[label_text] = value_label

        stats_group_layout.addLayout(stats_grid)
        data_layout.addWidget(stats_group)

        # Espaçador
        data_layout.addStretch()

        self.tab_widget.addTab(data_widget, "Dados")

    def load_sample_data(self):
        """Carrega dados de exemplo na memória"""
        # Dados de exemplo
        sample_data = [
            (0x0000, [0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07]),
            (0x6000, [0x48, 0x45, 0x4C, 0x4C, 0x4F, 0x20, 0x57, 0x4F]),  # "HELLO WO"
            (0x6008, [0x52, 0x4C, 0x44, 0x21, 0x00, 0x00, 0x00, 0x00]),  # "RLD!"
            (0x8000, [0xA9, 0x00, 0x85, 0x00, 0xA9, 0x01, 0x85, 0x01]),  # Código 6502
        ]

        for addr, data in sample_data:
            for i, byte in enumerate(data):
                self.memory_data[addr + i] = byte

    def on_region_changed(self, index):
        """Handler para mudança de região"""
        regions = [0x0000, 0x0100, 0x0200, 0x6000, 0x8000]
        if index < len(regions):
            self.current_address = regions[index]
            self.address_spin.setValue(self.current_address)
            self.refresh_display()

    def on_address_changed(self, address):
        """Handler para mudança de endereço"""
        self.current_address = address
        self.refresh_display()

    def refresh_display(self):
        """Atualiza o display da memória"""
        for row in range(self.lines_visible):
            addr = self.current_address + (row * self.bytes_per_line)

            # Endereço
            addr_item = QTableWidgetItem(f"{addr:04X}")
            addr_item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
            self.memory_table.setItem(row, 0, addr_item)

            # Bytes
            ascii_chars = ""
            for col in range(16):
                byte_addr = addr + col
                if byte_addr < len(self.memory_data):
                    byte_val = self.memory_data[byte_addr]
                    byte_item = QTableWidgetItem(f"{byte_val:02X}")
                    byte_item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)

                    # Destaque para último acesso
                    if byte_addr == self.last_access_address:
                        byte_item.setBackground(Qt.GlobalColor.yellow)
                        byte_item.setForeground(Qt.GlobalColor.black)

                    self.memory_table.setItem(row, col + 1, byte_item)

                    # Caractere ASCII
                    if 32 <= byte_val <= 126:
                        ascii_chars += chr(byte_val)
                    else:
                        ascii_chars += "."
                else:
                    byte_item = QTableWidgetItem("--")
                    byte_item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
                    self.memory_table.setItem(row, col + 1, byte_item)
                    ascii_chars += " "

            # Coluna ASCII
            ascii_item = QTableWidgetItem(ascii_chars)
            self.memory_table.setItem(row, 17, ascii_item)

    def next_page(self):
        """Vai para a próxima página"""
        self.current_address += self.bytes_per_line * self.lines_visible
        if self.current_address >= 65536:
            self.current_address = 0
        self.address_spin.setValue(self.current_address)
        self.refresh_display()

    def prev_page(self):
        """Vai para a página anterior"""
        self.current_address -= self.bytes_per_line * self.lines_visible
        if self.current_address < 0:
            self.current_address = 65536 - (self.bytes_per_line * self.lines_visible)
        self.address_spin.setValue(self.current_address)
        self.refresh_display()

    def highlight_access(self, address: int, access_type: str = "read"):
        """Destaca um acesso à memória"""
        self.last_access_address = address
        self.last_access_type = access_type
        self.refresh_display()

    def set_memory_byte(self, address: int, value: int):
        """Define um byte na memória"""
        if 0 <= address < len(self.memory_data):
            self.memory_data[address] = value & 0xFF
            self.highlight_access(address, "write")

    def get_memory_byte(self, address: int) -> int:
        """Obtém um byte da memória"""
        if 0 <= address < len(self.memory_data):
            self.highlight_access(address, "read")
            return self.memory_data[address]
        return 0

    def update_from_core(self, core):
        """Atualiza a memória a partir do core"""
        # Aqui você pode implementar a sincronização com o core real
        pass