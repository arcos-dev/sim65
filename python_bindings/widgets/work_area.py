from PyQt6.QtWidgets import QWidget, QFrame, QVBoxLayout, QSizePolicy
from PyQt6.QtCore import Qt, QPoint, QRect, QTimer, pyqtSignal
from PyQt6.QtGui import QPainter, QPen, QBrush, QColor, QMouseEvent, QCursor

class WorkAreaWidget(QFrame):
    """
    Área de trabalho central para simulação de breadboard.
    Suporta componentes arrastáveis com grid inteligente, snap-to-grid e rotação.
    """

    # Sinais para comunicação com outros componentes
    component_moved = pyqtSignal(object, QPoint)  # widget, new_position
    component_rotated = pyqtSignal(object, int)   # widget, rotation_angle

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("work-area")
        self.setMinimumSize(400, 300)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        self.setStyleSheet("background: #23272A; border: 2px solid #35393C; border-radius: 10px;")

        # Configurações do grid
        self.grid_size = 20  # Tamanho do grid em pixels
        self.snap_to_grid = True  # Ativar snap-to-grid
        self.show_grid = True  # Mostrar grid de fundo

        # Componentes e estado de arrastar
        self.components = []  # Lista de componentes: {'widget', 'pos', 'rotation', 'original_pos', 'id'}
        self.dragging = None  # (component_id, offset, start_pos)
        self.rotating = None  # (component_id, start_angle)
        self.next_component_id = 1  # ID único para cada componente

        # Timer para rotação suave
        self.rotation_timer = QTimer()
        self.rotation_timer.setInterval(50)  # 50ms para rotação suave
        self.rotation_timer.timeout.connect(self.update_rotation)

        # Configurar cursor
        self.setCursor(QCursor(Qt.CursorShape.ArrowCursor))

        # Ativar eventos de mouse
        self.setMouseTracking(True)
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)

    def add_component(self, widget, pos=None):
        """Adiciona um componente à área de trabalho"""
        if pos is None:
            # Posição padrão no grid
            pos = QPoint(self.grid_size * 2, self.grid_size * 2)

        # Snap inicial ao grid
        if self.snap_to_grid:
            pos = self.snap_to_grid_point(pos)

        # Verificar se a posição está ocupada
        while self.is_position_occupied(pos):
            pos = QPoint(pos.x() + self.grid_size, pos.y())
            if pos.x() > self.width() - 100:  # Se chegou ao limite direito
                pos = QPoint(self.grid_size * 2, pos.y() + self.grid_size)

        component = {
            'id': self.next_component_id,
            'widget': widget,
            'pos': pos,
            'rotation': 0,  # Rotação em graus
            'original_pos': QPoint(pos.x(), pos.y()),
            'original_rotation': 0
        }

        self.components.append(component)
        self.next_component_id += 1
        
        widget.setParent(self)
        widget.move(pos)
        widget.show()

        # Aplicar rotação inicial
        self.apply_rotation(widget, 0)
        self.update()

    def is_position_occupied(self, pos):
        """Verifica se uma posição está ocupada por outro componente"""
        for comp in self.components:
            if comp['pos'] == pos:
                return True
        return False

    def snap_to_grid_point(self, point):
        """Converte um ponto para o grid mais próximo"""
        x = round(point.x() / self.grid_size) * self.grid_size
        y = round(point.y() / self.grid_size) * self.grid_size
        return QPoint(x, y)

    def get_grid_point(self, point):
        """Retorna o ponto do grid mais próximo"""
        return self.snap_to_grid_point(point)

    def apply_rotation(self, widget, angle):
        """Aplica rotação a um widget"""
        # Salvar a rotação atual
        widget.setProperty("rotation", angle)

        # Aplicar transformação visual se o widget suportar
        if hasattr(widget, 'setRotation'):
            widget.setRotation(angle)

    def mousePressEvent(self, event: QMouseEvent):
        """Manipula eventos de clique do mouse"""
        if event.button() == Qt.MouseButton.LeftButton:
            # Verificar se clicou em algum componente (do mais recente para o mais antigo)
            for comp in reversed(self.components):
                widget = comp['widget']
                if not widget or not widget.isVisible():
                    continue
                    
                rect = QRect(comp['pos'], widget.size())

                if rect.contains(event.pos()):
                    # Se pressionar Ctrl + clique esquerdo, rotacionar
                    if event.modifiers() & Qt.KeyboardModifier.ControlModifier:
                        self.start_rotation(comp['id'], event.pos())
                    else:
                        # Arrastar componente
                        self.start_dragging(comp['id'], event.pos())

                    # Trazer componente para frente
                    self.components.remove(comp)
                    self.components.append(comp)
                    self.update()
                    break

        elif event.button() == Qt.MouseButton.RightButton:
            # Clique direito para rotacionar (alternativa)
            for comp in reversed(self.components):
                widget = comp['widget']
                if not widget or not widget.isVisible():
                    continue
                    
                rect = QRect(comp['pos'], widget.size())

                if rect.contains(event.pos()):
                    self.start_rotation(comp['id'], event.pos())
                    # Trazer componente para frente
                    self.components.remove(comp)
                    self.components.append(comp)
                    self.update()
                    break

    def start_dragging(self, component_id, pos):
        """Inicia o arrastar de um componente"""
        comp = self.get_component_by_id(component_id)
        if comp:
            offset = pos - comp['pos']
            self.dragging = (component_id, offset, QPoint(comp['pos'].x(), comp['pos'].y()))
            # Mudar cursor
            self.setCursor(QCursor(Qt.CursorShape.ClosedHandCursor))

    def start_rotation(self, component_id, pos):
        """Inicia a rotação de um componente"""
        comp = self.get_component_by_id(component_id)
        if comp:
            self.rotating = (component_id, comp['rotation'])
            # Mudar cursor
            self.setCursor(QCursor(Qt.CursorShape.CrossCursor))

    def get_component_by_id(self, component_id):
        """Retorna um componente pelo ID"""
        for comp in self.components:
            if comp['id'] == component_id:
                return comp
        return None

    def mouseMoveEvent(self, event: QMouseEvent):
        """Manipula eventos de movimento do mouse"""
        if self.dragging:
            component_id, offset, start_pos = self.dragging
            comp = self.get_component_by_id(component_id)
            
            if not comp or not comp['widget']:
                self.dragging = None
                self.setCursor(QCursor(Qt.CursorShape.ArrowCursor))
                return
                
            new_pos = event.pos() - offset

            # Snap ao grid se ativado
            if self.snap_to_grid:
                new_pos = self.snap_to_grid_point(new_pos)

            # Verificar se a nova posição não está ocupada por outro componente
            if not self.is_position_occupied_by_other(new_pos, component_id):
                # Atualizar posição
                comp['pos'] = new_pos
                comp['widget'].move(new_pos)

                # Emitir sinal de movimento
                self.component_moved.emit(comp['widget'], new_pos)
                self.update()

        elif self.rotating:
            component_id, start_angle = self.rotating
            comp = self.get_component_by_id(component_id)
            
            if not comp or not comp['widget']:
                self.rotating = None
                self.setCursor(QCursor(Qt.CursorShape.ArrowCursor))
                return
                
            # Calcular ângulo baseado na posição do mouse
            center = comp['pos'] + QPoint(comp['widget'].width() // 2, comp['widget'].height() // 2)
            angle = self.calculate_rotation_angle(center, event.pos())

            # Aplicar rotação
            self.apply_rotation(comp['widget'], angle)
            comp['rotation'] = angle

            # Emitir sinal de rotação
            self.component_rotated.emit(comp['widget'], angle)
            self.update()

    def is_position_occupied_by_other(self, pos, exclude_id):
        """Verifica se uma posição está ocupada por outro componente (excluindo um ID específico)"""
        for comp in self.components:
            if comp['id'] != exclude_id and comp['pos'] == pos:
                return True
        return False

    def calculate_rotation_angle(self, center, mouse_pos):
        """Calcula o ângulo de rotação baseado na posição do mouse"""
        dx = mouse_pos.x() - center.x()
        dy = mouse_pos.y() - center.y()
        angle = (180 / 3.14159) * (3.14159 / 2 + 3.14159 * (dy < 0)) + (180 / 3.14159) * (3.14159 + 3.14159 * (dx < 0)) + (180 / 3.14159) * (3.14159 / 2) * (dx * dy < 0)
        return int(angle) % 360

    def mouseReleaseEvent(self, event: QMouseEvent):
        """Manipula eventos de soltar o mouse"""
        if self.dragging:
            component_id, offset, start_pos = self.dragging
            comp = self.get_component_by_id(component_id)
            
            if comp:
                final_pos = comp['pos']

                # Se a posição mudou, salvar como nova posição original
                if final_pos != start_pos:
                    comp['original_pos'] = QPoint(final_pos.x(), final_pos.y())

            self.dragging = None
            self.setCursor(QCursor(Qt.CursorShape.ArrowCursor))

        elif self.rotating:
            component_id, start_angle = self.rotating
            comp = self.get_component_by_id(component_id)
            
            if comp:
                final_angle = comp['rotation']

                # Se a rotação mudou, salvar como nova rotação original
                if final_angle != start_angle:
                    comp['original_rotation'] = final_angle

            self.rotating = None
            self.setCursor(QCursor(Qt.CursorShape.ArrowCursor))

    def update_rotation(self):
        """Atualiza a rotação suave (para animações futuras)"""
        if self.rotating:
            self.update()

    def paintEvent(self, event):
        """Desenha o grid de fundo e outros elementos visuais"""
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)

        if self.show_grid:
            # Desenhar grid de fundo
            grid_color = QColor(50, 55, 60, 80)  # Semi-transparente
            painter.setPen(QPen(grid_color, 1))

            # Linhas verticais
            for x in range(0, self.width(), self.grid_size):
                painter.drawLine(x, 0, x, self.height())

            # Linhas horizontais
            for y in range(0, self.height(), self.grid_size):
                painter.drawLine(0, y, self.width(), y)

    def toggle_grid(self):
        """Alterna a visibilidade do grid"""
        self.show_grid = not self.show_grid
        self.update()

    def toggle_snap_to_grid(self):
        """Alterna o snap-to-grid"""
        self.snap_to_grid = not self.snap_to_grid

    def set_grid_size(self, size):
        """Define o tamanho do grid"""
        self.grid_size = size
        self.update()

    def clear_components(self):
        """Remove todos os componentes da área de trabalho"""
        for comp in self.components:
            if comp['widget']:
                comp['widget'].setParent(None)
                comp['widget'].deleteLater()
        
        self.components.clear()
        self.next_component_id = 1
        self.dragging = None
        self.rotating = None
        self.update()

    def get_component_at(self, pos):
        """Retorna o componente em uma posição específica"""
        for comp in reversed(self.components):
            if comp['widget'] and comp['widget'].isVisible():
                rect = QRect(comp['pos'], comp['widget'].size())
                if rect.contains(pos):
                    return comp
        return None

    def get_all_components(self):
        """Retorna todos os componentes"""
        return self.components