/**
 * @file ben_eater_io.c
 * @brief Sistema de I/O do Ben Eater para LCD 16x2
 * @author Anderson Costa
 * @version 3.1.0
 * @date 2025-01-06
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/devices/ben_eater_io.h"
#include "../../include/devices/lcd_16x2.h"

/* ============================================================================
 *                              CONSTANTES
 * ============================================================================ */

#define PORTB_ADDR 0x6000
#define PORTA_ADDR 0x6001
#define DDRB_ADDR  0x6002
#define DDRA_ADDR  0x6003

#define E_BIT  0x80  // %10000000
#define RW_BIT 0x40  // %01000000
#define RS_BIT 0x20  // %00100000

/* ============================================================================
 *                              ESTRUTURA PRINCIPAL
 * ============================================================================ */

struct ben_eater_io {
    // Portos de I/O
    uint8_t portb;      // Porta B (dados)
    uint8_t porta;      // Porta A (controle)
    uint8_t ddrb;       // Data Direction Register B
    uint8_t ddra;       // Data Direction Register A
    
    // Estado anterior para detectar mudanças
    uint8_t prev_portb;
    uint8_t prev_porta;
    
    // LCD conectado
    lcd_16x2_t *lcd;
    
    // Estado de inicialização
    bool initialized;
    
    // Contadores para timing
    uint32_t cycle_count;

    // Dado latched para escrita
    uint8_t latched_data;
};

/* ============================================================================
 *                              FUNÇÕES AUXILIARES
 * ============================================================================ */

static void ben_eater_io_process_lcd_command(ben_eater_io_t *io) {
    if (!io || !io->lcd) return;
    
    // Detecta mudanças nos portos
    bool portb_changed = (io->portb != io->prev_portb);
    bool porta_changed = (io->porta != io->prev_porta);
    
    if (!portb_changed && !porta_changed) return;
    
    // Extrai sinais de controle
    bool e_signal = (io->porta & E_BIT) != 0;
    bool rw_signal = (io->porta & RW_BIT) != 0;
    bool rs_signal = (io->porta & RS_BIT) != 0;
    
    // Detecta borda de descida do E (Enable)
    bool prev_e_signal = (io->prev_porta & E_BIT) != 0;
    bool e_falling_edge = prev_e_signal && !e_signal;
    bool e_rising_edge = !prev_e_signal && e_signal;

    if (e_rising_edge) {
        // Latch do dado na subida de E
        io->latched_data = io->portb;
        printf("[BEN_EATER_IO DEBUG] E rising edge: latched_data=0x%02X ('%c')\n", 
               io->latched_data, (io->latched_data >= 32 && io->latched_data <= 126) ? (char)io->latched_data : '?');
    }

    if (e_falling_edge) {
        printf("[BEN_EATER_IO DEBUG] E falling edge: RS=%d, RW=%d, data=0x%02X\n", rs_signal, rw_signal, io->latched_data);
        
        // Processa comando/dados no LCD
        if (rw_signal) {
            // Leitura - implementa busy flag
            if (rs_signal) {
                // Leitura de dados (não usado no protocolo)
                io->portb = 0x00;
            } else {
                // Leitura de status (busy flag + endereço)
                uint8_t status = lcd_16x2_read_byte(io->lcd, LCD_COMMAND_PORT);
                io->portb = status;
            }
        } else {
            // Escrita
            if (rs_signal) {
                // Dados (caractere)
                printf("[BEN_EATER_IO DEBUG] Calling lcd_16x2_write_data with 0x%02X\n", io->latched_data);
                lcd_16x2_write_data(io->lcd, io->latched_data);
            } else {
                // Comando
                printf("[BEN_EATER_IO DEBUG] Calling lcd_16x2_write_command with 0x%02X\n", io->latched_data);
                lcd_16x2_write_command(io->lcd, io->latched_data);
            }
        }
    }
    
    // Atualiza estado anterior
    io->prev_portb = io->portb;
    io->prev_porta = io->porta;
}

/* ============================================================================
 *                              IMPLEMENTAÇÃO DA API
 * ============================================================================ */

ben_eater_io_t* ben_eater_io_create(void) {
    ben_eater_io_t *io = malloc(sizeof(ben_eater_io_t));
    if (!io) {
        return NULL;
    }
    
    // Inicializa com zeros
    memset(io, 0, sizeof(ben_eater_io_t));
    
    // Cria LCD
    io->lcd = lcd_16x2_create();
    if (!io->lcd) {
        free(io);
        return NULL;
    }
    
    // Inicializa LCD
    if (lcd_16x2_init(io->lcd) != 0) {
        lcd_16x2_destroy(io->lcd);
        free(io);
        return NULL;
    }
    
    // Estado inicial dos portos
    io->portb = 0x00;
    io->porta = 0x00;
    io->ddrb = 0x00;
    io->ddra = 0x00;
    io->prev_portb = 0x00;
    io->prev_porta = 0x00;
    io->initialized = true;
    io->cycle_count = 0;
    io->latched_data = 0;
    
    return io;
}

void ben_eater_io_destroy(ben_eater_io_t *io) {
    if (!io) return;
    
    if (io->lcd) {
        lcd_16x2_destroy(io->lcd);
    }
    
    free(io);
}

int ben_eater_io_init(ben_eater_io_t *io) {
    if (!io) return -1;
    
    // Reset dos portos
    io->portb = 0x00;
    io->porta = 0x00;
    io->ddrb = 0x00;
    io->ddra = 0x00;
    io->prev_portb = 0x00;
    io->prev_porta = 0x00;
    io->cycle_count = 0;
    io->latched_data = 0;
    
    // Reset do LCD
    if (io->lcd) {
        lcd_16x2_reset(io->lcd);
    }
    
    return 0;
}

void ben_eater_io_reset(ben_eater_io_t *io) {
    if (!io) return;
    
    ben_eater_io_init(io);
}

uint8_t ben_eater_io_read_byte(ben_eater_io_t *io, uint16_t address) {
    if (!io) return 0x00;
    
    switch (address) {
        case PORTB_ADDR:
            return io->portb;
        case PORTA_ADDR:
            return io->porta;
        case DDRB_ADDR:
            return io->ddrb;
        case DDRA_ADDR:
            return io->ddra;
        default:
            return 0x00;
    }
}

void ben_eater_io_write_byte(ben_eater_io_t *io, uint16_t address, uint8_t value) {
    if (!io) return;
    
    switch (address) {
        case PORTB_ADDR:
            io->portb = value;
            break;
        case PORTA_ADDR:
            io->porta = value;
            break;
        case DDRB_ADDR:
            io->ddrb = value;
            break;
        case DDRA_ADDR:
            io->ddra = value;
            break;
        default:
            break;
    }
    
    // Processa mudanças no LCD
    ben_eater_io_process_lcd_command(io);
}

void ben_eater_io_cycle(ben_eater_io_t *io) {
    if (!io) return;
    
    io->cycle_count++;
    
    // Processa mudanças no LCD a cada ciclo
    ben_eater_io_process_lcd_command(io);
}

void ben_eater_io_get_lcd_state(ben_eater_io_t *io, lcd_16x2_state_t *lcd_state) {
    if (!io || !io->lcd || !lcd_state) return;
    
    lcd_16x2_get_state(io->lcd, lcd_state);
}

void ben_eater_io_get_state(ben_eater_io_t *io, ben_eater_io_state_t *state) {
    if (!io || !state) return;
    
    state->portb = io->portb;
    state->porta = io->porta;
    state->ddrb = io->ddrb;
    state->ddra = io->ddra;
    state->cycle_count = io->cycle_count;
    
    // Estado do LCD
    if (io->lcd) {
        lcd_16x2_get_state(io->lcd, &state->lcd_state);
    } else {
        memset(&state->lcd_state, 0, sizeof(lcd_16x2_state_t));
    }
}

bool ben_eater_io_is_initialized(ben_eater_io_t *io) {
    return io && io->initialized;
} 