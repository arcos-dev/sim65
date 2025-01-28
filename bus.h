/*******************************************************************************
 * bus.h
 *
 * Bus (CPU + Memory + Clock + I/O) Emulation Module with ACIA and TIA Support
 *
 * This header file declares the Bus structure and functions required to
 * emulate a simple 6502-like memory bus with integrated clock and optional
 * attached devices (ACIA 6550/6551 and TIA). It provides functions for
 * initialization, destruction, resetting, reading, and writing data.
 *
 * Author: Anderson Costa
 * Date: 2025-01-26
 ******************************************************************************/

#ifndef BUS_H
#define BUS_H

#include "acia.h"
#include "clock.h"
#include "memory.h"
#include "tia.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* -----------------------------------------------------------------------------
 * Bus structure
 * -----------------------------------------------------------------------------
 * The bus connects the CPU, memory, clock, and optional I/O devices.
 */
typedef struct
{
    cpu_clock_t *clock;  ///< Pointer to the clock
    memory_t memory;     ///< Memory structure
    bool clock_disabled; ///< Flag to indicate if the clock is disabled
    Acia6550 *acia;      ///< Optional pointer to an ACIA device
    TIA *tia;            ///< Optional pointer to a TIA device
} bus_t;

/* -----------------------------------------------------------------------------
 * Function Prototypes
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Initialize the bus.
 * @param bus Pointer to the bus structure.
 * @param memory_size Size of the system memory in bytes.
 * @param clock_frequency Clock frequency in Hz. Use CPU_CLOCK_DISABLED to
 * disable the clock.
 * @param acia Optional pointer to an ACIA device (can be NULL if not used).
 * @param tia Optional pointer to a TIA device (can be NULL if not used).
 * @return 0 on success, -1 on failure.
 */
int bus_init(bus_t *bus, uint32_t memory_size, double clock_frequency,
             Acia6550 *acia, TIA *tia);

/**
 * @brief Destroy the bus and free associated resources.
 * @param bus Pointer to the bus structure.
 */
void bus_destroy(bus_t *bus);

/**
 * @brief Reset the bus and all connected components.
 * @param bus Pointer to the bus structure.
 */
void bus_reset(bus_t *bus);

/**
 * @brief Read a byte from memory or I/O devices.
 * @param bus Pointer to the bus structure.
 * @param address Memory or I/O address to read from.
 * @return Byte read from memory or device.
 */
uint8_t bus_read_memory(bus_t *bus, uint16_t address);

/**
 * @brief Write a byte to memory or I/O devices.
 * @param bus Pointer to the bus structure.
 * @param address Memory or I/O address to write to.
 * @param value Byte to write.
 */
void bus_write_memory(bus_t *bus, uint16_t address, uint8_t value);

/**
 * @brief Loads a binary file into the bus memory starting at start_address.
 * @param bus Pointer to the Bus.
 * @param filename Path to the binary file to load.
 * @param start_address 16-bit address at which to load the file content.
 *
 * @return 0 on success, or -1 on error.
 */
int bus_load_program(bus_t *bus, const char *filename, uint16_t start_address);

#endif /* BUS_H */
