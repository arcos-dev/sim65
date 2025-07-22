/**
 * @file ben_eater_bus.c
 * @brief Barramento específico para o projeto Ben Eater
 * @author Anderson Costa
 * @version 3.0.0
 * @date 2025-01-06
 */

#include <stdlib.h>
#include <string.h>
#include "../../include/devices/ben_eater_bus.h"
#include "../../include/devices/via_6522.h"
#include <stdio.h>

/* ============================================================================
 *                              ESTRUTURA INTERNA
 * ============================================================================ */

struct ben_eater_bus {
    // Dispositivos conectados
    via_6522_t *via;

    // Memória
    uint8_t ram[256];      // RAM 256 bytes (0x0000-0x00FF)
    uint8_t rom[1024];     // ROM 1KB (0x8000-0x83FF)

    // Estado do barramento
    uint16_t address_bus;
    uint8_t data_bus;
    bool read_write;       // true = read, false = write

    // Callbacks
    ben_eater_bus_update_callback_t update_callback;
    void *callback_user_data;

    // Estado interno
    bool initialized;
};

/* ============================================================================
 *                              FUNÇÕES AUXILIARES
 * ============================================================================ */

static void ben_eater_bus_notify_update(ben_eater_bus_t *bus) {
    if (bus && bus->update_callback) {
        bus->update_callback(bus, bus->callback_user_data);
    }
}

static uint8_t ben_eater_bus_read_ram(ben_eater_bus_t *bus, uint16_t address) {
    return bus->ram[address & 0xFF];
}

static void ben_eater_bus_write_ram(ben_eater_bus_t *bus, uint16_t address, uint8_t data) {
    bus->ram[address & 0xFF] = data;
}

static uint8_t ben_eater_bus_read_rom(ben_eater_bus_t *bus, uint16_t address) {
    // Tratar 0xFFFC e 0xFFFD como espelho dos dois últimos bytes da ROM
    if (address == 0xFFFC) {
        return bus->rom[1022]; // penúltimo byte da ROM
    } else if (address == 0xFFFD) {
        return bus->rom[1023]; // último byte da ROM
    }
    // ROM normal (1KB, 0x8000-0x83FF)
    return bus->rom[address & 0x3FF];
}

static uint8_t ben_eater_bus_read_via(ben_eater_bus_t *bus, uint16_t address) {
    return via_6522_read_byte(bus->via, address & 0x0F);
}

static void ben_eater_bus_write_via(ben_eater_bus_t *bus, uint16_t address, uint8_t data) {
    via_6522_write_byte(bus->via, address & 0x0F, data);
}

// Callback adapter para o VIA
static void via_update_callback_adapter(via_6522_t *via, void *user_data) {
    (void)via;
    ben_eater_bus_t *bus = (ben_eater_bus_t*)user_data;
    ben_eater_bus_notify_update(bus);
}

/* ============================================================================
 *                              IMPLEMENTAÇÃO DA API
 * ============================================================================ */

ben_eater_bus_t* ben_eater_bus_create(void) {
    ben_eater_bus_t *bus = malloc(sizeof(ben_eater_bus_t));
    if (!bus) {
        return NULL;
    }

    // Inicializa estrutura
    memset(bus, 0, sizeof(ben_eater_bus_t));
    bus->initialized = false;

    // Cria VIA
    bus->via = via_6522_create();
    if (!bus->via) {
        free(bus);
        return NULL;
    }

    return bus;
}

void ben_eater_bus_destroy(ben_eater_bus_t *bus) {
    if (bus) {
        if (bus->via) {
            via_6522_destroy(bus->via);
        }
        free(bus);
    }
}

ben_eater_bus_status_t ben_eater_bus_init(ben_eater_bus_t *bus) {
    if (!bus) {
        return BEN_EATER_BUS_ERROR_INVALID_ARGUMENT;
    }

    // Inicializa RAM
    memset(bus->ram, 0, sizeof(bus->ram));

    // Inicializa ROM
    memset(bus->rom, 0, sizeof(bus->rom));

    // Inicializa VIA
    via_6522_status_t via_status = via_6522_init(bus->via);
    if (via_status != VIA_6522_SUCCESS) {
        return BEN_EATER_BUS_ERROR_DEVICE_INIT_FAILED;
    }

    // Configura callback do VIA para notificar atualizações do barramento
    via_6522_register_update_callback(bus->via, via_update_callback_adapter, bus);

    bus->initialized = true;
    return BEN_EATER_BUS_SUCCESS;
}

void ben_eater_bus_reset(ben_eater_bus_t *bus) {
    if (!bus) {
        return;
    }

    // Reseta RAM
    memset(bus->ram, 0, sizeof(bus->ram));

    // Reseta VIA
    via_6522_reset(bus->via);

    // Reseta estado do barramento
    bus->address_bus = 0x0000;
    bus->data_bus = 0x00;
    bus->read_write = true;
}

uint8_t ben_eater_bus_read_byte(ben_eater_bus_t *bus, uint16_t address) {
    if (!bus || !bus->initialized) {
        return 0xFF;
    }

    bus->address_bus = address;
    bus->read_write = true;

    uint8_t data = 0xFF;

    if (address <= 0x00FF) {
        // RAM (256 bytes)
        data = ben_eater_bus_read_ram(bus, address);
    } else if ((address >= 0x8000 && address <= 0x83FF) || address == 0xFFFC || address == 0xFFFD) {
        // ROM (1KB) + vetores de reset
        data = ben_eater_bus_read_rom(bus, address);
    } else if (0x6000 <= address && address <= 0x600F) {
        // VIA 6522
        data = ben_eater_bus_read_via(bus, address);
    } else {
        // Endereço inválido
        data = 0xFF;
    }

    bus->data_bus = data;
    ben_eater_bus_notify_update(bus);

    return data;
}

void ben_eater_bus_write_byte(ben_eater_bus_t *bus, uint16_t address, uint8_t data) {
    if (!bus || !bus->initialized) {
        return;
    }

    printf("[BEN_EATER_BUS DEBUG] Write: addr=0x%04X, data=0x%02X\n", address, data);

    bus->address_bus = address;
    bus->data_bus = data;
    bus->read_write = false;

    if (address <= 0x00FF) {
        // RAM (256 bytes)
        ben_eater_bus_write_ram(bus, address, data);
    } else if (0x6000 <= address && address <= 0x600F) {
        // VIA 6522
        printf("[BEN_EATER_BUS DEBUG] -> VIA 6522 write\n");
        ben_eater_bus_write_via(bus, address, data);
    } else if (address == 0xFFFC || address == 0xFFFD) {
        // Vetores de reset são graváveis
        if (address == 0xFFFC) {
            bus->rom[1022] = data; // penúltimo byte da ROM
        } else {
            bus->rom[1023] = data; // último byte da ROM
        }
    }
    // ROM normal não é gravável

    ben_eater_bus_notify_update(bus);
}

void ben_eater_bus_load_rom(ben_eater_bus_t *bus, const uint8_t *data, size_t size, uint16_t start_address) {
    if (!bus || !data) {
        return;
    }

    for (size_t i = 0; i < size; i++) {
        uint16_t addr = start_address + i;
        if (0x8000 <= addr && addr <= 0x83FF) {
            bus->rom[addr & 0x3FF] = data[i];
        }
    }
}

void ben_eater_bus_get_ram_dump(ben_eater_bus_t *bus, uint8_t *buffer, size_t buffer_size) {
    if (!bus || !buffer) {
        return;
    }

    size_t copy_size = (buffer_size < sizeof(bus->ram)) ? buffer_size : sizeof(bus->ram);
    memcpy(buffer, bus->ram, copy_size);
}

void ben_eater_bus_get_rom_dump(ben_eater_bus_t *bus, uint8_t *buffer, size_t buffer_size) {
    if (!bus || !buffer) {
        return;
    }

    size_t copy_size = (buffer_size < sizeof(bus->rom)) ? buffer_size : sizeof(bus->rom);
    memcpy(buffer, bus->rom, copy_size);
}

void ben_eater_bus_get_state(ben_eater_bus_t *bus, ben_eater_bus_state_t *state) {
    if (!bus || !state) {
        return;
    }

    state->address_bus = bus->address_bus;
    state->data_bus = bus->data_bus;
    state->read_write = bus->read_write;

    // Obtém estado do LCD através do VIA
    lcd_16x2_t *lcd = via_6522_get_lcd(bus->via);
    if (lcd) {
        lcd_16x2_get_state(lcd, &state->lcd_state);
    } else {
        memset(&state->lcd_state, 0, sizeof(lcd_16x2_state_t));
    }
}

lcd_16x2_t* ben_eater_bus_get_lcd(ben_eater_bus_t *bus) {
    return bus ? via_6522_get_lcd(bus->via) : NULL;
}

void ben_eater_bus_register_update_callback(ben_eater_bus_t *bus,
                                           ben_eater_bus_update_callback_t callback,
                                           void *user_data) {
    if (bus) {
        bus->update_callback = callback;
        bus->callback_user_data = user_data;
    }
}