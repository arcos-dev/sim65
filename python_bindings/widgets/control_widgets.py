#!/usr/bin/env python3
"""
Control Widgets - Widgets de Controle
=====================================

Widgets especializados para controles do emulador:
LEDs, switches, displays de registradores.

Autor: Anderson Costa
Versão: 1.0.0
Data: 2025-01-06
"""

from PyQt6.QtWidgets import QWidget, QLabel
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QPainter, QColor, QFont, QPen, QBrush, QLinearGradient

class LEDWidget(QWidget):
    """Widget LED com animação de brilho"""

    def __init__(self, color=QColor(255, 0, 0), parent=None):
        super().__init__(parent)
        self.setFixedSize(20, 20)
        self.color = color
        self.state = False
        self.brightness = 0.0
        self.target_brightness = 0.0

    def set_state(self, state: bool):
        """Define o estado do LED"""
        self.state = state
        self.target_brightness = 1.0 if state else 0.0

    def paintEvent(self, event):
        """Renderiza o LED com efeito de brilho"""
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        # Suaviza transição de brilho
        if abs(self.brightness - self.target_brightness) > 0.01:
            self.brightness += (self.target_brightness - self.brightness) * 0.3
            self.update()

        # Cor do LED com brilho
        led_color = QColor(
            int(self.color.red() * self.brightness),
            int(self.color.green() * self.brightness),
            int(self.color.blue() * self.brightness)
        )

        # Gradiente para efeito 3D
        grad = QLinearGradient(2, 2, 18, 18)
        grad.setColorAt(0, led_color.lighter(150))
        grad.setColorAt(1, led_color.darker(150))

        painter.setBrush(QBrush(grad))
        painter.setPen(QPen(QColor(80, 80, 80), 1))
        painter.drawEllipse(2, 2, 16, 16)

        # Reflexo
        if self.state:
            painter.setBrush(QBrush(QColor(255, 255, 255, 100)))
            painter.setPen(Qt.PenStyle.NoPen)
            painter.drawEllipse(5, 5, 6, 6)

class SwitchWidget(QWidget):
    """Widget switch com animação"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setFixedSize(30, 50)
        self.state = False
        self.animation_progress = 0.0
        self.target_progress = 0.0

    def set_state(self, state: bool):
        """Define o estado do switch"""
        self.state = state
        self.target_progress = 1.0 if state else 0.0

    def paintEvent(self, event):
        """Renderiza o switch com animação"""
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        # Suaviza animação
        if abs(self.animation_progress - self.target_progress) > 0.01:
            self.animation_progress += (self.target_progress - self.animation_progress) * 0.3
            self.update()

        # Base do switch
        painter.setBrush(QBrush(QColor(60, 60, 60)))
        painter.setPen(QPen(QColor(40, 40, 40), 2))
        painter.drawRoundedRect(5, 10, 20, 30, 10, 10)

        # Posição do botão
        button_y = 12 + int(self.animation_progress * 26)
        button_color = QColor(0, 255, 0) if self.state else QColor(255, 0, 0)

        # Botão com gradiente
        grad = QLinearGradient(8, button_y, 22, button_y + 8)
        grad.setColorAt(0, button_color.lighter(150))
        grad.setColorAt(1, button_color.darker(150))

        painter.setBrush(QBrush(grad))
        painter.setPen(QPen(QColor(40, 40, 40), 1))
        painter.drawRoundedRect(8, button_y, 14, 8, 4, 4)

class RegisterDisplay(QWidget):
    """Display de registrador com valor hexadecimal"""

    def __init__(self, name: str, parent=None):
        super().__init__(parent)
        self.name = name
        self.value = 0x00
        self.setFixedSize(80, 40)

    def set_value(self, value: int):
        """Define o valor do registrador"""
        self.value = value & 0xFF
        self.update()

    def paintEvent(self, event):
        """Renderiza o display do registrador"""
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        # Fundo
        painter.setBrush(QBrush(QColor(20, 20, 20)))
        painter.setPen(QPen(QColor(100, 100, 100), 2))
        painter.drawRoundedRect(2, 2, self.width()-4, self.height()-4, 8, 8)

        # Nome do registrador
        painter.setFont(QFont("Segoe UI Variable", 8, QFont.Weight.Bold))
        painter.setPen(QPen(QColor(200, 200, 200)))
        painter.drawText(5, 15, self.name)

        # Valor hexadecimal
        painter.setFont(QFont("Courier", 12, QFont.Weight.Bold))
        painter.setPen(QPen(QColor(0, 255, 0)))
        painter.drawText(5, 32, f"0x{self.value:02X}")

class StatusLabel(QLabel):
    """Label de status com estilo verde"""

    def __init__(self, text: str = "", parent=None):
        super().__init__(text, parent)
        self.setProperty("class", "green-label")
        self.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.setMinimumHeight(20)