#include "clock.h"
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

/* -----------------------------------------------------------------------------
 * Platform-specific data for high-resolution timing
 * -----------------------------------------------------------------------------
 * This structure holds platform-specific data for timing functions.
 */
typedef struct
{
#ifdef _WIN32
    LARGE_INTEGER frequency;  ///< Frequency of the performance counter
    LARGE_INTEGER start_time; ///< Start time of the clock
#else
    struct timespec start_time; ///< Start time of the clock
#endif
} clock_platform_data_t;

/* -----------------------------------------------------------------------------
 * Function Implementations
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Get the current high-resolution time in seconds.
 * @param data Pointer to the platform-specific data.
 * @return Current time in seconds.
 */
static double get_current_time(clock_platform_data_t *data)
{
#ifdef _WIN32
    LARGE_INTEGER current_time;
    QueryPerformanceCounter(&current_time);
    return (double) (current_time.QuadPart - data->start_time.QuadPart) /
           (double) data->frequency.QuadPart;
#else
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    return (current_time.tv_sec - data->start_time.tv_sec) +
           (current_time.tv_nsec - data->start_time.tv_nsec) / 1e9;
#endif
}

/**
 * @brief Initialize the clock with a given frequency.
 * @param clock Pointer to the clock structure.
 * @param frequency Clock frequency in Hz.
 * @return 0 on success, -1 on failure.
 */
int clock_init(cpu_clock_t *clock, double frequency)
{
    if (!clock || frequency <= 0.0)
        return -1;

    clock->frequency      = frequency;
    clock->cycle_count    = 0;
    clock->cycle_duration = 1.0 / frequency;
    clock->elapsed_time   = 0.0;
    clock->platform_data  = malloc(sizeof(clock_platform_data_t));

    if (!clock->platform_data)
        return -1;

    clock_platform_data_t *data =
        (clock_platform_data_t *) clock->platform_data;

#ifdef _WIN32
    QueryPerformanceFrequency(&data->frequency);
    QueryPerformanceCounter(&data->start_time);
#else
    clock_gettime(CLOCK_MONOTONIC, &data->start_time);
#endif

    return 0;
}

/**
 * @brief Destroy the clock and free associated resources.
 * @param clock Pointer to the clock structure.
 */
void clock_destroy(cpu_clock_t *clock)
{
    if (clock && clock->platform_data)
    {
        free(clock->platform_data);
        clock->platform_data = NULL;
    }
}

/**
 * @brief Wait until the next clock cycle.
 * @param clock Pointer to the clock structure.
 */
void clock_wait_next_cycle(cpu_clock_t *clock)
{
    if (!clock || !clock->platform_data)
        return;

    clock_platform_data_t *data =
        (clock_platform_data_t *) clock->platform_data;

    // Calculate the expected time for the next cycle
    double expected_time = (clock->cycle_count + 1) * clock->cycle_duration;
    double current_time  = get_current_time(data);

    // Calculate the remaining time to wait
    double sleep_time = expected_time - current_time;

    // Ensure sleep_time is not negative
    if (sleep_time < 0)
        sleep_time = 0;

    // Sleep for the remaining time
    if (sleep_time > 0)
    {
#ifdef _WIN32
        // Convert sleep_time to milliseconds
        DWORD sleep_ms = (DWORD) (sleep_time * 1000.0);

        if (sleep_ms > 0)
        {
            Sleep(sleep_ms);
        }
        else
        {
            // Sleep(0) yields the rest of the thread's time slice
            Sleep(0);
        }
#else
        struct timespec ts;

        ts.tv_sec  = (time_t) sleep_time;
        ts.tv_nsec = (long) ((sleep_time - ts.tv_sec) * 1e9);

        if (ts.tv_sec > 0 || ts.tv_nsec > 0)
        {
            nanosleep(&ts, NULL);
        }
        else
        {
            // Yield to other threads
            sched_yield();
        }
#endif
    }

    // Update elapsed time and increment the cycle count
    clock->elapsed_time = expected_time;
    clock->cycle_count++;
}

/**
 * @brief Reset the clock to its initial state.
 * @param clock Pointer to the clock structure.
 */
void clock_reset(cpu_clock_t *clock)
{
    if (!clock || !clock->platform_data)
        return;

    clock->cycle_count  = 0;
    clock->elapsed_time = 0.0;
    clock_platform_data_t *data =
        (clock_platform_data_t *) clock->platform_data;

#ifdef _WIN32
    QueryPerformanceCounter(&data->start_time);
#else
    clock_gettime(CLOCK_MONOTONIC, &data->start_time);
#endif
}

/**
 * @brief Get the current cycle count.
 * @param clock Pointer to the clock structure.
 * @return Current cycle count.
 */
uint64_t clock_get_cycle_count(const cpu_clock_t *clock)
{
    return clock ? clock->cycle_count : 0;
}

/**
 * @brief Get the elapsed time in seconds.
 * @param clock Pointer to the clock structure.
 * @return Elapsed time in seconds.
 */
double clock_get_elapsed_time(const cpu_clock_t *clock)
{
    return clock ? clock->elapsed_time : 0.0;
}
