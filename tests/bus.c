#include "bus.h"
#include "../cpu.h"
#include "../clock.h"
#include "../memory.h"

/**
 * @brief Initialize the bus.
 * @param bus Pointer to the bus structure.
 * @param memory_size Size of the system memory.
 * @param clock_frequency Clock frequency in Hz.
 * @return 0 on success, -1 on failure.
 */
int bus_init(bus_t *bus, uint32_t memory_size, double clock_frequency)
{
    if (!bus || memory_size == 0)
        return -1;

    // Initialize memory
    if (memory_init(&bus->memory, memory_size) != 0)
        return -1;

    bus->clock_disabled = (clock_frequency == CPU_CLOCK_DISABLED);

    if (!bus->clock_disabled)
    {
        // Initialize clock
        bus->clock = (cpu_clock_t *) malloc(sizeof(cpu_clock_t));

        if (!bus->clock || clock_init(bus->clock, clock_frequency) != 0)
        {
            memory_destroy(&bus->memory);
            return -1;
        }
    }

    return 0;
}

/**
 * @brief Destroy the bus and free associated resources.
 * @param bus Pointer to the bus structure.
 */
void bus_destroy(bus_t *bus)
{
    if (bus)
    {
        memory_destroy(&bus->memory);

        if (bus->clock)
        {
            clock_destroy(bus->clock);
            free(bus->clock);
        }
    }
}

/**
 * @brief Reset the bus and all connected components.
 * @param bus Pointer to the bus structure.
 */
void bus_reset(bus_t *bus)
{
    if (bus)
    {
        memory_reset(&bus->memory);

        if (bus->clock)
            clock_reset(bus->clock);
    }
}

/**
 * @brief Read a byte from memory.
 * @param bus Pointer to the bus structure.
 * @param address Memory address to read from.
 * @return Byte read from memory.
 */
uint8_t bus_read_memory(bus_t *bus, uint16_t address)
{
    return memory_read(&bus->memory, address);
}

/**
 * @brief Write a byte to memory.
 * @param bus Pointer to the bus structure.
 * @param address Memory address to write to.
 * @param value Byte to write.
 */
void bus_write_memory(bus_t *bus, uint16_t address, uint8_t value)
{
    memory_write(&bus->memory, address, value);
}
