"""
Widgets Package - Widgets Especializados
========================================

Pacote contendo widgets especializados para a interface do emulador.
"""

# Imports inteligentes para compatibilidade
try:
    from .lcd_widget import LCD16x2Widget
    from .control_widgets import LEDWidget, SwitchWidget, RegisterDisplay, StatusLabel
except ImportError:
    try:
        from python_bindings.widgets.lcd_widget import LCD16x2Widget
        from python_bindings.widgets.control_widgets import LEDWidget, SwitchWidget, RegisterDisplay, StatusLabel
    except ImportError:
        from lcd_widget import LCD16x2Widget
        from control_widgets import LEDWidget, SwitchWidget, RegisterDisplay, StatusLabel

__all__ = [
    'LCD16x2Widget',
    'LEDWidget', 
    'SwitchWidget', 
    'RegisterDisplay', 
    'StatusLabel'
] 