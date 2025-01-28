#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bus.h"
#include "../cpu.h"
#include "../clock.h"
#include "../memory.h"

/**
 * @brief Structure to hold the result of a single test
 */
typedef struct
{
    const char *filename;      ///< Test binary filename
    bool passed;               ///< Test passed flag
    uint16_t final_pc;         ///< Final Program Counter value
    uint8_t a, x, y;           ///< CPU registers: Accumulator, X, Y
    uint8_t sp;                ///< Stack Pointer
    uint8_t p;                 ///< Processor status flags
} test_result_t;

/**
 * @brief Structure defining a test case
 */
typedef struct
{
    const char *filename;      ///< Path to test binary
    uint16_t expected_pc;      ///< Expected final PC value
    bool trace;                ///< Enable instruction tracing
} test_def_t;

// Test suite definition
static test_def_t tests[] = {
    // Klaus Dormann tests
    {"6502_functional_test.bin",   0x3469, false},
    {"6502_decimal_test.bin",      0x044b, false},

    // Bird Computer test
    {"bird6502.bin",               0x861c, false},

    // Ruud Baltissen test
    {"ttl6502.bin",                0xf5ea, false},

    // Lorenz tests for undocumented opcodes
    {"lorenz/slo_asoa.bin",        0x08b3, false},
    {"lorenz/slo_asoax.bin",       0x08ca, false},
    {"lorenz/slo_asoay.bin",       0x08ca, false},
    {"lorenz/slo_asoix.bin",       0x08c4, false},
    {"lorenz/slo_asoiy.bin",       0x08ce, false},
    {"lorenz/slo_asoz.bin",        0x08b6, false},
    {"lorenz/slo_asozx.bin",       0x08c0, false},
    {"lorenz/rlaa.bin",            0x08aa, false},
    {"lorenz/rlaax.bin",           0x08c0, false},
    {"lorenz/rlaay.bin",           0x08c0, false},
    {"lorenz/rlaix.bin",           0x08ba, false},
    {"lorenz/rlaiy.bin",           0x08c4, false},
    {"lorenz/rlaz.bin",            0x08ad, false},
    {"lorenz/rlazx.bin",           0x08b6, false},
    {"lorenz/sre_lsea.bin",        0x08a8, false},
    {"lorenz/sre_lseax.bin",       0x08be, false},
    {"lorenz/sre_lseay.bin",       0x08be, false},
    {"lorenz/sre_lseix.bin",       0x08b8, false},
    {"lorenz/sre_lseiy.bin",       0x08c2, false},
    {"lorenz/sre_lsez.bin",        0x08ab, false},
    {"lorenz/sre_lsezx.bin",       0x08b4, false},
    {"lorenz/rraa.bin",            0x0887, false},
    {"lorenz/rraax.bin",           0x089d, false},
    {"lorenz/rraay.bin",           0x089d, false},
    {"lorenz/rraix.bin",           0x0897, false},
    {"lorenz/rraiy.bin",           0x08a1, false},
    {"lorenz/rraz.bin",            0x088a, false},
    {"lorenz/rrazx.bin",           0x0893, false},
    {"lorenz/sax_axsa.bin",        0x088d, false},
    {"lorenz/sax_axsix.bin",       0x0897, false},
    {"lorenz/sax_axsz.bin",        0x0890, false},
    {"lorenz/sax_axszy.bin",       0x0899, false},
    {"lorenz/laxa.bin",            0x088e, false},
    {"lorenz/laxay.bin",           0x08a4, false},
    {"lorenz/laxix.bin",           0x089e, false},
    {"lorenz/laxiy.bin",           0x08a8, false},
    {"lorenz/laxz.bin",            0x0891, false},
    {"lorenz/laxzy.bin",           0x089a, false},
    {"lorenz/dcp_dcma.bin",        0x088c, false},
    {"lorenz/dcp_dcmax.bin",       0x08a2, false},
    {"lorenz/dcp_dcmay.bin",       0x08a2, false},
    {"lorenz/dcp_dcmix.bin",       0x089c, false},
    {"lorenz/dcp_dcmiy.bin",       0x08a6, false},
    {"lorenz/dcp_dcmz.bin",        0x088f, false},
    {"lorenz/dcp_dcmzx.bin",       0x0898, false},
    {"lorenz/isc_insa.bin",        0x088c, false},
    {"lorenz/isc_insax.bin",       0x08a2, false},
    {"lorenz/isc_insay.bin",       0x08a2, false},
    {"lorenz/isc_insix.bin",       0x089c, false},
    {"lorenz/isc_insiy.bin",       0x08a6, false},
    {"lorenz/isc_insz.bin",        0x088f, false},
    {"lorenz/isc_inszx.bin",       0x0898, false},
    {"lorenz/ancb.bin",            0x08d8, false},
    {"lorenz/alrb.bin",            0x08aa, false},
    {"lorenz/arrb.bin",            0x0947, false},
    {"lorenz/sbxb.bin",            0x08c3, false},
    {"lorenz/lasay.bin",           0x08f1, false},
    {"lorenz/shaay.bin",           0x08d6, false},
    {"lorenz/shaiy.bin",           0x08d9, false},
    {"lorenz/shxay.bin",           0x08b5, false},
    {"lorenz/shyax.bin",           0x08b5, false},
    {"lorenz/tas_shsay.bin",       0x08f5, false},
    {"lorenz/aneb.bin",            0x08cb, false},
    {"lorenz/lxab.bin",            0x08c2, false},

    // Visual6502 test for decimal mode
    {"6502DecimalMode.bin",        0x8133, false},

    // Piotr Fusik tests
    {"cpu_decimal.bin",            0x302f, false},
    {"cpu_las.bin",                0x304f, false},

    // Avery Lee tests
    {"avery.bin",                  0x20db, false},
    {"avery2.bin",                 0x20fa, false},
    {"avery3.bin",                 0x209d, false},

    // HCM6502 tests
    {"AllSuiteA.bin",              0x45c0, false},
};

/**
 * @brief Expected cycle counts for 6502 instructions
 *
 * Organized by opcode groups. Reference data from:
 * http://6502.org/tutorials/6502opcodes.html
 */
static const uint8_t exp_cycles[] = {
    // ----- Documented Opcodes (00-FF) ----------------------------------------

    // $00-$0F: BRK, ORA, ASL, PHP
    7, 6, 6, 3, 5, 3, 2, 2, 4, 6,                            // 00-0F

    // $10-$1F: BPL, ORA (indirect,Y), CLC, ORA (abs,Y/X)
    2, 2, 2, 3, 4, 2, 5, 2, 6, 4, 6, 2, 4, 5, 2, 4, 5, 7,    // 10-1F

    // $20-$2F: JSR, AND, BIT, ROL, PLP
    6, 6, 6, 3, 3, 5, 4, 2, 2, 4, 4, 6,                      // 20-2F

    // $30-$3F: BMI, AND (indirect,Y), SEC, AND (abs,Y/X)
    2, 2, 2, 3, 4, 2, 5, 2, 6, 4, 6, 2, 4, 5, 4, 5, 7,       // 30-3F

    // $40-$4F: RTI, EOR, LSR, PHA, JMP
    6, 3, 5, 3, 2, 2, 3, 4, 6,                               // 40-4F

    // $50-$5F: BVC, EOR (indirect,Y), CLI, EOR (abs,Y/X)
    2, 3, 4, 2, 2, 3, 4, 2, 5, 2, 6, 4, 6, 2, 4, 5, 4, 5, 7, // 50-5F

    // $60-$6F: RTS, ADC, ROR, PLA
    2, 6, 3, 2, 4, 2, 7, 4, 3, 5, 5, 4, 2, 5, 6,             // 60-6F

    // $70-$7F: BVS, ADC (indirect,Y), SEI, ADC (abs,Y/X)
    2, 2, 2, 3, 4, 3, 4, 2, 2, 5, 2, 6, 4, 4, 5, 4, 5,       // 70-7F

    // $80-$8F: STA, STY, STX, DEY, TXA
    2, 2, 6, 2, 7, 5, 5, 6, 5, 6, 6, 2, 7,                   // 80-8F

    // $90-$9F: BCC, STA (indirect,Y), TYA, STA (abs,Y/X)
    2, 6, 3, 3, 3, 2, 2, 4, 4, 4,                            // 90-9F

    // $A0-$AF: LDY, LDA, LDX, TAY, TAX
    2, 2, 2, 3, 4, 2, 6, 2, 3, 6, 2, 4, 4, 4, 2, 5, 5,       // A0-AF

    // $B0-$BF: BCS, LDA (indirect,Y), CLV, LDA (abs,Y/X)
    2, 5, 5,                                                 // B0-BF (partial)
    2, 6, 2, 3, 3, 3, 2, 2, 2, 4, 4, 4,                      // ...continued
    2, 2, 2, 3, 4, 2, 5, 2, 6, 4, 4, 4, 2, 2, 4, 5, 2, 2,    // ...continued

    // $C0-$CF: CPY, CMP, DEC, INY
    4, 5, 4, 5, 4, 5,                                        // C0-CF (partial)
    2, 6, 3, 3, 5, 2, 2, 2, 4, 4, 6,                         // ...continued

    // $D0-$DF: BNE, CMP (indirect,Y), CMP (abs,Y/X)
    2, 2, 2, 3, 4, 2, 5, 2, 6, 4, 6, 2, 4, 5, 4, 5, 7,       // D0-DF

    // $E0-$EF: CPX, SBC, INC, INX
    2, 2, 6, 3, 2, 4, 2, 7, 4, 3, 5, 3, 5, 2, 2, 4, 6,       // E0-EF

    // $F0-$FF: BEQ, SBC (indirect,Y), SED, SBC (abs,Y/X)
    2, 2, 2, 3, 4, 2, 2, 5, 2, 6, 4, 4, 5, 2, 4, 2, 5,       // F0-FF
    2, 2, 6, 2, 7, 5, 5, 6, 2, 5, 2, 6, 6, 2, 7,             // ...continued

    // ----- Undocumented Opcodes ----------------------------------------------

    // NOP variants
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3,                // $1A, $3A, etc.

    // Memory operations
    4, 4, 4, 4, 4, 4, 4,                                     // Various

    // Shift/rotate variants
    2, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5,                   // SLO/RLA/SRE/RRA
    5, 6, 8, 8, 6, 7, 7,                                     // SLO
    5, 6, 8, 8, 6, 7, 7,                                     // RLA
    5, 6, 8, 8, 6, 7, 7,                                     // SRE
    5, 6, 8, 8, 6, 7, 7,                                     // RRA

    // SAX/LAX
    3, 4, 6, 4,                                              // SAX
    3, 4, 6, 4, 2, 5, 4, 2, 6, 5,                            // LAX

    // DCP/ISC
    5, 6, 8, 8, 6, 7, 7,                                     // DCP
    5, 6, 8, 8, 6, 7, 7,                                     // ISC

    // Misc undocumented
    2, 2, 2, 2, 2,                                           // ANC, ALR, ARR, SBX
    2, 4, 5,                                                 // LAS
    6, 5, 5, 5, 5, 2, 5, 5, 5, 5,                            // SHA, SHX, SHY, TAS
    2, 2,                                                    // ANE, LXA
    3, 3                                                     // Final
};

/**
 * @brief Load binary file into memory
 * @param bus Pointer to the bus structure
 * @param filename Path to binary file
 * @return true on success, false on error
 */
static bool load_file(bus_t *bus, const char *filename)
{
    FILE *f = fopen(filename, "rb");

    if (!f)
    {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return false;
    }

    uint8_t *memory   = bus->memory.data;
    size_t bytes_read = fread(memory, 1, bus->memory.size, f);
    fclose(f);

    if (bytes_read != bus->memory.size)
    {
        fprintf(stderr, "Error: Incomplete read for '%s' (%llu/%u bytes)\n",
                filename, bytes_read, bus->memory.size);
        return false;
    }

    return true;
}

/**
 * @brief Execute a test case and capture results
 * @param bus Pointer to the bus structure
 * @param filename Test binary path
 * @param expected_pc Expected final PC value
 * @param trace Enable instruction tracing
 * @param result Pointer to store test results
 */
static void run_test(bus_t *bus, const char *filename, uint16_t expected_pc,
                     bool trace, test_result_t *result)
{
    // Initialize result structure
    memset(result, 0, sizeof(*result));
    result->filename = filename;

    if (!load_file(bus, filename))
    {
        result->passed = false;
        return;
    }

    cpu6502_reset();
    uint16_t prev_pc;

    do
    {
        prev_pc = cpu->pc;
        cpu6502_step();

        if (trace)
        {
            printf("PC=%04X  A=%02X  X=%02X  Y=%02X  SP=%02X  P=%02X\n",
                   cpu->pc, cpu->a, cpu->x, cpu->y, cpu->sp,
                   cpu6502_get_status());
        }
    } while (prev_pc != cpu->pc); // Run until PC stabilizes

    // Capture final state
    result->final_pc = cpu->pc;
    result->a        = cpu->a;
    result->x        = cpu->x;
    result->y        = cpu->y;
    result->sp       = cpu->sp;
    result->p        = cpu6502_get_status();
    result->passed   = (result->final_pc == expected_pc);
}

/* ========================================================================== */
/*                               Output Formatting                            */
/* ========================================================================== */

// ANSI escape sequences for colored PASS/FAIL labels
#define FAIL "\x1b[1;31mFAIL\x1b[0m  "
#define PASS "\x1b[1;32mPASS\x1b[0m  "

/**
 * @brief Print results table header
 */
static void print_table_header(void)
{
    printf("\n%-30s  %-6s  %-6s  %-4s  %-4s  %-4s  %-7s\n", "Test File",
           "Status", "PC", "A", "X", "Y", "SP/P");
    printf("%-30s  %-6s  %-6s  %-4s  %-4s  %-4s  %-7s\n",
           "------------------------------", "------", "------", "----", "----",
           "----", "-------");
}

/**
 * @brief Print formatted test result
 * @param res Pointer to test result
 */
static void print_test_result(const test_result_t *res)
{
    const char *status = res->passed ? PASS : FAIL;
    printf("%-30s  %-6s  %04X    %02X    %02X    %02X    %02X/%02X\n",
           res->filename, status, res->final_pc, res->a, res->x, res->y,
           res->sp, res->p);
}

/**
 * @brief Validate instruction cycle counts against reference data
 */
static void test_cycles(bus_t *bus)
{
    printf("Starting cycle accuracy test...\n");

    if (!load_file(bus, "cycles.bin"))
    {
        fprintf(stderr, "Cycle test aborted: missing test ROM\n");
        return;
    }

    cpu6502_reset();

    size_t idx         = 0;
    size_t total_tests = sizeof(exp_cycles) / sizeof(exp_cycles[0]);
    bool test_active   = false;

    while (1)
    {
        uint16_t pc    = cpu->pc;
        uint8_t opcode = bus_read_memory(bus, pc);
        int cycles     = cpu6502_step();

        if (pc == 0x3000)
            test_active = true;

        if (pc == 0x200A)
            break;

        if (test_active)
        {
            if (idx >= total_tests)
            {
                printf("ERROR: Test overflow at index %zu\n", idx);
                return;
            }

            if (cycles != exp_cycles[idx])
            {
                printf("FAIL: $%04X: $%02X  Actual: %d  Expected: %d\n", pc,
                       opcode, cycles, exp_cycles[idx]);
                printf("Cycle test: " FAIL "\n");
                return;
            }

            idx++;
        }
    }

    printf("Cycle test: " PASS "  (%zu instructions verified)\n", idx);
}

/* ========================================================================== */
/*                                    Main                                    */
/* ========================================================================== */

int main(void)
{
    // Initialize the bus, memory, and clock
    bus_t bus;

    // Initialize the bus with a memory size and clock frequency disabled
    if (bus_init(&bus, 64 * 1024, CPU_CLOCK_DISABLED) != 0)
    {
        fprintf(stderr, "Failed to initialize bus.\n");
        return EXIT_FAILURE;
    }

    if (cpu6502_init(&bus) != 0)
    {
        fprintf(stderr, "CPU initialization failed\n");
        bus_destroy(&bus);
        return EXIT_FAILURE;
    }

    printf("6502 Emulator Test Suite\n");
    print_table_header();

    const size_t test_count = sizeof(tests) / sizeof(tests[0]);
    test_result_t *results  = malloc(test_count * sizeof(test_result_t));

    if (!results)
    {
        fprintf(stderr, "Memory allocation failed\n");
        cpu6502_destroy();
        bus_destroy(&bus);
        return EXIT_FAILURE;
    }

    // Execute all tests
    for (size_t i = 0; i < test_count; i++)
    {
        run_test(&bus, tests[i].filename, tests[i].expected_pc, tests[i].trace,
                 &results[i]);
        print_test_result(&results[i]);
    }

    // Calculate and display summary
    size_t passed = 0;

    for (size_t i = 0; i < test_count; i++)
    {
        if (results[i].passed)
            passed++;
    }

    printf("\nSummary: %zu/%zu tests passed\n", passed, test_count);
    free(results);

    // Perform cycle accuracy test
    test_cycles(&bus);

    // Clean up
    cpu6502_destroy();
    bus_destroy(&bus);

    return EXIT_SUCCESS;
}
