/**
 * EMU6502 API - Implementação da interface Python
 * ===============================================
 *
 * Implementação das funções da API que conecta o emulador sim65
 * com a interface Python existente.
 *
 * Esta implementação segue a arquitetura do sim65 usando bus_init,
 * cpu6502_ini        // PORTA (0x6001) - controle do LCD (Ben Eater style)
        if (address == 0x6001) {
            bool rs = (value & 0x20) != 0;  // RS bit (PA5)
            bool rw = (value & 0x40) != 0;  // RW bit (PA6)
            bool e = (value & 0x80) != 0;   // Enable bit (PA7)

            // Estado anterior do Enable
            bool prev_e = (last_porta_control & 0x80) != 0;
            bool e_falling_edge = prev_e && !e;  // Borda de descida do Enable.
 *
 * Autor: Anderson Costa
 * Data: 2025-01-21
 */

#include "emu6502_api.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Incluir headers do sim65
#include "../../src/cpu.h"
#include "../../src/memory.h"
#include "../../src/bus.h"
#include "../../src/via.h"
#include "../../src/clock.h"
#include "../../src/acia.h"
#include "../../src/tia.h"

// Estrutura interna do emulador seguindo a arquitetura do sim65
typedef struct {
    emu65_config_t config;

    // Componentes principais
    bus_t bus;
    VIA6522* via;
    Acia6550* acia;
    TIA* tia;

    // Estados internos para interface Python
    emu65_bus_state_t last_bus_state;
    lcd_16x2_state_t lcd_state;
    via_state_t via_state;

    bool initialized;
    bool running;

    // Callback para interceptar escritas
    void (*write_callback)(struct emu6502_context* ctx, uint16_t address, uint8_t value);
} emu6502_context_t;

// Ponteiro global para o contexto ativo (para callbacks de bus)
// Global context for tracking active emulator instance
static emu6502_context_t* g_active_context = NULL;

// Função auxiliar para inicializar o LCD state
static void init_lcd_state(lcd_16x2_state_t* lcd) {
    memset(lcd->display, ' ', 32);
    lcd->display[16] = '\0';
    lcd->display[33] = '\0';
    lcd->cursor_row = 0;
    lcd->cursor_col = 0;
    lcd->display_on = true;
    lcd->cursor_on = false;
    lcd->blink_on = false;
    lcd->busy = false;
    lcd->function_set = 0x38; // 8-bit, 2 line, 5x8 dots
    lcd->entry_mode = 0x06;   // Increment cursor, no shift
    lcd->display_control = 0x0C; // Display on, cursor off, blink off
}

EMU6502_API void* emu6502_create(const emu65_config_t* config) {
    emu6502_context_t* ctx = (emu6502_context_t*)malloc(sizeof(emu6502_context_t));
    if (!ctx) {
        return NULL;
    }

    // Copiar configuração
    if (config) {
        ctx->config = *config;
    } else {
        // Configuração padrão
        ctx->config.clock_frequency = 1000000.0;
        ctx->config.decimal_mode = false;
        ctx->config.debug_mode = false;
        ctx->config.trace_execution = false;
        ctx->config.max_instructions = 1000000;
    }

    // Inicializar ponteiros
    ctx->via = NULL;
    ctx->acia = NULL;
    ctx->tia = NULL;
    ctx->initialized = false;
    ctx->running = false;

    // Inicializar estados
    memset(&ctx->last_bus_state, 0, sizeof(emu65_bus_state_t));
    init_lcd_state(&ctx->lcd_state);
    memset(&ctx->via_state, 0, sizeof(via_state_t));

    return ctx;
}

EMU6502_API void emu6502_destroy(void* emu) {
    if (!emu) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    // Limpar componentes na ordem correta
    cpu6502_destroy();

    if (ctx->via) {
        via_destroy(ctx->via);
    }

    if (ctx->acia) {
        acia_destroy(ctx->acia);
    }

    if (ctx->tia) {
        tia_destroy(ctx->tia);
    }

    bus_destroy(&ctx->bus);

    free(ctx);
}

EMU6502_API int emu6502_init(void* emu) {
    if (!emu) {
        return -1;
    }

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (ctx->initialized) {
        return 0; // Já inicializado
    }

    // Inicializar VIA 6522
    ctx->via = via_init();
    if (!ctx->via) {
        return -1;
    }

    // Inicializar ACIA (opcional para compatibilidade)
    ctx->acia = acia_init();

    // Inicializar TIA (opcional para compatibilidade)
    ctx->tia = tia_init(TV_SYSTEM_NTSC);

    // Inicializar bus com memória de 64KB, clock e dispositivos
    if (bus_init(&ctx->bus, 65536, ctx->config.clock_frequency, ctx->acia, ctx->tia) != 0) {
        // Limpar em caso de erro
        if (ctx->via) via_destroy(ctx->via);
        if (ctx->acia) acia_destroy(ctx->acia);
        if (ctx->tia) tia_destroy(ctx->tia);
        return -1;
    }

    // Inicializar CPU com o bus - SEMPRE FORÇAR NOVA INSTÂNCIA
    // Primeiro destruir CPU existente (se houver) para resolver problema de singleton
    cpu6502_destroy();

    if (cpu6502_init(&ctx->bus) != 0) {
        // Se falhou após destruição, erro real
        bus_destroy(&ctx->bus);
        if (ctx->via) via_destroy(ctx->via);
        if (ctx->acia) acia_destroy(ctx->acia);
        if (ctx->tia) tia_destroy(ctx->tia);
        return -1;
    }

    ctx->initialized = true;

    // Configurar o contexto ativo para monitoramento de bus
    g_active_context = ctx;

    return 0;
}

EMU6502_API int emu6502_reset(void* emu) {
    if (!emu) return -1;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (!ctx->initialized) {
        return -1;
    }

    // cpu6502_reset() retorna número de ciclos (7), não código de erro
    // Valores positivos são sucesso, negativos são erro
    int result = cpu6502_reset();

    if (result < 0) {
        return result; // Erro real
    }

    return 0; // Sucesso - convertemos ciclos para código de sucesso
}

EMU6502_API int emu6502_step(void* emu) {
    if (!emu) return -1;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (!ctx->initialized) {
        return -1;
    }

    // Capturar estado antes da execução
    cpu_state_t prev_state;
    emu6502_get_cpu_state(emu, &prev_state);

    // Capturar estado VIA antes da execução para detectar mudanças
    uint8_t prev_portb = 0;
    uint8_t prev_porta = 0;
    if (ctx->via) {
        prev_portb = bus_read_memory(&ctx->bus, 0x6000);  // VIA PORTB
        prev_porta = bus_read_memory(&ctx->bus, 0x6001);  // VIA PORTA
    }

    // Executar a instrução
    int result = cpu6502_step();

    // Capturar estado após a execução
    cpu_state_t curr_state;
    emu6502_get_cpu_state(emu, &curr_state);

    // Verificar mudanças VIA após execução e processar LCD
    if (ctx->via) {
        uint8_t curr_portb = bus_read_memory(&ctx->bus, 0x6000);  // VIA PORTB
        uint8_t curr_porta = bus_read_memory(&ctx->bus, 0x6001);  // VIA PORTA

        // Se houve mudança nos registros VIA, processar como operação LCD
        if (curr_porta != prev_porta || curr_portb != prev_portb) {
            // Simular a mesma lógica do emu6502_write_byte para VIA
            bool rs = (curr_porta & 0x20) != 0;  // RS bit (PA5)
            bool rw = (curr_porta & 0x40) != 0;  // RW bit (PA6)
            bool e = (curr_porta & 0x80) != 0;   // Enable bit (PA7)

            // Detectar borda de descida do Enable
            bool prev_e = (prev_porta & 0x80) != 0;
            bool e_falling_edge = prev_e && !e;

            // Processar LCD na borda de descida do Enable
            if (e_falling_edge && !rw) {
                if (rs) {
                    // RS=1: Dados (escrever caractere)
                    if (curr_portb >= 32 && curr_portb < 127) {
                        emu6502_lcd_write_char(emu, curr_portb);
                    }
                } else {
                    // RS=0: Comando
                    lcd_16x2_state_t* lcd = &ctx->lcd_state;

                    if (curr_portb == 0x01) {
                        // Clear Display
                        emu6502_lcd_clear(emu);
                    } else if ((curr_portb & 0xF8) == 0x38) {
                        // Function Set (0x38 = 8-bit, 2 line, 5x8 dots)
                        lcd->function_set = curr_portb;
                    } else if ((curr_portb & 0xF8) == 0x08) {
                        // Display On/Off Control (0x08-0x0F)
                        lcd->display_control = curr_portb;
                        lcd->display_on = (curr_portb & 0x04) != 0;
                        lcd->cursor_on = (curr_portb & 0x02) != 0;
                        lcd->blink_on = (curr_portb & 0x01) != 0;
                    } else if ((curr_portb & 0xFC) == 0x04) {
                        // Entry Mode Set (0x04-0x07)
                        lcd->entry_mode = curr_portb;
                    } else if ((curr_portb & 0x80) == 0x80) {
                        // Set DDRAM Address (0x80-0xFF)
                        uint8_t addr = curr_portb & 0x7F;
                        if (addr < 0x40) {
                            lcd->cursor_row = 0;
                            lcd->cursor_col = addr;
                        } else if (addr >= 0x40 && addr < 0x40 + 16) {
                            lcd->cursor_row = 1;
                            lcd->cursor_col = addr - 0x40;
                        }
                    }
                }
            }
        }
    }

    // Simular captura de barramento baseado na mudança de PC
    // Isto é uma simplificação - operações reais de barramento seriam mais complexas
    ctx->last_bus_state.address = curr_state.pc;
    ctx->last_bus_state.data = 0x00; // Não temos o dado exato, mas PC mudou
    ctx->last_bus_state.rw = true; // Assume leitura de instrução

    return result;
}

EMU6502_API int emu6502_run_cycles(void* emu, uint32_t cycles) {
    if (!emu) return -1;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (!ctx->initialized) {
        return -1;
    }

    for (uint32_t i = 0; i < cycles; i++) {
        int result = cpu6502_step();
        if (result < 0) {
            return result;
        }
    }

    return 0;
}

EMU6502_API int emu6502_load_program(void* emu, const char* data, size_t size, uint16_t address) {
    if (!emu || !data || size == 0) {
        return -1;
    }

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (!ctx->initialized) {
        return -1;
    }

    // Verificar bounds
    if (address + size > 65536) {
        return -1;
    }

    // Carregar dados na memória através do bus
    for (size_t i = 0; i < size; i++) {
        bus_write_memory(&ctx->bus, address + i, (uint8_t)data[i]);
    }

    // IMPORTANTE: Configurar vetor de reset para apontar para o endereço inicial
    bus_write_memory(&ctx->bus, 0xFFFC, address & 0xFF);        // Low byte
    bus_write_memory(&ctx->bus, 0xFFFD, (address >> 8) & 0xFF); // High byte

    return 0;
}EMU6502_API int emu6502_load_rom(void* emu, const char* data, size_t size, uint16_t address) {
    // Para esta implementação, ROM e programa são tratados da mesma forma
    return emu6502_load_program(emu, data, size, address);
}

EMU6502_API int emu6502_load_file(void* emu, const char* filename, uint16_t address) {
    if (!emu || !filename) return -1;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (!ctx->initialized) {
        return -1;
    }

    return bus_load_program(&ctx->bus, filename, address);
}

EMU6502_API uint8_t emu6502_read_byte(void* emu, uint16_t address) {
    if (!emu) return 0;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (!ctx->initialized) {
        return 0;
    }

    return bus_read_memory(&ctx->bus, address);
}

EMU6502_API void emu6502_write_byte(void* emu, uint16_t address, uint8_t value) {
    if (!emu) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (!ctx->initialized) {
        return;
    }

    // Atualizar o estado do barramento para captura
    ctx->last_bus_state.address = address;
    ctx->last_bus_state.data = value;
    ctx->last_bus_state.rw = false; // Write operation    // Interceptar escritas no VIA para LCD
    if (address >= 0x6000 && address <= 0x600F) {
        static uint8_t last_portb_data = 0;  // Valor escrito no PORTB (dados LCD)
        static uint8_t last_porta_control = 0;  // Estado anterior do PORTA para detectar mudanças

        // Fazer a escrita no bus primeiro
        bus_write_memory(&ctx->bus, address, value);

        // PORTB (0x6000) - dados do LCD (Ben Eater style)
        if (address == 0x6000) {
            last_portb_data = value;
        }

        // PORTA (0x6001) - controle do LCD (Ben Eater style)
        if (address == 0x6001) {
            printf("[ANDERSON DEBUG] PORTA = 0x%02X, E=%d, RS=%d\n",
                   value,
                   (value & 0x80) ? 1 : 0,  // E bit (PA7)
                   (value & 0x20) ? 1 : 0   // RS bit (PA5)
            );
            fflush(stdout);

            bool rs = (value & 0x20) != 0;  // RS bit (PA5)
            bool rw = (value & 0x40) != 0;  // RW bit (PA6)
            bool e = (value & 0x80) != 0;   // Enable bit (PA7)

            // Estado anterior do Enable
            bool prev_e = (last_porta_control & 0x80) != 0;
            bool e_falling_edge = prev_e && !e;  // Borda de descida do Enable            printf("Control bits: RS=%d, E=%d, RW=%d\n", rs, e, rw);
            fflush(stdout);

            // Processa comandos na borda de descida do Enable
            if (e_falling_edge && !rw) {
                printf("LCD Data from PORTB: 0x%02X ('%c')\n", last_portb_data,
                       (last_portb_data >= 32 && last_portb_data < 127) ? last_portb_data : '?');
                fflush(stdout);

                if (rs) {
                    // RS=1: Dados (escrever caractere)
                    if (last_portb_data >= 32 && last_portb_data < 127) {
                        emu6502_lcd_write_char(emu, last_portb_data);
                        printf("LCD: Caractere '%c' escrito no display\n", last_portb_data);
                        fflush(stdout);
                    }
                } else {
                    // RS=0: Comando
                    printf("LCD: Comando 0x%02X\n", last_portb_data);
                    fflush(stdout);

                    emu6502_context_t* ctx = (emu6502_context_t*)emu;
                    lcd_16x2_state_t* lcd = &ctx->lcd_state;

                    if (last_portb_data == 0x01) {
                        // Clear Display
                        emu6502_lcd_clear(emu);
                        printf("LCD: Display limpo\n");
                        fflush(stdout);
                    } else if ((last_portb_data & 0xF8) == 0x38) {
                        // Function Set (0x38 = 8-bit, 2 line, 5x8 dots)
                        lcd->function_set = last_portb_data;
                        printf("LCD: Function Set 0x%02X configurado\n", last_portb_data);
                        fflush(stdout);
                    } else if ((last_portb_data & 0xF8) == 0x08) {
                        // Display On/Off Control (0x08-0x0F)
                        lcd->display_control = last_portb_data;
                        lcd->display_on = (last_portb_data & 0x04) != 0;
                        lcd->cursor_on = (last_portb_data & 0x02) != 0;
                        lcd->blink_on = (last_portb_data & 0x01) != 0;
                        printf("LCD: Display Control 0x%02X - ON:%d, CURSOR:%d, BLINK:%d\n",
                               last_portb_data, lcd->display_on, lcd->cursor_on, lcd->blink_on);
                        fflush(stdout);
                    } else if ((last_portb_data & 0xFC) == 0x04) {
                        // Entry Mode Set (0x04-0x07)
                        lcd->entry_mode = last_portb_data;
                        printf("LCD: Entry Mode 0x%02X configurado\n", last_portb_data);
                        fflush(stdout);
                    } else if ((last_portb_data & 0x80) == 0x80) {
                        // Set DDRAM Address (0x80-0xFF)
                        uint8_t addr = last_portb_data & 0x7F;
                        if (addr < 0x40) {
                            lcd->cursor_row = 0;
                            lcd->cursor_col = addr;
                        } else if (addr >= 0x40 && addr < 0x40 + 16) {
                            lcd->cursor_row = 1;
                            lcd->cursor_col = addr - 0x40;
                        }
                        printf("LCD: Set cursor to row=%d, col=%d (addr=0x%02X)\n",
                               lcd->cursor_row, lcd->cursor_col, addr);
                        fflush(stdout);
                    } else {
                        printf("LCD: Comando não implementado: 0x%02X\n", last_portb_data);
                        fflush(stdout);
                    }
                }
            }

            last_porta_control = value;  // Salva estado atual para próxima comparação
        }

        return; // Já fizemos a escrita
    }

    bus_write_memory(&ctx->bus, address, value);
}

EMU6502_API void emu6502_get_cpu_state(void* emu, cpu_state_t* state) {
    if (!emu || !state) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (!ctx->initialized) {
        memset(state, 0, sizeof(cpu_state_t));
        return;
    }

    // Acessar o CPU global (sim65 usa CPU global)
    extern cpu6502_t *cpu;
    if (cpu) {
        state->pc = cpu->pc;
        state->a = cpu->a;
        state->x = cpu->x;
        state->y = cpu->y;
        state->sp = cpu->sp;
        state->status = cpu->status;
        state->cycles = (uint64_t)cpu->cycles;
    } else {
        memset(state, 0, sizeof(cpu_state_t));
    }
}

EMU6502_API void emu6502_get_bus_state(void* emu, emu65_bus_state_t* state) {
    if (!emu || !state) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;
    *state = ctx->last_bus_state;
}

EMU6502_API void emu6502_get_via_state(void* emu, via_state_t* state) {
    if (!emu || !state) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (!ctx->initialized || !ctx->via) {
        memset(state, 0, sizeof(via_state_t));
        return;
    }

    state->address = 0x6000; // Endereço base do VIA
    // Ler registradores do VIA usando a interface correta
    state->data_direction_a = via_read(ctx->via, VIA_REG_DDRA);
    state->data_direction_b = via_read(ctx->via, VIA_REG_DDRB);
    state->output_register_a = via_read(ctx->via, VIA_REG_ORA);
    state->output_register_b = via_read(ctx->via, VIA_REG_ORB);
    state->input_register_a = via_read(ctx->via, VIA_REG_ORA);
    state->input_register_b = via_read(ctx->via, VIA_REG_ORB);
}

EMU6502_API void emu6502_get_lcd_state(void* emu, lcd_16x2_state_t* state) {
    if (!emu || !state) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;
    *state = ctx->lcd_state;
}

// Funções do LCD (simuladas - atualizarão o estado interno)
EMU6502_API void emu6502_lcd_clear(void* emu) {
    if (!emu) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;
    init_lcd_state(&ctx->lcd_state);
}

EMU6502_API void emu6502_lcd_write_char(void* emu, char c) {
    if (!emu) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;
    lcd_16x2_state_t* lcd = &ctx->lcd_state;

    if (lcd->cursor_row < 2 && lcd->cursor_col < 16) {
        int pos = lcd->cursor_row * 17 + lcd->cursor_col;
        lcd->display[pos] = c;

        lcd->cursor_col++;
        if (lcd->cursor_col >= 16) {
            lcd->cursor_col = 0;
            lcd->cursor_row = (lcd->cursor_row + 1) % 2;
        }
    }
}

EMU6502_API void emu6502_lcd_write_string(void* emu, const char* str) {
    if (!emu || !str) return;

    while (*str) {
        emu6502_lcd_write_char(emu, *str++);
    }
}

EMU6502_API void emu6502_lcd_set_cursor(void* emu, uint8_t row, uint8_t col) {
    if (!emu) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (row < 2 && col < 16) {
        ctx->lcd_state.cursor_row = row;
        ctx->lcd_state.cursor_col = col;
    }
}

// Funções de configuração
EMU6502_API void emu6502_set_clock_frequency(void* emu, double freq) {
    if (!emu || freq <= 0) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;
    ctx->config.clock_frequency = freq;

    // Nota: No sim65, a frequência é definida na inicialização do bus
    // Para mudança dinâmica seria necessário re-inicializar
}

EMU6502_API double emu6502_get_clock_frequency(void* emu) {
    if (!emu) return 0.0;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;
    return ctx->config.clock_frequency;
}

EMU6502_API void emu6502_set_debug_mode(void* emu, bool enabled) {
    if (!emu) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;
    ctx->config.debug_mode = enabled;
}

EMU6502_API bool emu6502_get_debug_mode(void* emu) {
    if (!emu) return false;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;
    return ctx->config.debug_mode;
}

// Funções de E/S VIA
EMU6502_API void emu6502_via_write(void* emu, uint16_t address, uint8_t value) {
    if (!emu) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (ctx->initialized && ctx->via) {
        via_write(ctx->via, address & 0x0F, value);
    }
}

EMU6502_API uint8_t emu6502_via_read(void* emu, uint16_t address) {
    if (!emu) return 0;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    if (ctx->initialized && ctx->via) {
        return via_read(ctx->via, address & 0x0F);
    }

    return 0;
}

// Funções LCD compatíveis com a interface Python
EMU6502_API void lcd_16x2_write_data(void* emu, uint8_t data) {
    // Tratar como caractere ASCII
    emu6502_lcd_write_char(emu, (char)data);
}

EMU6502_API void lcd_16x2_write_command(void* emu, uint8_t command) {
    if (!emu) return;

    emu6502_context_t* ctx = (emu6502_context_t*)emu;

    // Processar comandos básicos do LCD
    switch (command) {
        case 0x01: // Clear display
            emu6502_lcd_clear(emu);
            break;
        case 0x02: // Return home
            ctx->lcd_state.cursor_row = 0;
            ctx->lcd_state.cursor_col = 0;
            break;
        case 0x0C: // Display on, cursor off
            ctx->lcd_state.display_on = true;
            ctx->lcd_state.cursor_on = false;
            break;
        case 0x0E: // Display on, cursor on
            ctx->lcd_state.display_on = true;
            ctx->lcd_state.cursor_on = true;
            break;
        case 0x0F: // Display on, cursor on, blink on
            ctx->lcd_state.display_on = true;
            ctx->lcd_state.cursor_on = true;
            ctx->lcd_state.blink_on = true;
            break;
        default:
            // Para comandos de posicionamento do cursor (0x80 + posição)
            if (command >= 0x80) {
                uint8_t pos = command & 0x7F;
                if (pos < 16) {
                    ctx->lcd_state.cursor_row = 0;
                    ctx->lcd_state.cursor_col = pos;
                } else if (pos >= 0x40 && pos < 0x50) {
                    ctx->lcd_state.cursor_row = 1;
                    ctx->lcd_state.cursor_col = pos - 0x40;
                }
            }
            break;
    }
}