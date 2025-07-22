/**
 * @file main.c
 * @brief Função principal do emulador 6502
 * @author Anderson Costa
 * @version 3.0.0
 * @date 2025-01-06
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/emu6502.h"
#include "../include/interface/curses_interface.h"

int main(int argc, char *argv[]) {
    (void)argc; // Suprime warning de parâmetro não usado
    (void)argv; // Suprime warning de parâmetro não usado

    printf("=== Emulador 6502 v3.0.0 ===\n");
    printf("Autor: Anderson Costa\n");
    printf("Data: 2025-01-06\n\n");

    // Configuração padrão do emulador
    emu6502_config_t config = {
        .clock_frequency = 1000000.0,  // 1 MHz
        .decimal_mode = false,
        .debug_mode = false,
        .trace_execution = false,
        .max_instructions = 1000000
    };

    // Cria e inicializa o emulador
    emu6502_t *emu = emu6502_create(&config);
    if (!emu) {
        fprintf(stderr, "Erro: Não foi possível criar o emulador\n");
        return 1;
    }

    emu6502_status_t status = emu6502_init(emu);
    if (status != EMU6502_SUCCESS) {
        fprintf(stderr, "Erro: Falha na inicialização do emulador (status: %d)\n", status);
        emu6502_destroy(emu);
        return 1;
    }

    printf("Emulador inicializado com sucesso!\n");

    // Inicializa interface
    if (curses_interface_init() != 0) {
        fprintf(stderr, "Erro: Falha na inicialização da interface\n");
        emu6502_destroy(emu);
        return 1;
    }

    // Executa o emulador
    int result = curses_interface_run(emu);

    // Limpeza
    curses_interface_cleanup();
    emu6502_destroy(emu);

    printf("Emulador finalizado.\n");
    return result;
}