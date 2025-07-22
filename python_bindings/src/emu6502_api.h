/**
 * EMU6502 API - Interface Python para o emulador 6502
 * ===================================================
 *
 * Este arquivo define a interface C que será exposta para Python
 * através de uma biblioteca compartilhada (DLL no Windows).
 *
 * Integra o emulador sim65 completo com a GUI Python existente.
 *
 * Autor: Anderson Costa
 * Data: 2025-01-21
 */

#ifndef EMU6502_API_H
#define EMU6502_API_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Definir exportação para DLL no Windows
#ifdef _WIN32
    #ifdef EMU6502_EXPORTS
        #define EMU6502_API __declspec(dllexport)
    #else
        #define EMU6502_API __declspec(dllimport)
    #endif
#else
    #define EMU6502_API __attribute__((visibility("default")))
#endif

// Estruturas de configuração e estado
typedef struct {
    double clock_frequency;
    bool decimal_mode;
    bool debug_mode;
    bool trace_execution;
    uint32_t max_instructions;
} emu65_config_t;

typedef struct {
    uint16_t address;
    uint8_t data;
    bool rw; // true = read, false = write
} emu65_bus_state_t;

typedef struct {
    char display[34]; // 2 linhas x 17 chars (16 + null terminator cada)
    uint8_t cursor_row;
    uint8_t cursor_col;
    bool display_on;
    bool cursor_on;
    bool blink_on;
    bool busy;
    uint8_t function_set;
    uint8_t entry_mode;
    uint8_t display_control;
} lcd_16x2_state_t;

typedef struct {
    uint16_t pc;
    uint8_t a, x, y, sp;
    uint8_t status;
    uint64_t cycles;
} cpu_state_t;

typedef struct {
    uint16_t address;
    uint8_t data_direction_a;
    uint8_t data_direction_b;
    uint8_t output_register_a;
    uint8_t output_register_b;
    uint8_t input_register_a;
    uint8_t input_register_b;
} via_state_t;

// Funções principais da API
EMU6502_API void* emu6502_create(const emu65_config_t* config);
EMU6502_API void emu6502_destroy(void* emu);
EMU6502_API int emu6502_init(void* emu);
EMU6502_API int emu6502_reset(void* emu);
EMU6502_API int emu6502_step(void* emu);
EMU6502_API int emu6502_run_cycles(void* emu, uint32_t cycles);

// Funções de carregamento
EMU6502_API int emu6502_load_program(void* emu, const char* data, size_t size, uint16_t address);
EMU6502_API int emu6502_load_rom(void* emu, const char* data, size_t size, uint16_t address);
EMU6502_API int emu6502_load_file(void* emu, const char* filename, uint16_t address);

// Funções de acesso à memória
EMU6502_API uint8_t emu6502_read_byte(void* emu, uint16_t address);
EMU6502_API void emu6502_write_byte(void* emu, uint16_t address, uint8_t value);

// Funções de estado
EMU6502_API void emu6502_get_cpu_state(void* emu, cpu_state_t* state);
EMU6502_API void emu6502_get_bus_state(void* emu, emu65_bus_state_t* state);
EMU6502_API void emu6502_get_via_state(void* emu, via_state_t* state);
EMU6502_API void emu6502_get_lcd_state(void* emu, lcd_16x2_state_t* state);

// Funções do LCD
EMU6502_API void emu6502_lcd_clear(void* emu);
EMU6502_API void emu6502_lcd_write_char(void* emu, char c);
EMU6502_API void emu6502_lcd_write_string(void* emu, const char* str);
EMU6502_API void emu6502_lcd_set_cursor(void* emu, uint8_t row, uint8_t col);

// Funções LCD compatíveis com a interface Python
EMU6502_API void lcd_16x2_write_data(void* emu, uint8_t data);
EMU6502_API void lcd_16x2_write_command(void* emu, uint8_t command);

// Funções de configuração
EMU6502_API void emu6502_set_clock_frequency(void* emu, double freq);
EMU6502_API double emu6502_get_clock_frequency(void* emu);
EMU6502_API void emu6502_set_debug_mode(void* emu, bool enabled);
EMU6502_API bool emu6502_get_debug_mode(void* emu);

// Funções de E/S
EMU6502_API void emu6502_via_write(void* emu, uint16_t address, uint8_t value);
EMU6502_API uint8_t emu6502_via_read(void* emu, uint16_t address);

#ifdef __cplusplus
}
#endif

#endif // EMU6502_API_H
