"""
Panels Package - Painéis da Interface
=====================================

Pacote contendo os painéis principais da interface do emulador.
"""

# Imports inteligentes para compatibilidade
try:
    from .control_panel import ControlPanel
    from .memory_panel import MemoryPanel
    from .status_panel import StatusPanel
except ImportError:
    try:
        from python_bindings.panels.control_panel import ControlPanel
        from python_bindings.panels.memory_panel import MemoryPanel
        from python_bindings.panels.status_panel import StatusPanel
    except ImportError:
        from control_panel import ControlPanel
        from memory_panel import MemoryPanel
        from status_panel import StatusPanel

__all__ = [
    'ControlPanel',
    'MemoryPanel', 
    'StatusPanel'
] 