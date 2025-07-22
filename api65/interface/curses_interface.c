/**
 * @file curses_interface.c
 * @brief Implementação da interface curses para o emulador 6502
 * @author Anderson Costa
 * @version 3.0.0
 * @date 2025-01-06
 */

#include <stdio.h>
#include <stdlib.h>
#include "../../include/interface/curses_interface.h"

int curses_interface_init(void) {
    // Implementação mínima - sem curses por enquanto
    printf("Interface curses inicializada (modo texto)\n");
    return 0;
}

int curses_interface_run(emu6502_t *emu) {
    if (!emu) {
        return -1;
    }

    // Implementação mínima - apenas executa algumas instruções
    printf("Executando emulador 6502...\n");
    
    // Em vez de chamar emu6502_run, vamos executar algumas instruções manualmente
    for (int i = 0; i < 10; i++) {
        emu6502_status_t status = emu6502_step(emu);
        if (status != EMU6502_SUCCESS) {
            printf("Erro ao executar instrução %d: %d\n", i, status);
            return -1;
        }
    }
    
    printf("Execução concluída com sucesso\n");
    return 0;
}

void curses_interface_cleanup(void) {
    printf("Interface curses finalizada\n");
}