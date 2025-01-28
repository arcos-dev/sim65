#include "memory.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initialize memory with a given size.
 * @param memory Pointer to the memory structure.
 * @param size Size of the memory in bytes.
 * @return 0 on success, -1 on failure.
 */
int memory_init(memory_t *memory, uint32_t size)
{
    if (!memory || size == 0)
        return -1;

    memory->data = (uint8_t *) malloc(size);
    if (!memory->data)
        return -1;

    memory->size = size;
    memory_reset(memory); // Initialize memory to zero
    return 0;
}

/**
 * @brief Destroy memory and free associated resources.
 * @param memory Pointer to the memory structure.
 */
void memory_destroy(memory_t *memory)
{
    if (memory && memory->data)
    {
        free(memory->data);
        memory->data = NULL;
        memory->size = 0;
    }
}

/**
 * @brief Reset memory (fill with zeros).
 * @param memory Pointer to the memory structure.
 */
void memory_reset(memory_t *memory)
{
    if (memory && memory->data)
        memset(memory->data, 0, memory->size);
}

/**
 * @brief Read a byte from memory.
 * @param memory Pointer to the memory structure.
 * @param address Memory address to read from.
 * @return Byte read from memory.
 */
uint8_t memory_read(memory_t *memory, uint16_t address)
{
    if (!memory || !memory->data || address >= memory->size)
        return 0;

    return memory->data[address];
}

/**
 * @brief Write a byte to memory.
 * @param memory Pointer to the memory structure.
 * @param address Memory address to write to.
 * @param value Byte to write.
 */
void memory_write(memory_t *memory, uint16_t address, uint8_t value)
{
    if (memory && memory->data && address < memory->size)
        memory->data[address] = value;
}