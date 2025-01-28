/*******************************************************************************
 * acia.h
 *
 * Asynchronous Communications Interface Adapter (ACIA) 6550/6551 Emulation
 *
 * This header defines the interface for the ACIA 6550/6551 emulation module.
 * It provides functions to initialize, reset, destroy, and interact with the
 * ACIA registers, as well as processing transmit and receive operations.
 *
 * Author: Anderson Costa
 * Date: 2025-01-04
 ******************************************************************************/

#ifndef ACIA_H
#define ACIA_H

#include <stdbool.h>
#include <stdint.h>

// ACIA Memory Map
#define ACIA_START_ADDRESS 0xD000
#define ACIA_END_ADDRESS   0xD00F

// ACIA Register Offsets
typedef enum
{
    ACIA_REG_STATUS              = 0x00, // Status Register
    ACIA_REG_DATA_TX             = 0x01, // Transmit Data Register
    ACIA_REG_DATA_RX             = 0x02, // Receive Data Register
    ACIA_REG_CONTROL             = 0x03  // Control Register
    // Additional registers can be defined here
} AciaRegister;

// Status Register Bits
typedef enum
{
    ACIA_STATUS_TX_READY         = 0x01, // Transmitter Ready
    ACIA_STATUS_RX_READY         = 0x02, // Receiver Ready
    ACIA_STATUS_OVERRUN          = 0x04, // Overrun Error
    ACIA_STATUS_PARITY           = 0x08, // Parity Error
    ACIA_STATUS_FRAME            = 0x10  // Framing Error
    // Additional status bits can be defined here
} AciaStatus;

// Control Register Bits
typedef enum
{
    ACIA_CONTROL_ENABLE_TX       = 0x01, // Enable Transmitter
    ACIA_CONTROL_ENABLE_RX       = 0x02, // Enable Receiver
    ACIA_CONTROL_INTERRUPT_TX    = 0x04, // Transmit Interrupt Enable
    ACIA_CONTROL_INTERRUPT_RX    = 0x08  // Receive Interrupt Enable
    // Additional control bits can be defined here
} AciaControl;

// Buffer Sizes
#define ACIA_TX_BUFFER_SIZE 256
#define ACIA_RX_BUFFER_SIZE 256

/**
 * Represents the state of an ACIA 6550/6551 device.
 */
typedef struct
{
    // Transmit (TX) Buffer
    char tx_buffer[ACIA_TX_BUFFER_SIZE];
    uint16_t tx_head;
    uint16_t tx_tail;
    bool tx_ready;

    // Receive (RX) Buffer
    char rx_buffer[ACIA_RX_BUFFER_SIZE];
    uint16_t rx_head;
    uint16_t rx_tail;
    bool rx_ready;

    // Control Register
    uint8_t control_reg;

    // Status Register
    uint8_t status_reg;

    // Additional internal state can be added here
} Acia6550;

/**
 * @brief Initializes and allocates a new ACIA6550 instance.
 *
 * @return Pointer to the initialized Acia6550 structure, or NULL on
 * failure.
 */
Acia6550 *acia_init(void);

/**
 * @brief Resets the ACIA6550 to its default state.
 *
 * @param acia Pointer to the Acia6550 instance to reset.
 */
void acia_reset(Acia6550 *acia);

/**
 * @brief Destroys and frees the ACIA6550 instance.
 *
 * @param acia Pointer to the Acia6550 instance to destroy.
 */
void acia_destroy(Acia6550 *acia);

/**
 * @brief Reads a byte from the specified ACIA register address.
 *
 * @param acia Pointer to the Acia6550 instance.
 * @param address The memory address of the ACIA register to read.
 * @return The byte read from the register, or 0 if the address is invalid.
 */
uint8_t acia_read(Acia6550 *acia, uint16_t address);

/**
 * @brief Writes a byte to the specified ACIA register address.
 *
 * @param acia Pointer to the Acia6550 instance.
 * @param address The memory address of the ACIA register to write.
 * @param data The byte to write to the register.
 */
void acia_write(Acia6550 *acia, uint16_t address, uint8_t data);

/**
 * @brief Processes the transmit buffer, sending data out to the virtual
 * serial.
 *
 * @param acia Pointer to the Acia6550 instance.
 */
void acia_process_tx(Acia6550 *acia);

/**
 * @brief Provides input data to the receive buffer, simulating incoming
 * serial data.
 *
 * @param acia Pointer to the Acia6550 instance.
 * @param data The null-terminated string to insert into the RX buffer.
 */
void acia_provide_input(Acia6550 *acia, const char *data);

#endif // ACIA_H