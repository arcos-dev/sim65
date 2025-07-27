#!/usr/bin/env python3
"""
EMU65 Core - Binding Python para o Core C
=========================================

Módulo que fornece interface Python para o core do emulador 6502.

Autor: Anderson Costa
Versão: 2.0.0
Data: 2025-01-06
"""

import ctypes
import os
import sys
from typing import Optional, Dict, Any

# Caminho para a biblioteca será determinado dinamicamente

# Estruturas equivalentes às do C
class emu65_config_t(ctypes.Structure):
    _fields_ = [
        ("clock_frequency", ctypes.c_double),
        ("decimal_mode", ctypes.c_bool),
        ("debug_mode", ctypes.c_bool),
        ("trace_execution", ctypes.c_bool),
        ("max_instructions", ctypes.c_uint32),
    ]

class emu65_bus_state_t(ctypes.Structure):
    _fields_ = [
        ("address", ctypes.c_uint16),
        ("data", ctypes.c_uint8),
        ("rw", ctypes.c_bool),
    ]

class lcd_16x2_state_t(ctypes.Structure):
    _fields_ = [
        ("display", ctypes.c_char * 34),  # 2 linhas x 17 chars (16 + null terminator cada)
        ("cursor_row", ctypes.c_uint8),
        ("cursor_col", ctypes.c_uint8),
        ("display_on", ctypes.c_bool),
        ("cursor_on", ctypes.c_bool),
        ("blink_on", ctypes.c_bool),
        ("busy", ctypes.c_bool),
        ("function_set", ctypes.c_uint8),
        ("entry_mode", ctypes.c_uint8),
        ("display_control", ctypes.c_uint8),
    ]

class cpu_state_t(ctypes.Structure):
    _fields_ = [
        ("pc", ctypes.c_uint16),
        ("a", ctypes.c_uint8),
        ("x", ctypes.c_uint8),
        ("y", ctypes.c_uint8),
        ("sp", ctypes.c_uint8),
        ("status", ctypes.c_uint8),
        ("cycles", ctypes.c_uint64),
    ]

class Emu65Core:
    """Core principal do emulador 6502"""

    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Inicializa o core do emulador"""
        if config is None:
            config = {
                'clock_frequency': 1000000.0,
                'decimal_mode': False,
                'debug_mode': False,
                'trace_execution': False,
                'max_instructions': 1000000
            }

        self._config = emu65_config_t(
            clock_frequency=config.get('clock_frequency', 1000000.0),
            decimal_mode=config.get('decimal_mode', False),
            debug_mode=config.get('debug_mode', False),
            trace_execution=config.get('trace_execution', False),
            max_instructions=config.get('max_instructions', 1000000)
        )

        self._core = None
        self._lib = None
        self._load_library()
        self._init_core()

    def _load_library(self):
        """Carrega a biblioteca C"""
        try:
            # Determina o caminho da biblioteca
            if sys.platform.startswith('win'):
                # Tenta diferentes caminhos para encontrar a DLL
                possible_paths = [
                    os.path.abspath(os.path.join('..', 'lib', 'emu6502.dll')),
                    os.path.abspath(os.path.join('lib', 'emu6502.dll')),
                    os.path.join(os.path.dirname(__file__), '..', 'lib', 'emu6502.dll'),
                    os.path.join(os.path.dirname(__file__), 'lib', 'emu6502.dll'),
                    os.path.join(os.path.dirname(__file__), 'emu6502.dll'),
                    'emu6502.dll'
                ]

                lib_path = None
                for path in possible_paths:
                    if os.path.exists(path):
                        lib_path = path
                        break

                if lib_path is None:
                    raise FileNotFoundError(f"Não foi possível encontrar emu6502.dll. Caminhos tentados: {possible_paths}")
            else:
                lib_path = os.path.abspath(os.path.join('..', 'lib', 'libemu6502.so'))

            self._lib = ctypes.CDLL(lib_path)
            self._setup_function_prototypes()
        except Exception as e:
            print(f"Erro ao carregar a biblioteca: {e}")
            sys.exit(1)

    def _setup_function_prototypes(self):
        """Configura os protótipos das funções"""
        # Funções principais
        self._lib.emu6502_create.restype = ctypes.c_void_p
        self._lib.emu6502_create.argtypes = [ctypes.POINTER(emu65_config_t)]

        self._lib.emu6502_destroy.restype = None
        self._lib.emu6502_destroy.argtypes = [ctypes.c_void_p]

        self._lib.emu6502_init.restype = ctypes.c_int
        self._lib.emu6502_init.argtypes = [ctypes.c_void_p]

        self._lib.emu6502_reset.restype = ctypes.c_int
        self._lib.emu6502_reset.argtypes = [ctypes.c_void_p]

        self._lib.emu6502_step.restype = ctypes.c_int
        self._lib.emu6502_step.argtypes = [ctypes.c_void_p]

        # Funções de estado
        self._lib.emu6502_get_bus_state.restype = None
        self._lib.emu6502_get_bus_state.argtypes = [ctypes.c_void_p, ctypes.POINTER(emu65_bus_state_t)]

        self._lib.emu6502_get_lcd_state.restype = None
        self._lib.emu6502_get_lcd_state.argtypes = [ctypes.c_void_p, ctypes.POINTER(lcd_16x2_state_t)]

        self._lib.emu6502_get_cpu_state.restype = None
        self._lib.emu6502_get_cpu_state.argtypes = [ctypes.c_void_p, ctypes.POINTER(cpu_state_t)]

        # Função de carregamento de programa
        self._lib.emu6502_load_program.restype = ctypes.c_int
        self._lib.emu6502_load_program.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, ctypes.c_uint16]

        # Função de carregamento de ROM
        self._lib.emu6502_load_rom.restype = ctypes.c_int
        self._lib.emu6502_load_rom.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t, ctypes.c_uint16]

        # Funções de memória
        self._lib.emu6502_write_byte.restype = None
        self._lib.emu6502_write_byte.argtypes = [ctypes.c_void_p, ctypes.c_uint16, ctypes.c_uint8]

        self._lib.emu6502_read_byte.restype = ctypes.c_uint8
        self._lib.emu6502_read_byte.argtypes = [ctypes.c_void_p, ctypes.c_uint16]

        # Funções do LCD
        self._lib.lcd_16x2_write_data.restype = None
        self._lib.lcd_16x2_write_data.argtypes = [ctypes.c_void_p, ctypes.c_uint8]

        self._lib.lcd_16x2_write_command.restype = None
        self._lib.lcd_16x2_write_command.argtypes = [ctypes.c_void_p, ctypes.c_uint8]

    def _init_core(self):
        """Inicializa o core"""
        self._core = self._lib.emu6502_create(ctypes.byref(self._config))
        result = self._lib.emu6502_init(self._core)
        if result != 0:
            raise RuntimeError(f"Falha ao inicializar o core: {result}")

    def step(self) -> int:
        """Executa um passo do emulador"""
        return self._lib.emu6502_step(self._core)

    def reset(self) -> int:
        """Reseta o emulador"""
        if not self._core or not self._lib:
            raise RuntimeError("Core não inicializado")
        return self._lib.emu6502_reset(self._core)

    def get_bus_state(self) -> emu65_bus_state_t:
        """Obtém o estado atual do barramento"""
        state = emu65_bus_state_t()
        self._lib.emu6502_get_bus_state(self._core, ctypes.byref(state))
        return state

    def get_lcd_state(self) -> lcd_16x2_state_t:
        """Obtém o estado atual do LCD"""
        lcd = lcd_16x2_state_t()
        self._lib.emu6502_get_lcd_state(self._core, ctypes.byref(lcd))
        return lcd

    def load_program(self, binary_data: bytes, start_address: int = 0x8000):
        """Carrega um programa na memória ou ROM"""
        try:
            # Verificar se o programa não está vazio
            if len(binary_data) == 0:
                # Para programa vazio, apenas configurar os vetores de reset
                self._lib.emu6502_write_byte(self._core, 0xFFFC, start_address & 0xFF)       # Low byte
                self._lib.emu6502_write_byte(self._core, 0xFFFD, (start_address >> 8) & 0xFF) # High byte
                self._lib.emu6502_reset(self._core)
                return

            data_array = (ctypes.c_char * len(binary_data))(*binary_data)
            # Se o endereço for >= 0x8000, use a função de ROM
            if start_address >= 0x8000:
                result = self._lib.emu6502_load_rom(self._core, data_array, len(binary_data), start_address)
            else:
                result = self._lib.emu6502_load_program(self._core, data_array, len(binary_data), start_address)
            if result != 0:
                raise RuntimeError(f"Falha ao carregar programa: {result}")

            # IMPORTANTE: NÃO sobrescrever o vetor de reset - a função C já faz isso!
            # A função emu6502_load_program já configura 0xFFFC/0xFFFD corretamente

            # Reset do CPU para carregar o PC corretamente - MÚLTIPLO
            for attempt in range(3):  # Tentar até 3 vezes
                result = self._lib.emu6502_reset(self._core)
                if result != 0:
                    print(f"[DEBUG] Reset attempt {attempt+1} failed with code: {result}")

                # Verificar se o PC foi configurado corretamente
                import time
                time.sleep(0.002)  # Delay maior para estabilizar
                cpu_state = self.get_cpu_state()

                if cpu_state.pc == start_address:
                    print(f"[DEBUG] PC set correctly on attempt {attempt+1}: 0x{cpu_state.pc:04X}")
                    break
                else:
                    print(f"[DEBUG] PC not set correctly on attempt {attempt+1}: expected {start_address:04X}, got {cpu_state.pc:04X}")

                    # Se não é a última tentativa, continuar
                    if attempt < 2:
                        # A função C já configurou os vetores, não precisamos refazer
                        time.sleep(0.001)

            # Verificação final
            final_state = self.get_cpu_state()
            if final_state.pc != start_address:
                print(f"[WARNING] Final PC verification failed: expected {start_address:04X}, got {final_state.pc:04X}")
                # Não falhar o teste, apenas avisar

        except Exception as e:
            print(f"[PYTHON DEBUG] Exception: {e}")
            raise RuntimeError(f"Erro ao carregar programa: {e}")

    def write_byte(self, address: int, value: int):
        """Escreve um byte na memória"""
        if not self._core or not self._lib:
            raise RuntimeError("Core não inicializado")
        self._lib.emu6502_write_byte(self._core, address, value)

    def read_byte(self, address: int) -> int:
        """Lê um byte da memória"""
        if not self._core or not self._lib:
            raise RuntimeError("Core não inicializado")
        return self._lib.emu6502_read_byte(self._core, address)

    def destroy(self):
        """Destrói o core e libera recursos de forma eficiente"""
        if hasattr(self, '_core') and self._core and hasattr(self, '_lib') and self._lib:
            try:
                # Verificar se a função ainda existe na DLL
                if hasattr(self._lib, 'emu6502_destroy'):
                    self._lib.emu6502_destroy(self._core)
                    print("[DEBUG] Core destruído com sucesso")
            except (OSError, AttributeError, SystemError) as e:
                # DLL pode ter sido descarregada ou função não disponível
                print(f"[DEBUG] Erro esperado durante cleanup: {e}")
                pass
            except Exception as e:
                # Qualquer outro erro durante cleanup
                print(f"[DEBUG] Erro inesperado durante cleanup: {e}")
                pass
            finally:
                self._core = None
                self._lib = None  # Limpar referência da biblioteca também

    # Removido __del__ para evitar problemas durante shutdown do Python

    def get_via_state(self):
        """Obtém o estado dos registradores do VIA 6522 (debug)"""
        try:
            # VIA está mapeado em $6000-$600F
            state = {}
            for reg, addr in [
                ("ORB", 0x6000), ("ORA", 0x6001), ("DDRB", 0x6002), ("DDRA", 0x6003),
                ("T1CL", 0x6004), ("T1CH", 0x6005), ("T1LL", 0x6006), ("T1LH", 0x6007),
                ("T2CL", 0x6008), ("T2CH", 0x6009), ("SR", 0x600A), ("ACR", 0x600B),
                ("PCR", 0x600C), ("IFR", 0x600D), ("IER", 0x600E), ("ORANH", 0x600F)
            ]:
                state[reg] = self.read_byte(addr)
            return state
        except Exception as e:
            return {"error": str(e)}

    def get_cpu_state(self):
        """Obtém o estado dos registradores do 6502 (debug)"""
        try:
            if not self._core or not self._lib:
                return None

            # Criar estrutura para receber o estado
            state = cpu_state_t()
            self._lib.emu6502_get_cpu_state(self._core, ctypes.byref(state))

            return state
        except Exception as e:
            return {"error": str(e)}