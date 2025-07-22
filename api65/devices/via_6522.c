/**
 * @file via_6522.c
 * @brief Implementação do chip VIA 6522 (Versatile Interface Adapter)
 * @author Anderson Costa
 * @version 1.0.0
 * @date 2025-01-06
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../include/devices/via_6522.h"
#include "../../include/devices/lcd_16x2.h"

/* ============================================================================
 *                              ESTRUTURA INTERNA
 * ============================================================================ */

struct via_6522 {
    // Registradores do VIA
    uint8_t orb;            // Output Register B
    uint8_t ora;            // Output Register A
    uint8_t ddrb;           // Data Direction Register B
    uint8_t ddra;           // Data Direction Register A
    uint8_t t1cl;           // Timer 1 Counter Low
    uint8_t t1ch;           // Timer 1 Counter High
    uint8_t t1ll;           // Timer 1 Latch Low
    uint8_t t1lh;           // Timer 1 Latch High
    uint8_t t2cl;           // Timer 2 Counter Low
    uint8_t t2ch;           // Timer 2 Counter High
    uint8_t sr;             // Shift Register
    uint8_t acr;            // Auxiliary Control Register
    uint8_t pcr;            // Peripheral Control Register
    uint8_t ifr;            // Interrupt Flag Register
    uint8_t ier;            // Interrupt Enable Register
    uint8_t oranh;          // Output Register A (no handshake)

    // Estado anterior para detectar mudanças
    uint8_t prev_orb;
    uint8_t prev_ora;

    // LCD conectado
    lcd_16x2_t *lcd;

    // Callback para notificar mudanças
    via_6522_update_callback_t update_callback;
    void *callback_user_data;

    // Estado interno
    bool initialized;
    uint32_t cycle_count;
};

/* ============================================================================
 *                              FUNÇÕES AUXILIARES
 * ============================================================================ */

static void via_6522_notify_update(via_6522_t *via) {
    if (via && via->update_callback) {
        via->update_callback(via, via->callback_user_data);
    }
}

static void via_6522_process_lcd_protocol(via_6522_t *via) {
    if (!via) return;
    
    printf("[VIA DEBUG] via_6522_process_lcd_protocol: via->lcd=%p\n", (void*)via->lcd);
    if (!via->lcd) return;
    
    // Detecta borda de descida do E (Enable)
    bool e_signal = (via->ora & VIA_E_BIT) != 0;
    bool prev_e_signal = (via->prev_ora & VIA_E_BIT) != 0;
    bool e_falling_edge = prev_e_signal && !e_signal;
    
    printf("[VIA DEBUG] E signal: prev=%d, curr=%d, falling_edge=%d\n", 
           prev_e_signal, e_signal, e_falling_edge);
    printf("[VIA DEBUG] PORTA: prev=0x%02X, curr=0x%02X\n", via->prev_ora, via->ora);
    
    if (e_falling_edge) {
        // Extrai sinais de controle
        bool rw_signal = (via->ora & VIA_RW_BIT) != 0;
        bool rs_signal = (via->ora & VIA_RS_BIT) != 0;
        
        printf("[VIA DEBUG] Falling edge detected! RW=%d, RS=%d, PORTB=0x%02X\n", 
               rw_signal, rs_signal, via->orb);
        
        if (!rw_signal) {  // Write operation
            if (rs_signal) {
                // Write data
                printf("[VIA DEBUG] Calling lcd_16x2_write_data with 0x%02X\n", via->orb);
                lcd_16x2_write_data(via->lcd, via->orb);
            } else {
                // Write command
                printf("[VIA DEBUG] Calling lcd_16x2_write_command with 0x%02X\n", via->orb);
                lcd_16x2_write_command(via->lcd, via->orb);
            }
        }
    }
    
    // Atualiza prev_ora para próxima iteração
    via->prev_ora = via->ora;
}

/* ============================================================================
 *                              IMPLEMENTAÇÃO DA API
 * ============================================================================ */

via_6522_t* via_6522_create(void) {
    via_6522_t *via = malloc(sizeof(via_6522_t));
    if (!via) {
        return NULL;
    }

    // Inicializa estrutura
    memset(via, 0, sizeof(via_6522_t));
    via->initialized = false;

    // Cria LCD
    via->lcd = lcd_16x2_create();
    if (!via->lcd) {
        free(via);
        return NULL;
    }

    return via;
}

void via_6522_destroy(via_6522_t *via) {
    if (via) {
        if (via->lcd) {
            lcd_16x2_destroy(via->lcd);
        }
        free(via);
    }
}

via_6522_status_t via_6522_init(via_6522_t *via) {
    if (!via) {
        return VIA_6522_ERROR_INVALID_ARGUMENT;
    }

    // Inicializa registradores
    via->orb = 0x00;
    via->ora = 0x00;
    via->ddrb = 0x00;
    via->ddra = 0x00;
    via->t1cl = 0x00;
    via->t1ch = 0x00;
    via->t1ll = 0x00;
    via->t1lh = 0x00;
    via->t2cl = 0x00;
    via->t2ch = 0x00;
    via->sr = 0x00;
    via->acr = 0x00;
    via->pcr = 0x00;
    via->ifr = 0x00;
    via->ier = 0x00;
    via->oranh = 0x00;

    // Estado anterior
    via->prev_orb = 0x00;
    via->prev_ora = 0x00;

    // Inicializa LCD
    lcd_16x2_status_t lcd_status = lcd_16x2_init(via->lcd);
    if (lcd_status != LCD_16X2_SUCCESS) {
        return VIA_6522_ERROR_INVALID_ARGUMENT;
    }

    via->initialized = true;
    return VIA_6522_SUCCESS;
}

void via_6522_reset(via_6522_t *via) {
    if (!via) return;

    // Reseta registradores
    via->orb = 0x00;
    via->ora = 0x00;
    via->ddrb = 0x00;
    via->ddra = 0x00;
    via->t1cl = 0x00;
    via->t1ch = 0x00;
    via->t1ll = 0x00;
    via->t1lh = 0x00;
    via->t2cl = 0x00;
    via->t2ch = 0x00;
    via->sr = 0x00;
    via->acr = 0x00;
    via->pcr = 0x00;
    via->ifr = 0x00;
    via->ier = 0x00;
    via->oranh = 0x00;

    // Estado anterior
    via->prev_orb = 0x00;
    via->prev_ora = 0x00;

    // Reseta LCD
    lcd_16x2_reset(via->lcd);
}

uint8_t via_6522_read_byte(via_6522_t *via, uint16_t address) {
    if (!via || !via->initialized) {
        return 0xFF;
    }

    uint8_t data = 0xFF;

    switch (address & 0x0F) {
        case VIA_ORB:
            data = via->orb;
            break;
        case VIA_ORA:
            data = via->ora;
            break;
        case VIA_DDRB:
            data = via->ddrb;
            break;
        case VIA_DDRA:
            data = via->ddra;
            break;
        case VIA_T1CL:
            data = via->t1cl;
            break;
        case VIA_T1CH:
            data = via->t1ch;
            break;
        case VIA_T1LL:
            data = via->t1ll;
            break;
        case VIA_T1LH:
            data = via->t1lh;
            break;
        case VIA_T2CL:
            data = via->t2cl;
            break;
        case VIA_T2CH:
            data = via->t2ch;
            break;
        case VIA_SR:
            data = via->sr;
            break;
        case VIA_ACR:
            data = via->acr;
            break;
        case VIA_PCR:
            data = via->pcr;
            break;
        case VIA_IFR:
            data = via->ifr;
            break;
        case VIA_IER:
            data = via->ier;
            break;
        case VIA_ORANH:
            data = via->oranh;
            break;
        default:
            data = 0xFF;
            break;
    }

    printf("[VIA DEBUG] Read: addr=0x%02X, data=0x%02X\n", address & 0x0F, data);
    return data;
}

void via_6522_write_byte(via_6522_t *via, uint16_t address, uint8_t value) {
    if (!via || !via->initialized) {
        return;
    }

    printf("[VIA DEBUG] Write: addr=0x%02X, value=0x%02X\n", address & 0x0F, value);
    if ((address & 0x0F) == VIA_ORA) {
        printf("[VIA DEBUG] *** Escrita em PORTA ($6001): 0x%02X\n", value);
    }

    switch (address & 0x0F) {
        case VIA_ORB:
            via->orb = value;
            break;
        case VIA_ORA:
            via->ora = value;
            break;
        case VIA_DDRB:
            via->ddrb = value;
            break;
        case VIA_DDRA:
            via->ddra = value;
            break;
        case VIA_T1CL:
            via->t1cl = value;
            break;
        case VIA_T1CH:
            via->t1ch = value;
            break;
        case VIA_T1LL:
            via->t1ll = value;
            break;
        case VIA_T1LH:
            via->t1lh = value;
            break;
        case VIA_T2CL:
            via->t2cl = value;
            break;
        case VIA_T2CH:
            via->t2ch = value;
            break;
        case VIA_SR:
            via->sr = value;
            break;
        case VIA_ACR:
            via->acr = value;
            break;
        case VIA_PCR:
            via->pcr = value;
            break;
        case VIA_IFR:
            via->ifr = value;
            break;
        case VIA_IER:
            via->ier = value;
            break;
        case VIA_ORANH:
            via->oranh = value;
            break;
        default:
            break;
    }

    // Processa protocolo LCD se necessário
    via_6522_process_lcd_protocol(via);

    // Notifica mudanças
    via_6522_notify_update(via);
}

void via_6522_get_state(via_6522_t *via, via_6522_state_t *state) {
    if (!via || !state) return;

    state->portb = via->orb;
    state->porta = via->ora;
    state->ddrb = via->ddrb;
    state->ddra = via->ddra;
    state->t1cl = via->t1cl;
    state->t1ch = via->t1ch;
    state->t1ll = via->t1ll;
    state->t1lh = via->t1lh;
    state->t2cl = via->t2cl;
    state->t2ch = via->t2ch;
    state->sr = via->sr;
    state->acr = via->acr;
    state->pcr = via->pcr;
    state->ifr = via->ifr;
    state->ier = via->ier;
    state->oranh = via->oranh;
}

void via_6522_register_update_callback(via_6522_t *via, 
                                      via_6522_update_callback_t callback,
                                      void *user_data) {
    if (!via) return;

    via->update_callback = callback;
    via->callback_user_data = user_data;
}

void via_6522_cycle(via_6522_t *via) {
    if (!via) return;

    via->cycle_count++;

    // Processa protocolo LCD a cada ciclo
    via_6522_process_lcd_protocol(via);
}

bool via_6522_is_initialized(via_6522_t *via) {
    return via && via->initialized;
}

uint8_t via_6522_get_portb(via_6522_t *via) {
    return via ? via->orb : 0x00;
}

uint8_t via_6522_get_porta(via_6522_t *via) {
    return via ? via->ora : 0x00;
}

lcd_16x2_t* via_6522_get_lcd(via_6522_t *via) {
    return via ? via->lcd : NULL;
} 