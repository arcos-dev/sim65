/*******************************************************************************
 * bus.c
 *
 * Bus (CPU + Memory + Clock + I/O) Emulation Module with ACIA and TIA Support
 *
 * This source file implements the Bus structure and functions required to
 * emulate a simple 6502-like memory bus with integrated clock and optional
 * attached devices (ACIA 6550/6551 and TIA). It provides functions for
 * initialization, destruction, resetting, reading, and writing data.
 *
 * Author: Anderson Costa
 * Date: 2025-01-26
 ******************************************************************************/

#include "bus.h"
#include "tia.h"
#include "memory.h"
#include "acia.h"
#include "clock.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
             Acia6550 *acia, TIA *tia)
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
    else
    {
        bus->clock = NULL;
    }

    // Assign ACIA and TIA pointers
    bus->acia = acia;
    bus->tia  = tia;

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
        // Destroy ACIA
        if (bus->acia)
        {
            acia_destroy(bus->acia);
            bus->acia = NULL;
        }

        // Destroy TIA
        if (bus->tia)
        {
            tia_destroy(bus->tia);
            bus->tia = NULL;
        }

        // Destroy memory
        memory_destroy(&bus->memory);

        // Destroy clock
        if (bus->clock)
        {
            clock_destroy(bus->clock);
            free(bus->clock);
            bus->clock = NULL;
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
        // Reset memory
        memory_reset(&bus->memory);

        // Reset clock
        if (bus->clock)
            clock_reset(bus->clock);

        // Reset ACIA
        if (bus->acia)
            acia_reset(bus->acia);

        // Reset TIA
        // if (bus->tia)
        //    tia_reset(bus->tia);
    }
}

/**
 * @brief Read a byte from memory ou dispositivos I/O.
 * @param bus Pointer to the bus structure.
 * @param address Memory ou endereço I/O para ler.
 * @return Byte lido da memória ou dispositivo.
 */
uint8_t bus_read_memory(bus_t *bus, uint16_t address)
{
    if (bus == NULL)
        return 0;

    // Check if TIA is present and address is in TIA's range
    if (bus->tia != NULL && address <= TIA_END_ADDRESS)
    {
        return tia_read(bus->tia, address);
    }

    // Check if ACIA is present and address is in ACIA's range
    if (bus->acia != NULL && address >= ACIA_START_ADDRESS &&
        address <= ACIA_END_ADDRESS)
    {
        return acia_read(bus->acia, address);
    }

    // Handle any additional memory-mapped I/O

    // Otherwise, read from main memory
    return memory_read(&bus->memory, address);
}

/**
 * @brief Write a byte to memory ou dispositivos I/O.
 * @param bus Pointer to the bus structure.
 * @param address Memory ou endereço I/O para escrever.
 * @param value Byte para escrever.
 */
void bus_write_memory(bus_t *bus, uint16_t address, uint8_t value)
{
    if (bus == NULL)
        return;

    // Check if TIA is present and address is in TIA's range
    if (bus->tia != NULL && address <= TIA_END_ADDRESS)
    {
        tia_write(bus->tia, address, value);
        return;
    }

    // Check if ACIA is present and address is in ACIA's range
    if (bus->acia != NULL && address >= ACIA_START_ADDRESS &&
        address <= ACIA_END_ADDRESS)
    {
        acia_write(bus->acia, address, value);
        return;
    }

    // Handle any additional memory-mapped I/O
    switch (address)
    {
        case 0xD012:
            // Prints the character on the console
            printf("%c", value);
            fflush(stdout); // Ensures immediate exit
            return;

        case 0xD020:
            // Handles the control register for color or other device
            // printf("Writing to 0xD020: 0x%02X\n", value);
            return;

        case 0xD021:
            // Handles another register control
            // printf("Writing to 0xD021: 0x%02X\n", value);
            return;

        default:
            // If not mapped to I/O, write to main memory
            memory_write(&bus->memory, address, value);
            return;
    }
}

/**
 * @brief Loads a binary file into the bus memory starting at start_address.
 * @param bus Pointer to the Bus.
 * @param filename Path to the binary file to load.
 * @param start_address 16-bit address at which to load the file content.
 *
 * @return 0 on success, or -1 on error.
 */
int bus_load_program(bus_t *bus, const char *filename, uint16_t start_address)
{
    // Check bus
    if (bus == NULL)
    {
        fprintf(stderr, "bus_load_program: bus is NULL.\n");
        return -1;
    }

    // Open the file in binary mode
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "Failed to open file '%s'.\n", filename);
        return -1;
    }

    // Find the file size
    if (fseek(file, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "Error seeking file '%s'.\n", filename);
        fclose(file);
        return -1;
    }

    long filesize = ftell(file);

    if (filesize < 0)
    {
        fprintf(stderr, "Invalid file size for '%s'.\n", filename);
        fclose(file);
        return -1;
    }

    // Check if the file fits in memory from start_address
    if ((uint32_t) filesize > (0x10000U - start_address))
    {
        fprintf(stderr,
                "File '%s' (size %ld) does not fit at start address 0x%04X.\n",
                filename, filesize, start_address);
        fclose(file);
        return -1;
    }

    // Rewind to start for reading
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "Error rewinding file '%s'.\n", filename);
        fclose(file);
        return -1;
    }

    // Read file into the bus memory
    size_t read_size =
        fread(&bus->memory.data[start_address], 1, (size_t) filesize, file);
    fclose(file);

    // Check for a complete read
    if (read_size != (size_t) filesize)
    {
        fprintf(stderr, "Failed to read the entire file '%s'.\n", filename);
        return -1;
    }

    // Log debug info
    printf("Loading file '%s' at address 0x%04X\n", filename, start_address);
    printf("ROM size: %zu bytes\n", read_size);
    printf("Copied ROM data to memory from 0x%04X to 0x%04lX\n", start_address,
           (unsigned long) (start_address + read_size - 1));

    return 0;
}
