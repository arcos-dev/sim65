#include "via.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

VIA6522 *via_init(void) {
    VIA6522 *via = (VIA6522 *)calloc(1, sizeof(VIA6522));
    if (via) {
        memset(via->reg, 0, sizeof(via->reg));
        via->t1c = via->t1l = 0;
        via->t2c = via->t2l = 0;
        via->ifr = 0;
        via->ier = 0;
        via->serial_in_head = via->serial_in_tail = 0;
        // SR
        via->shift_reg = 0;
        via->shift_count = 0;
        via->shift_active = false;
        via->shift_mode = VIA_SR_DISABLED;
        via->sr_irq_flag = false;
        via->sr_tx_ready = true;
        via->sr_rx_ready = false;
    }
    return via;
}

void via_destroy(VIA6522 *via) {
    if (via) {
        free(via);
    }
}

void via_serial_feed(VIA6522 *via, const char *str) {
    if (!via || !str) return;
    while (*str) {
        int next_head = (via->serial_in_head + 1) % VIA_SERIAL_IN_BUF_SIZE;
        if (next_head == via->serial_in_tail) break; // buffer cheio
        via->serial_in_buf[via->serial_in_head] = *str++;
        via->serial_in_head = next_head;
    }
}

// Injeta byte recebido no SR (simula RX serial)
void via_serial_rx_byte(VIA6522 *via, uint8_t byte) {
    if (!via) return;
    via->shift_reg = byte;
    via->sr_rx_ready = true;
    via->ifr |= 0x10; // IFR bit 4: SR interrupt
}

// Avança timers e shift register
void via_tick(VIA6522 *via) {
    if (!via) return;
    // Timer 1
    if (via->t1c > 0) {
        via->t1c--;
        if (via->t1c == 0) {
            via->ifr |= 0x40; // Set T1 interrupt flag
        }
    }
    // Timer 2
    if (via->t2c > 0) {
        via->t2c--;
        if (via->t2c == 0) {
            via->ifr |= 0x20; // Set T2 interrupt flag
        }
    }
    // Shift register
    via_tick_serial(via);
}

// Simula clock de serial: desloca 1 bit por chamada
void via_tick_serial(VIA6522 *via) {
    if (!via || !via->shift_active) return;
    // Desloca 1 bit à esquerda (output)
    via->shift_reg <<= 1;
    via->shift_count--;
    if (via->shift_count == 0) {
        via->shift_active = false;
        via->sr_tx_ready = true;
        via->ifr |= 0x10; // IFR bit 4: SR interrupt
        // Exibe byte transmitido
        putchar(via->shift_reg); // Para debug/monitor
    }
}

uint8_t via_read(VIA6522 *via, uint16_t address) {
    if (!via)
        return 0xFF;
    uint16_t reg = (address - VIA_BASE_ADDRESS) & 0x0F;
    switch (reg) {
        case VIA_REG_ORB: {
            uint8_t val = via->reg[VIA_REG_ORB];
            // PB6: serial in
            if (via->serial_in_head != via->serial_in_tail)
                val |= VIA_SERIAL_IN_BIT;
            else
                val &= (uint8_t)~VIA_SERIAL_IN_BIT;
            return val;
        }
        case VIA_REG_ORA: {
            // Se PB6 estiver setado (dado disponível), retorna próximo caractere
            if (via->serial_in_head != via->serial_in_tail) {
                char c = via->serial_in_buf[via->serial_in_tail];
                via->serial_in_tail = (via->serial_in_tail + 1) % VIA_SERIAL_IN_BUF_SIZE;
                return (uint8_t)c;
            } else {
                return via->reg[VIA_REG_ORA];
            }
        }
        case VIA_REG_SR: {
            // Leitura do SR: se sr_rx_ready, retorna byte recebido
            if (via->sr_rx_ready) {
                via->sr_rx_ready = false;
                via->ifr &= (uint8_t)~0x10; // Clear SR interrupt flag
                return via->shift_reg;
            } else {
                return 0x00; // No data
            }
        }
        case VIA_REG_IFR:
            return via->ifr;
        case VIA_REG_IER:
            return via->ier | 0x80;
        default:
            return via->reg[reg];
    }
}

void via_write(VIA6522 *via, uint16_t address, uint8_t value) {
    if (!via) return;
    uint16_t reg = (address - VIA_BASE_ADDRESS) & 0x0F;
    switch (reg) {
        case VIA_REG_ORB: {
            // PB7: serial out
            if (value & VIA_SERIAL_OUT_BIT)
                putchar(via->reg[VIA_REG_ORA]); // Envia valor de ORA
            via->reg[VIA_REG_ORB] = value;
            break;
        }
        case VIA_REG_ORA:
            via->reg[VIA_REG_ORA] = value;
            break;
        case VIA_REG_DDRB:
        case VIA_REG_DDRA:
            via->reg[reg] = value;
            break;
        case VIA_REG_T1CL:
            via->t1l = (via->t1l & 0xFF00) | value;
            break;
        case VIA_REG_T1CH:
            via->t1l = (via->t1l & 0x00FF) | (value << 8);
            via->t1c = via->t1l;
            break;
        case VIA_REG_T2CL:
            via->t2l = (via->t2l & 0xFF00) | value;
            break;
        case VIA_REG_T2CH:
            via->t2l = (via->t2l & 0x00FF) | (value << 8);
            via->t2c = via->t2l;
            break;
        case VIA_REG_SR: {
            // Escrever no SR inicia transmissão
            via->shift_reg = value;
            via->shift_count = 8;
            via->shift_active = true;
            via->sr_tx_ready = false;
            via->ifr &= (uint8_t)~0x10; // Clear SR interrupt flag
            break;
        }
        case VIA_REG_IFR:
            via->ifr &= ~value; // Limpa flags escritas como 1
            break;
        case VIA_REG_IER:
            if (value & 0x80)
                via->ier |= (value & 0x7F);
            else
                via->ier &= ~(value & 0x7F);
            break;
        default:
            via->reg[reg] = value;
            break;
    }
} 