#!/usr/bin/env python3
"""
LCD Widget - Widget LCD 16x2 Realista
=====================================

Widget que simula um display LCD 16x2 real com cursor piscante,
cores realistas e efeitos visuais.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-01-06
"""

from PyQt6.QtWidgets import QFrame
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtGui import QPainter, QColor, QFont, QPen, QBrush, QLinearGradient

class LCD16x2Widget(QFrame):
    """Widget LCD 16x2 realista com animação de cursor"""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("lcd-widget")
        self.setMinimumSize(400, 120)
        self.setMaximumSize(500, 150)

        # Estado do display
        self.display_text = ["                ", "                "]
        self.cursor_row = 0
        self.cursor_col = 0
        self.cursor_visible = True
        self.display_on = True
        self.blink_on = False

        # Cores realistas do LCD
        self.colors = {
            'on': {
                'bg': QColor(15, 35, 15),      # Verde escuro (fundo ligado)
                'text': QColor(0, 255, 0),     # Verde brilhante (texto)
                'frame': QColor(80, 80, 80),   # Cinza escuro (moldura)
                'glass': QColor(20, 45, 20)    # Verde médio (vidro)
            },
            'off': {
                'bg': QColor(25, 25, 25),      # Cinza muito escuro (fundo desligado)
                'text': QColor(40, 40, 40),    # Cinza escuro (texto invisível)
                'frame': QColor(60, 60, 60),   # Cinza escuro (moldura)
                'glass': QColor(30, 30, 30)    # Cinza escuro (vidro)
            }
        }

        # Fonte LCD realista
        self.lcd_font = QFont("Courier", 14, QFont.Weight.Bold)
        self.lcd_font.setFixedPitch(True)
        self.lcd_font.setLetterSpacing(QFont.SpacingType.PercentageSpacing, 110)

        # Animação do cursor
        self.cursor_blink = True
        self.blink_state = True
        self.blink_timer = QTimer()
        self.blink_timer.timeout.connect(self.toggle_blink)
        self.blink_timer.start(500)

        # Efeitos visuais
        self.backlight_intensity = 1.0  # 0.0 = desligado, 1.0 = ligado

    def set_display_text(self, row1: str, row2: str):
        """Define o texto das duas linhas do display"""
        self.display_text[0] = row1.ljust(16)[:16]
        self.display_text[1] = row2.ljust(16)[:16]
        self.update()

    def set_cursor(self, row: int, col: int):
        """Define a posição do cursor"""
        self.cursor_row = max(0, min(1, row))
        self.cursor_col = max(0, min(15, col))
        self.update()

    def set_display_on(self, on: bool):
        """Liga/desliga o display"""
        self.display_on = on
        self.backlight_intensity = 1.0 if on else 0.0
        self.update()

    def set_cursor_visible(self, visible: bool):
        """Mostra/esconde o cursor"""
        self.cursor_visible = visible
        self.update()

    def set_blink_on(self, blink: bool):
        """Ativa/desativa o piscamento do cursor"""
        self.blink_on = blink
        self.update()

    def toggle_blink(self):
        """Alterna o estado de piscada do cursor"""
        if self.blink_on:
            self.blink_state = not self.blink_state
            self.update()

    def paintEvent(self, event):
        """Renderiza o widget LCD com aparência realista"""
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        # Seleciona cores baseado no estado do display
        colors = self.colors['on'] if self.display_on else self.colors['off']

        # Moldura externa (plástico do LCD)
        painter.setPen(QPen(colors['frame'], 3))
        painter.setBrush(QBrush(colors['frame']))
        painter.drawRoundedRect(self.rect().adjusted(2, 2, -2, -2), 8, 8)

        # Área do display (vidro)
        display_rect = self.rect().adjusted(10, 10, -10, -10)
        
        # Gradiente para simular vidro
        gradient = QLinearGradient(display_rect.topLeft().toPointF(), display_rect.bottomRight().toPointF())
        if self.display_on:
            gradient.setColorAt(0, colors['glass'])
            gradient.setColorAt(1, colors['bg'])
        else:
            gradient.setColorAt(0, colors['glass'])
            gradient.setColorAt(1, colors['bg'])
        
        painter.setBrush(QBrush(gradient))
        painter.setPen(QPen(colors['frame'], 1))
        painter.drawRect(display_rect)

        # Área de texto (com padding interno)
        text_rect = display_rect.adjusted(8, 8, -8, -8)

        # Texto do display
        if self.display_on:
            # Ajusta intensidade do texto baseado no backlight
            text_color = QColor(
                int(colors['text'].red() * self.backlight_intensity),
                int(colors['text'].green() * self.backlight_intensity),
                int(colors['text'].blue() * self.backlight_intensity)
            )
            painter.setPen(QPen(text_color))
            painter.setFont(self.lcd_font)

            # Linha 1
            y1 = text_rect.top() + 25
            painter.drawText(text_rect.left() + 5, y1, self.display_text[0])

            # Linha 2
            y2 = text_rect.top() + 55
            painter.drawText(text_rect.left() + 5, y2, self.display_text[1])

            # Cursor (mais realista)
            if self.cursor_visible and (not self.blink_on or self.blink_state):
                cursor_y = y1 if self.cursor_row == 0 else y2
                cursor_x = text_rect.left() + 5 + self.cursor_col * 20
                
                # Cursor como bloco sólido (mais realista)
                painter.setBrush(QBrush(text_color))
                painter.setPen(QPen(text_color, 1))
                painter.drawRect(cursor_x - 1, cursor_y - 15, 18, 20)

        else:
            # Display desligado - mostra apenas silhueta do texto
            painter.setPen(QPen(colors['text']))
            painter.setFont(self.lcd_font)
            
            # Linha 1 (muito sutil)
            y1 = text_rect.top() + 25
            painter.drawText(text_rect.left() + 5, y1, self.display_text[0])
            
            # Linha 2 (muito sutil)
            y2 = text_rect.top() + 55
            painter.drawText(text_rect.left() + 5, y2, self.display_text[1])

        # Efeito de reflexo (sutil)
        if self.display_on:
            reflection = QLinearGradient(display_rect.topLeft().toPointF(), display_rect.topRight().toPointF())
            reflection.setColorAt(0, QColor(255, 255, 255, 10))
            reflection.setColorAt(0.5, QColor(255, 255, 255, 0))
            reflection.setColorAt(1, QColor(255, 255, 255, 10))
            painter.setBrush(QBrush(reflection))
            painter.setPen(QPen(Qt.PenStyle.NoPen))
            painter.drawRect(display_rect)

    def get_status_text(self) -> str:
        """Retorna texto de status do LCD"""
        status = []
        status.append(f"Display: {'ON' if self.display_on else 'OFF'}")
        status.append(f"Cursor: ({self.cursor_row},{self.cursor_col})")
        status.append(f"Blink: {'ON' if self.blink_on else 'OFF'}")
        status.append(f"Text: '{self.display_text[0].strip()}' | '{self.display_text[1].strip()}'")
        return " | ".join(status)