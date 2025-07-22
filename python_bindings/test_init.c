#include <stdio.h>
#include "../src/via.h"
#include "../src/acia.h"
#include "../src/tia.h"
#include "../src/bus.h"
#include "../src/cpu.h"

int main() {
    printf("Testando inicialização dos componentes...\n");

    // Testar VIA
    printf("1. Testando VIA...\n");
    VIA6522* via = via_init();
    if (!via) {
        printf("   ERRO: Falha ao inicializar VIA\n");
        return 1;
    }
    printf("   VIA: OK\n");

    // Testar ACIA
    printf("2. Testando ACIA...\n");
    Acia6550* acia = acia_init();
    if (!acia) {
        printf("   ERRO: Falha ao inicializar ACIA\n");
        via_destroy(via);
        return 1;
    }
    printf("   ACIA: OK\n");

    // Testar TIA
    printf("3. Testando TIA...\n");
    TIA* tia = tia_init(TV_SYSTEM_NTSC);
    if (!tia) {
        printf("   ERRO: Falha ao inicializar TIA\n");
        via_destroy(via);
        acia_destroy(acia);
        return 1;
    }
    printf("   TIA: OK\n");

    // Testar BUS
    printf("4. Testando BUS...\n");
    bus_t bus;
    int result = bus_init(&bus, 65536, 1000000.0, acia, tia);
    if (result != 0) {
        printf("   ERRO: Falha ao inicializar BUS (código: %d)\n", result);
        via_destroy(via);
        acia_destroy(acia);
        tia_destroy(tia);
        return 1;
    }
    printf("   BUS: OK\n");

    // Testar CPU
    printf("5. Testando CPU...\n");
    result = cpu6502_init(&bus);
    if (result != 0) {
        printf("   ERRO: Falha ao inicializar CPU (código: %d)\n", result);
        bus_destroy(&bus);
        via_destroy(via);
        acia_destroy(acia);
        tia_destroy(tia);
        return 1;
    }
    printf("   CPU: OK\n");

    printf("Todos os componentes inicializados com sucesso!\n");

    // Limpar
    cpu6502_destroy();
    bus_destroy(&bus);
    via_destroy(via);
    acia_destroy(acia);
    tia_destroy(tia);

    return 0;
}
