from PyQt6.QtWidgets import QFrame
from PyQt6.QtCore import Qt, QRectF
from PyQt6.QtGui import QPainter, QColor, QFont, QPen, QBrush

class ChipWidget(QFrame):
    """
    Widget visual de chip (CI) com pinos laterais e nome centralizado.
    Suporta drag & drop na área de trabalho.
    """
    def __init__(self, name="6502 CPU", pin_names=None, parent=None):
        super().__init__(parent)
        self.name = name
        self.pin_names = pin_names or [f"P{i+1}" for i in range(16)]
        self.n_pins = len(self.pin_names)
        self.setFixedSize(120, max(60, 20 + self.n_pins * 10))
        self.setStyleSheet("background: transparent;")
        self.setMouseTracking(True)
        self.hovered_pin = None

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        rect = self.rect().adjusted(8, 8, -8, -8)
        # Corpo do chip
        painter.setBrush(QBrush(QColor(60, 60, 60)))
        painter.setPen(QPen(QColor(40, 40, 40), 2))
        painter.drawRoundedRect(rect, 8, 8)
        # Nome do chip
        painter.setFont(QFont("Segoe UI Variable", 10, QFont.Weight.Bold))
        painter.setPen(QPen(QColor(255, 255, 255)))
        painter.drawText(rect, Qt.AlignmentFlag.AlignCenter, self.name)
        # Pinos
        pin_size = 8
        n_half = self.n_pins // 2
        for i, pin in enumerate(self.pin_names):
            if i < n_half:
                # Lado esquerdo
                x = rect.left() - pin_size
                y = rect.top() + 10 + i * (rect.height() - 20) // (n_half - 1)
                align = Qt.AlignmentFlag.AlignRight
            else:
                # Lado direito
                x = rect.right()
                y = rect.top() + 10 + (i - n_half) * (rect.height() - 20) // (self.n_pins - n_half - 1)
                align = Qt.AlignmentFlag.AlignLeft
            # Pino
            pin_rect = QRectF(x, y - pin_size // 2, pin_size, pin_size)
            painter.setBrush(QBrush(QColor(180, 180, 180)))
            painter.setPen(QPen(QColor(80, 80, 40), 1))
            painter.drawEllipse(pin_rect)
            # Nome do pino
            painter.setFont(QFont("Segoe UI Variable", 7))
            painter.setPen(QPen(QColor(200, 200, 200)))
            if align == Qt.AlignmentFlag.AlignRight:
                painter.drawText(x - 30, y + 3, 28, 10, align, pin)
            else:
                painter.drawText(x + pin_size + 2, y + 3, 28, 10, align, pin) 