#ifndef BUS_H
#define BUS_H

#include <stdlib.h>
#include <stdbool.h>
#include "../clock.h"
#include "../memory.h"

/* -----------------------------------------------------------------------------
 * Bus structure
 * -----------------------------------------------------------------------------
 * The bus connects the CPU, memory, and clock.
 */
typedef struct
{
    cpu_clock_t *clock; ///< Pointer to the clock
    memory_t memory;    ///< Memory structure
    bool clock_disabled;
} bus_t;

/* -----------------------------------------------------------------------------
 * Function Prototypes
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Initialize the bus.
 * @param bus Pointer to the bus structure.
 * @param memory_size Size of the system memory.
 * @param clock_frequency Clock frequency in Hz.
 * @return 0 on success, -1 on failure.
 */
int bus_init(bus_t *bus, uint32_t memory_size, double clock_frequency);

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
 * @brief Read a byte from memory.
 * @param bus Pointer to the bus structure.
 * @param address Memory address to read from.
 * @return Byte read from memory.
 */
uint8_t bus_read_memory(bus_t *bus, uint16_t address);

/**
 * @brief Write a byte to memory.
 * @param bus Pointer to the bus structure.
 * @param address Memory address to write to.
 * @param value Byte to write.
 */
void bus_write_memory(bus_t *bus, uint16_t address, uint8_t value);

#endif /* BUS_H */
