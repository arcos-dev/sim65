#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

/* -----------------------------------------------------------------------------
 * Memory structure
 * -----------------------------------------------------------------------------
 * The memory structure holds the following:
 * - data: Pointer to the memory data (dynamically allocated array)
 * - size: Size of the memory in bytes
 */
typedef struct
{
    uint8_t *data; ///< Pointer to the memory data
    uint32_t size; ///< Size of the memory in bytes
} memory_t;

/* -----------------------------------------------------------------------------
 * Function Prototypes
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Initialize memory with a given size.
 * @param memory Pointer to the memory structure.
 * @param size Size of the memory in bytes.
 * @return 0 on success, -1 on failure.
 */
int memory_init(memory_t *memory, uint32_t size);

/**
 * @brief Destroy memory and free associated resources.
 * @param memory Pointer to the memory structure.
 */
void memory_destroy(memory_t *memory);

/**
 * @brief Reset memory (fill with zeros).
 * @param memory Pointer to the memory structure.
 */
void memory_reset(memory_t *memory);

/**
 * @brief Read a byte from memory.
 * @param memory Pointer to the memory structure.
 * @param address Memory address to read from.
 * @return Byte read from memory.
 */
uint8_t memory_read(memory_t *memory, uint16_t address);

/**
 * @brief Write a byte to memory.
 * @param memory Pointer to the memory structure.
 * @param address Memory address to write to.
 * @param value Byte to write.
 */
void memory_write(memory_t *memory, uint16_t address, uint8_t value);

#endif /* MEMORY_H */