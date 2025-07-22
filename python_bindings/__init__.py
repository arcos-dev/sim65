"""
Pacote principal do EMU65 Python
"""

# Imports inteligentes que funcionam em ambos os contextos
try:
    # Quando executado como módulo (pytest, etc.)
    from python_bindings.emu65_core import Emu65Core
    from python_bindings.programs_6502 import Programs6502
    from python_bindings.panels.control_panel import ControlPanel
    from python_bindings.panels.memory_panel import MemoryPanel
    from python_bindings.panels.status_panel import StatusPanel
    from python_bindings.widgets.lcd_widget import LCD16x2Widget
    from python_bindings.widgets.chip_widget import ChipWidget
except ImportError:
    # Quando executado diretamente
    from emu65_core import Emu65Core
    from programs_6502 import Programs6502
    from panels.control_panel import ControlPanel
    from panels.memory_panel import MemoryPanel
    from panels.status_panel import StatusPanel
    from widgets.lcd_widget import LCD16x2Widget
    from widgets.chip_widget import ChipWidget

__version__ = "2.0.0"
__author__ = "Anderson Costa"

__all__ = [
    'Emu65Core',
    'Programs6502',
    'ControlPanel',
    'MemoryPanel',
    'StatusPanel',
    'LCD16x2Widget',
    'ChipWidget'
] 