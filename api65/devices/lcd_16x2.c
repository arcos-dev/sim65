/**
 * @file lcd_16x2.c
 * @brief Implementação do dispositivo LCD 16x2 (HD44780)
 * @author Anderson Costa
 * @version 3.0.0
 * @date 2025-01-06
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../include/devices/lcd_16x2.h"

/* ============================================================================
 *                              ESTRUTURA INTERNA
 * ============================================================================ */

struct lcd_16x2 {
    // Estado do display
    char display[LCD_16X2_ROWS][LCD_16X2_COLS + 1];
    uint8_t cursor_row;
    uint8_t cursor_col;

    // Controle do display
    bool display_on;
    bool cursor_on;
    bool blink_on;
    bool busy;

    // Registradores internos
    uint8_t function_set;
    uint8_t entry_mode;
    uint8_t display_control;

    // DDRAM (Display Data RAM)
    char ddram[LCD_16X2_BUFFER_SIZE];
    uint8_t ddram_addr;

    // Callback para atualização
    lcd_16x2_update_callback_t update_callback;
    void *callback_user_data;

    // Estado interno
    bool initialized;
    uint8_t last_command;
    bool expecting_data;
};

/* ============================================================================
 *                              FUNÇÕES AUXILIARES
 * ============================================================================ */

static void lcd_16x2_clear_display(lcd_16x2_t *lcd) {
    memset(lcd->display, ' ', sizeof(lcd->display));
    for (int i = 0; i < LCD_16X2_ROWS; i++) {
        lcd->display[i][LCD_16X2_COLS] = '\0';
    }
    lcd->cursor_row = 0;
    lcd->cursor_col = 0;
    lcd->ddram_addr = 0;
}

static void lcd_16x2_return_home(lcd_16x2_t *lcd) {
    lcd->cursor_row = 0;
    lcd->cursor_col = 0;
    lcd->ddram_addr = 0;
}

static void lcd_16x2_process_command(lcd_16x2_t *lcd, uint8_t command) {
    lcd->busy = true;

    switch (command & 0xF0) {
        case LCD_CLEAR_DISPLAY:
            lcd_16x2_clear_display(lcd);
            printf("[LCD DEBUG] Clear Display\n");
            break;

        case LCD_RETURN_HOME:
            lcd_16x2_return_home(lcd);
            printf("[LCD DEBUG] Return Home\n");
            break;

        case LCD_ENTRY_MODE_SET:
            lcd->entry_mode = command;
            printf("[LCD DEBUG] Entry Mode Set: 0x%02X\n", command);
            break;

        case LCD_DISPLAY_CONTROL:
            lcd->display_control = command;
            lcd->display_on = (command & LCD_DISPLAY_ON) != 0;
            lcd->cursor_on = (command & LCD_CURSOR_ON) != 0;
            lcd->blink_on = (command & LCD_BLINK_ON) != 0;
            printf("[LCD DEBUG] Display Control: 0x%02X (ON=%d, CURSOR=%d, BLINK=%d)\n", 
                   command, lcd->display_on, lcd->cursor_on, lcd->blink_on);
            break;

        case LCD_CURSOR_SHIFT:
            // Implementação básica de shift
            printf("[LCD DEBUG] Cursor Shift: 0x%02X\n", command);
            break;

        case LCD_FUNCTION_SET:
            lcd->function_set = command;
            printf("[LCD DEBUG] Function Set: 0x%02X\n", command);
            break;

        case LCD_SET_CGRAM_ADDR:
            // CGRAM não implementado por simplicidade
            printf("[LCD DEBUG] Set CGRAM Addr: 0x%02X\n", command);
            break;

        case LCD_SET_DDRAM_ADDR:
            lcd->ddram_addr = command & 0x7F;
            lcd->cursor_row = lcd->ddram_addr / LCD_16X2_COLS;
            lcd->cursor_col = lcd->ddram_addr % LCD_16X2_COLS;
            printf("[LCD DEBUG] Set DDRAM Addr: 0x%02X (row=%d, col=%d)\n", 
                   command, lcd->cursor_row, lcd->cursor_col);
            break;

        default:
            // Comando não reconhecido
            printf("[LCD DEBUG] Unknown command: 0x%02X\n", command);
            break;
    }

    lcd->busy = false;

    // Chama callback se registrado
    if (lcd->update_callback) {
        lcd->update_callback(lcd, lcd->callback_user_data);
    }
}

/* ============================================================================
 *                              IMPLEMENTAÇÃO DA API
 * ============================================================================ */

lcd_16x2_t* lcd_16x2_create(void) {
    lcd_16x2_t *lcd = malloc(sizeof(lcd_16x2_t));
    if (!lcd) {
        return NULL;
    }

    // Inicializa estrutura
    memset(lcd, 0, sizeof(lcd_16x2_t));
    lcd->initialized = false;

    return lcd;
}

void lcd_16x2_destroy(lcd_16x2_t *lcd) {
    if (lcd) {
        free(lcd);
    }
}

lcd_16x2_status_t lcd_16x2_init(lcd_16x2_t *lcd) {
    if (!lcd) {
        return LCD_16X2_ERROR_INVALID_ARGUMENT;
    }

    // Inicializa display
    lcd_16x2_clear_display(lcd);

    // Configurações padrão
    lcd->display_on = true;
    lcd->cursor_on = false;
    lcd->blink_on = false;
    lcd->busy = false;
    lcd->function_set = 0x38; // 8-bit, 2 lines, 5x8 font
    lcd->entry_mode = 0x06;   // Increment cursor, no shift
    lcd->display_control = 0x0C; // Display on, cursor off, blink off

    lcd->initialized = true;
    return LCD_16X2_SUCCESS;
}

void lcd_16x2_reset(lcd_16x2_t *lcd) {
    if (!lcd) {
        return;
    }

    lcd_16x2_clear_display(lcd);
    lcd->cursor_row = 0;
    lcd->cursor_col = 0;
    lcd->ddram_addr = 0;
    lcd->busy = false;
    lcd->expecting_data = false;
}

uint8_t lcd_16x2_read_byte(lcd_16x2_t *lcd, uint16_t address) {
    if (!lcd || !lcd->initialized) {
        return 0xFF;
    }

    switch (address) {
        case LCD_DATA_PORT:
            // Retorna o último comando ou status
            return lcd->last_command;

        case LCD_COMMAND_PORT:
            // Retorna status (busy flag + endereço)
            return (lcd->busy ? 0x80 : 0x00) | (lcd->ddram_addr & 0x7F);

        default:
            return 0xFF;
    }
}

void lcd_16x2_write_byte(lcd_16x2_t *lcd, uint16_t address, uint8_t value) {
    if (!lcd || !lcd->initialized) {
        printf("[LCD DEBUG] Write byte failed: not initialized\n");
        return;
    }

    printf("[LCD DEBUG] Write byte: addr=0x%04X, value=0x%02X\n", address, value);

    switch (address) {
        case LCD_DATA_PORT:
            printf("[LCD DEBUG] LCD_DATA_PORT write: expecting_data=%d\n", lcd->expecting_data);
            if (lcd->expecting_data) {
                // Dados para o display
                printf("[LCD DEBUG] Writing data to display\n");
                // Avança cursor
                lcd->cursor_col++;
                lcd->ddram_addr++;

                // Quebra de linha se necessário
                if (lcd->cursor_col >= LCD_16X2_COLS) {
                    lcd->cursor_col = 0;
                    lcd->cursor_row++;
                    if (lcd->cursor_row >= LCD_16X2_ROWS) {
                        lcd->cursor_row = 0; // Wrap around
                    }
                }
                lcd->display[lcd->cursor_row][lcd->cursor_col] = (char)value;
                lcd->ddram[lcd->ddram_addr - 1] = (char)value;
                lcd->expecting_data = false;
                printf("[LCD DEBUG] Data written: '%c' at (%d,%d)\n", (char)value, lcd->cursor_row, lcd->cursor_col);
            } else {
                // Comando
                printf("[LCD DEBUG] Processing command\n");
                lcd->last_command = value;
                lcd_16x2_process_command(lcd, value);
            }
            break;

        case LCD_COMMAND_PORT:
            // Comando
            printf("[LCD DEBUG] LCD_COMMAND_PORT write: processing command\n");
            lcd->last_command = value;
            lcd_16x2_process_command(lcd, value);
            break;

        default:
            // Endereço inválido
            printf("[LCD DEBUG] Invalid address: 0x%04X\n", address);
            break;
    }
}

void lcd_16x2_get_state(lcd_16x2_t *lcd, lcd_16x2_state_t *state) {
    if (!lcd || !state) {
        return;
    }

    memcpy(state->display, lcd->display, sizeof(lcd->display));
    state->cursor_row = lcd->cursor_row;
    state->cursor_col = lcd->cursor_col;
    state->display_on = lcd->display_on;
    state->cursor_on = lcd->cursor_on;
    state->blink_on = lcd->blink_on;
    state->busy = lcd->busy;
    state->function_set = lcd->function_set;
    state->entry_mode = lcd->entry_mode;
    state->display_control = lcd->display_control;
}

int lcd_16x2_get_display_text(lcd_16x2_t *lcd, char *buffer, size_t buffer_size) {
    if (!lcd || !buffer || buffer_size == 0) {
        return 0;
    }

    int total_chars = 0;
    for (int row = 0; row < LCD_16X2_ROWS; row++) {
        size_t row_len = strlen(lcd->display[row]);
        if ((size_t)(total_chars + row_len + 1) < buffer_size) {
            strcpy(buffer + total_chars, lcd->display[row]);
            total_chars += row_len;
            if (row < LCD_16X2_ROWS - 1) {
                buffer[total_chars++] = '\n';
            }
        }
    }

    buffer[total_chars] = '\0';
    return total_chars;
}

bool lcd_16x2_is_busy(lcd_16x2_t *lcd) {
    return lcd ? lcd->busy : false;
}

void lcd_16x2_register_update_callback(lcd_16x2_t *lcd,
                                      lcd_16x2_update_callback_t callback,
                                      void *user_data) {
    if (lcd) {
        lcd->update_callback = callback;
        lcd->callback_user_data = user_data;
    }
}

EMU6502_API void lcd_16x2_write_command(lcd_16x2_t *lcd, uint8_t command) {
    printf("[LCD DEBUG] lcd_16x2_write_command chamado: 0x%02X\n", command);
    if (!lcd || !lcd->initialized) {
        return;
    }

    lcd->last_command = command;
    lcd_16x2_process_command(lcd, command);
}

EMU6502_API void lcd_16x2_write_data(lcd_16x2_t *lcd, uint8_t data) {
    printf("[LCD DEBUG] lcd_16x2_write_data chamado: 0x%02X ('%c')\n", data, (data >= 32 && data <= 126) ? (char)data : '?');
    if (!lcd || !lcd->initialized) {
        printf("[LCD DEBUG] Write data failed: not initialized\n");
        return;
    }

    printf("[LCD DEBUG] Write data: 0x%02X ('%c') at (%d,%d)\n", 
           data, (data >= 32 && data <= 126) ? (char)data : '?', 
           lcd->cursor_row, lcd->cursor_col);

    // Escreve o caractere na posição atual do cursor
    if (lcd->cursor_row < LCD_16X2_ROWS && lcd->cursor_col < LCD_16X2_COLS) {
        lcd->display[lcd->cursor_row][lcd->cursor_col] = (char)data;
        lcd->ddram[lcd->ddram_addr] = (char)data;
        printf("[LCD DEBUG] Character written to display[%d][%d] = '%c'\n", 
               lcd->cursor_row, lcd->cursor_col, (char)data);
    } else {
        printf("[LCD DEBUG] Invalid cursor position: (%d,%d)\n", lcd->cursor_row, lcd->cursor_col);
    }

    // Avança cursor
    lcd->cursor_col++;
    lcd->ddram_addr++;

    // Quebra de linha se necessário
    if (lcd->cursor_col >= LCD_16X2_COLS) {
        lcd->cursor_col = 0;
        lcd->cursor_row++;
        if (lcd->cursor_row >= LCD_16X2_ROWS) {
            lcd->cursor_row = 0; // Wrap around
        }
        printf("[LCD DEBUG] Cursor wrapped to (%d,%d)\n", lcd->cursor_row, lcd->cursor_col);
    }

    // Chama callback se registrado
    if (lcd->update_callback) {
        lcd->update_callback(lcd, lcd->callback_user_data);
    }
}