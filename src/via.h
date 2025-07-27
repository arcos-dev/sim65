#ifndef VIA_H
#define VIA_H

#include <stdint.h>
#include <stdbool.h>

#define VIA_BASE_ADDRESS 0x6000
#define VIA_END_ADDRESS  0x600F

// Registradores do VIA 6522
#define VIA_REG_ORB   0x00
#define VIA_REG_ORA   0x01
#define VIA_REG_DDRB  0x02
#define VIA_REG_DDRA  0x03
#define VIA_REG_T1CL  0x04
#define VIA_REG_T1CH  0x05
#define VIA_REG_T1LL  0x06
#define VIA_REG_T1LH  0x07
#define VIA_REG_T2CL  0x08
#define VIA_REG_T2CH  0x09
#define VIA_REG_SR    0x0A
#define VIA_REG_ACR   0x0B
#define VIA_REG_PCR   0x0C
#define VIA_REG_IFR   0x0D
#define VIA_REG_IER   0x0E
#define VIA_REG_ORA2  0x0F // ORA sem latch

#define VIA_SERIAL_OUT_BIT 0x80 // PB7
#define VIA_SERIAL_IN_BIT  0x40 // PB6
#define VIA_SERIAL_IN_BUF_SIZE 256

// Modos do shift register (SR)
typedef enum {
    VIA_SR_DISABLED = 0,
    VIA_SR_OUTPUT_MANUAL = 1,
    VIA_SR_OUTPUT_T1 = 2,
    VIA_SR_OUTPUT_T2 = 3,
    VIA_SR_INPUT_EXTERNAL = 4
} via_sr_mode_t;

// Estrutura do VIA 6522
typedef struct {
    uint8_t reg[16]; // Registradores
    // Timers
    uint16_t t1c, t1l;
    uint16_t t2c, t2l;
    // Flags de interrupção
    uint8_t ifr, ier;
    // Serial virtual (entrada)
    char serial_in_buf[VIA_SERIAL_IN_BUF_SIZE];
    int serial_in_head, serial_in_tail;
    // Serial virtual (saída)
    // Shift register (SR)
    uint8_t shift_reg;
    uint8_t shift_count;
    bool shift_active;
    via_sr_mode_t shift_mode;
    // Flags para indicar byte transmitido/recebido
    bool sr_irq_flag;
    bool sr_tx_ready;
    bool sr_rx_ready;
} VIA6522;

VIA6522 *via_init(void);
void via_destroy(VIA6522 *via);
uint8_t via_read(VIA6522 *via, uint16_t address);
void via_write(VIA6522 *via, uint16_t address, uint8_t value);
void via_tick(VIA6522 *via); // Avança timers e SR
void via_serial_feed(VIA6522 *via, const char *str); // Alimenta buffer de entrada
void via_tick_serial(VIA6522 *via); // Simula clock de serial
void via_serial_rx_byte(VIA6522 *via, uint8_t byte); // Injeta byte recebido no SR

#endif // VIA_H