#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

/* -----------------------------------------------------------------------------
 * Clock structure
 * -----------------------------------------------------------------------------
 * The clock structure holds the following:
 * - frequency: Clock frequency in Hz
 * - cycle_count: Total cycles executed
 * - cycle_duration: Duration of one cycle in seconds
 * - elapsed_time: Elapsed time in seconds
 * - platform_data: Pointer to platform-specific data
 */
typedef struct
{
    double frequency;      ///< Clock frequency in Hz
    uint64_t cycle_count;  ///< Total cycles executed
    double cycle_duration; ///< Duration of one cycle in seconds
    double elapsed_time;   ///< Elapsed time in seconds
    void *platform_data;   ///< Pointer to platform-specific data
} cpu_clock_t;

/* -----------------------------------------------------------------------------
 * CPU Clock Configurations
 * -----------------------------------------------------------------------------
 * Predefined clock configurations for different systems.
 */
typedef enum
{
    /* Disable predefined clock */
    CPU_CLOCK_DISABLED = 0,

    /* Apple I clock configuration (1.0 MHz) */
    CPU_CLOCK_APPLE_I = 10000000,

    /* Apple II clock configuration (1.023 MHz) */
    CPU_CLOCK_APPLE_II = 10230000,

    /* Atari 2600 clock configuration (1.19 MHz) */
    CPU_CLOCK_ATARI_2600 = 11900000,

    /* Atari 8-bit computers (400/800/XL/XE) clock configuration (1.79 MHz) */
    CPU_CLOCK_ATARI_8BIT = 17900000,

    /* Commodore 64 PAL clock configuration (0.985 MHz) */
    CPU_CLOCK_COMMODORE_64_PAL = 9850000,

    /* Commodore 64 NTSC clock configuration (1.023 MHz) */
    CPU_CLOCK_COMMODORE_64_NTSC = 10230000,

    /* Commodore VIC-20 PAL clock configuration (1.02 MHz) */
    CPU_CLOCK_COMMODORE_VIC20_PAL = 10200000,

    /* Commodore VIC-20 NTSC clock configuration (1.10 MHz) */
    CPU_CLOCK_COMMODORE_VIC20_NTSC = 11000000,

    /* Nintendo Entertainment System (NES) NTSC clock configuration (1.79 MHz) */
    CPU_CLOCK_NES_NTSC = 1789772,

    /* Nintendo Entertainment System (NES) PAL clock configuration (1.66 MHz) */
    CPU_CLOCK_NES_PAL = 16626070,

    /* BBC Micro clock configuration (2.0 MHz) */
    CPU_CLOCK_BBC_MICRO = 20000000

} clock_config_t;

/* -----------------------------------------------------------------------------
 * Function Prototypes
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Initialize the clock with a given frequency.
 * @param clock Pointer to the clock structure.
 * @param frequency Clock frequency in Hz.
 * @return 0 on success, -1 on failure.
 */
int clock_init(cpu_clock_t *clock, double frequency);

/**
 * @brief Destroy the clock and free associated resources.
 * @param clock Pointer to the clock structure.
 */
void clock_destroy(cpu_clock_t *clock);

/**
 * @brief Wait until the next clock cycle.
 * @param clock Pointer to the clock structure.
 */
void clock_wait_next_cycle(cpu_clock_t *clock);

/**
 * @brief Reset the clock to its initial state.
 * @param clock Pointer to the clock structure.
 */
void clock_reset(cpu_clock_t *clock);

/**
 * @brief Get the current cycle count.
 * @param clock Pointer to the clock structure.
 * @return Current cycle count.
 */
uint64_t clock_get_cycle_count(const cpu_clock_t *clock);

/**
 * @brief Get the elapsed time in seconds.
 * @param clock Pointer to the clock structure.
 * @return Elapsed time in seconds.
 */
double clock_get_elapsed_time(const cpu_clock_t *clock);

#endif /* CLOCK_H */
