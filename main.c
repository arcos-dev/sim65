/*******************************************************************************
 * main.c
 *
 * Example of an interactive “monitor” for a 6502 emulator, with features:
 *   - Loading binary files
 *   - Resetting the CPU
 *   - Stepping instructions
 *   - Running continuously
 *   - Viewing/modifying memory (ASCII table)
 *   - Viewing/modifying CPU registers
 *   - Triggering IRQ/NMI
 *   - Stack dumping (ASCII table)
 *   - Feeding serial data to the ACIA
 *   - Clearing the screen
 *   - Disassembling code (simple example)
 *   - Toggling CPU clock
 *   - Adjusting clock frequency
 *
 * Enhanced for better analysis/study, especially with ASCII-based tables for
 * memory visualization.
 *
 * Author: Anderson Costa
 * Date: 2025-01-28
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
  #include <conio.h>
  #define CLEAR_COMMAND "cls"
#else
  #include <sys/select.h>
  #include <unistd.h>
  #define CLEAR_COMMAND "clear"
#endif

#include "acia.h"
#include "bus.h"
#include "cpu.h"
#include "palette.h"
#include "tia.h"

/* -----------------------------------------------------------------------------
 * Global emulator objects
 * -----------------------------------------------------------------------------
 */
static bus_t g_bus;
static Acia6550 *g_acia = NULL;
static TIA *g_tia       = NULL;

/* -----------------------------------------------------------------------------
 * Forward declarations
 * -----------------------------------------------------------------------------
 */
static void run_monitor_loop(void);
static bool monitor_parse_command(const char *line);

static void print_help(void);

static void do_reset_cpu(void);
static void do_step(int steps);
static void do_run(int steps);
static void do_memdump(uint16_t start, int count);
static void do_show_registers(void);
static void do_serial_in(const char *input_str);
static void do_irq(void);
static void do_nmi(void);
static void do_stackdump(int count);
static void do_clear_screen(void);
static int do_load_program(const char *filename, uint16_t address);
static void do_disassemble(uint16_t start, int count);
static void do_clock_command(const char *args);

/* Utility functions for ASCII table output */
static void print_ascii_table_header(uint16_t start_addr);
static void print_ascii_table_line(uint16_t addr, const uint8_t *data,
                                   int length);

/* Utility to read a line from stdin (blocking). */
static char *read_line_from_stdin(void);

/* Cross-platform kbhit / non-blocking char read */
#ifdef _WIN32
static int kbhit_nonblock(void)
{
    return _kbhit() ? 1 : 0;
}

static int read_char_nonblock(void)
{
    if (_kbhit())
        return _getch();
    return -1;
}
#else
static int kbhit_nonblock(void)
{
    fd_set readfds;
    struct timeval tv;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    if (ret < 0)
        return -1;

    if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds))
        return 1;

    return 0;
}

static int read_char_nonblock(void)
{
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n == 1)
        return c;
    return -1;
}
#endif

/* -----------------------------------------------------------------------------
 * main
 * -----------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <program.bin> <start_address>\n", argv[0]);
        fprintf(stderr, "Example: %s hello.bin 0xC000\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    char *endptr;
    uint16_t start_address = (uint16_t) strtol(argv[2], &endptr, 16);

    if (*endptr != '\0')
    {
        fprintf(stderr, "Invalid start address format (use e.g. 0x8000)\n");
        return EXIT_FAILURE;
    }

    /* Initialize ACIA */
    g_acia = acia_init();

    if (!g_acia)
    {
        fprintf(stderr, "Error: Failed to allocate ACIA.\n");
        return EXIT_FAILURE;
    }

    /* Initialize TIA (optional) */
    g_tia = tia_init(TV_SYSTEM_NTSC);

    if (!g_tia)
    {
        fprintf(stderr, "Error: Failed to allocate TIA.\n");
        acia_destroy(g_acia);
        return EXIT_FAILURE;
    }

    /* Set memory size and default clock frequency (1 MHz) */
    uint32_t memory_size   = 0x10000;
    double clock_frequency = 1000000.0;

    /* Initialize Bus */
    if (bus_init(&g_bus, memory_size, clock_frequency, g_acia, g_tia) != 0)
    {
        fprintf(stderr, "Error: Failed to initialize the Bus.\n");
        acia_destroy(g_acia);
        tia_destroy(g_tia);
        return EXIT_FAILURE;
    }

    /* Load initial program */
    if (bus_load_program(&g_bus, filename, start_address) != 0)
    {
        fprintf(stderr, "Error: Failed to load the program.\n");
        bus_destroy(&g_bus);
        return EXIT_FAILURE;
    }

    /* Set Reset Vector to 'start_address' */
    bus_write_memory(&g_bus, 0xFFFC, (uint8_t) (start_address & 0xFF));
    bus_write_memory(&g_bus, 0xFFFD, (uint8_t) ((start_address >> 8) & 0xFF));
    fprintf(stderr, "[Info] Reset Vector set to 0x%04X\n", start_address);

    /* Initialize CPU */
    if (cpu6502_init(&g_bus) != 0)
    {
        fprintf(stderr, "Error: Failed to initialize the CPU.\n");
        bus_destroy(&g_bus);
        return EXIT_FAILURE;
    }

    /* Reset CPU */
    if (cpu6502_reset() < 0)
    {
        fprintf(stderr, "Error: Failed to reset the CPU.\n");
        cpu6502_destroy();
        bus_destroy(&g_bus);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "\n[Info] 6502 Emulator Monitor\n");
    fprintf(stderr, "Type 'help' for available commands.\n");

    run_monitor_loop();

    /* Cleanup */
    cpu6502_destroy();
    bus_destroy(&g_bus);

    return EXIT_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * run_monitor_loop
 * -----------------------------------------------------------------------------
 */
static void run_monitor_loop(void)
{
    while (true)
    {
        printf("6502> ");
        fflush(stdout);

        char *user_line = read_line_from_stdin();

        if (!user_line)
        {
            printf("\n");
            break; /* EOF or error */
        }

        /* Trim leading spaces */
        char *p = user_line;

        while (*p && isspace((unsigned char) *p))
            p++;

        if (*p == '\0')
        {
            free(user_line);
            continue; /* empty line */
        }

        bool exit_requested = monitor_parse_command(p);
        free(user_line);

        if (exit_requested)
            break;
    }
}

/* -----------------------------------------------------------------------------
 * monitor_parse_command
 * -----------------------------------------------------------------------------
 */
static bool monitor_parse_command(const char *line)
{
    char cmd[64];
    int n = 0;

    if (sscanf(line, "%63s%n", cmd, &n) != 1)
        return false;

    /* tolower command */
    for (char *c = cmd; *c; c++)
        *c = (char) tolower(*c);

    /* skip to arguments */
    const char *args = line + n;

    while (*args && isspace((unsigned char) *args))
        args++;

    if (strcmp(cmd, "help") == 0)
    {
        print_help();
    }
    else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0)
    {
        return true;
    }
    else if (strcmp(cmd, "reset") == 0)
    {
        do_reset_cpu();
    }
    else if (strcmp(cmd, "step") == 0)
    {
        int steps = 1;
        sscanf(args, "%d", &steps);

        if (steps < 1)
            steps = 1;

        do_step(steps);
    }
    else if (strcmp(cmd, "run") == 0)
    {
        int steps = 0;
        if (sscanf(args, "%d", &steps) == 1 && steps > 0)
            do_run(steps);
        else
            do_run(-1);
    }
    else if ((strcmp(cmd, "mem") == 0) || (strcmp(cmd, "dump") == 0))
    {
        uint16_t addr = 0;
        int count     = 16;

        if (sscanf(args, "%hx %d", &addr, &count) >= 1)
            do_memdump(addr, count);
        else
            printf("Usage: %s <hex_address> [count]\n", cmd);
    }
    else if (strcmp(cmd, "pc") == 0)
    {
        uint16_t addr = 0;
        if (sscanf(args, "%hx", &addr) == 1)
        {
            cpu->pc = addr;
            printf("[OK] PC set to 0x%04X\n", addr);
        }
        else
        {
            printf("Usage: pc <hex_address>\n");
        }
    }
    else if (strcmp(cmd, "reg") == 0 || strcmp(cmd, "regs") == 0)
    {
        do_show_registers();
    }
    else if (strcmp(cmd, "serial") == 0)
    {
        if (*args)
            do_serial_in(args);
        else
            printf("Usage: serial <string>\n");
    }
    else if (strcmp(cmd, "irq") == 0)
    {
        do_irq();
    }
    else if (strcmp(cmd, "nmi") == 0)
    {
        do_nmi();
    }
    else if (strcmp(cmd, "stack") == 0)
    {
        int count = 16;
        sscanf(args, "%d", &count);

        if (count < 1)
            count = 16;

        do_stackdump(count);
    }
    else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0)
    {
        do_clear_screen();
    }
    else if (strcmp(cmd, "load") == 0)
    {
        /* load <filename> [addr] */
        char filename[256];
        uint16_t addr = 0xC000;
        int n2        = 0;

        if (sscanf(args, "%255s%n", filename, &n2) == 1)
        {
            const char *addr_str = args + n2;

            if (sscanf(addr_str, "%hx", &addr) != 1)
            {
                /* no valid address given, keep default */
            }

            if (do_load_program(filename, addr) == 0)
            {
                printf("[OK] Loaded '%s' at 0x%04X.\n", filename, addr);
                printf("Use 'pc 0x%04X' or 'reset' if needed.\n", addr);
            }
        }
        else
        {
            printf("Usage: load <filename> [addr]\n");
        }
    }
    else if ((strcmp(cmd, "disasm") == 0) || (strcmp(cmd, "dasm") == 0))
    {
        uint16_t addr = 0;
        int count     = 10;

        if (sscanf(args, "%hx %d", &addr, &count) >= 1)
            do_disassemble(addr, count);
        else
            printf("Usage: disasm <hex_address> [count]\n");
    }
    else if (strcmp(cmd, "clock") == 0)
    {
        do_clock_command(args);
    }
    else
    {
        printf("Unknown command: %s\n", cmd);
    }

    return false;
}

/* -----------------------------------------------------------------------------
 * print_help
 * -----------------------------------------------------------------------------
 */
static void print_help(void)
{
    printf("Available commands:\n");
    printf("  help                  - Show this help message\n");
    printf("  quit | q              - Quit the emulator\n");
    printf("  reset                 - Reset the CPU (PC from Reset Vector)\n");
    printf("  step [N]              - Execute N instructions (default = 1)\n");
    printf(
        "  run [N]               - Run continuously, optionally for N "
        "instructions\n");
    printf(
        "  mem <addr> [count]    - Hex dump memory in ASCII table, default "
        "count=16\n");
    printf("  dump <addr> [count]   - Alias for 'mem'\n");
    printf("  pc <addr>             - Set CPU PC to <addr>\n");
    printf("  reg (or regs)         - Show CPU registers\n");
    printf("  irq                   - Trigger an IRQ (if I-flag=0)\n");
    printf("  nmi                   - Trigger an NMI\n");
    printf(
        "  stack [N]             - Show top N bytes of the CPU stack (ASCII "
        "table)\n");
    printf(
        "  serial <str>          - Feed <str> into ACIA as if received on "
        "serial\n");
    printf("  clear | cls           - Clear the screen\n");
    printf("  load <file> [addr]    - Load a new binary into memory\n");
    printf(
        "  disasm <addr> [count] - Disassemble code from <addr> "
        "(default=10)\n");
    printf("  clock on/off          - Enable or disable the CPU clock\n");
    printf("  clock freq <value>    - Set a new clock frequency in Hz\n");
    printf("\nExamples:\n");
    printf("  step 10               - Step 10 instructions\n");
    printf("  pc 0xC000             - Set PC to 0xC000\n");
    printf(
        "  mem 0xC000 32         - Dump 32 bytes from 0xC000 in ASCII table\n");
    printf("  run 1000              - Run 1000 instructions\n");
    printf("  irq                   - Manually trigger an IRQ\n");
    printf("  load newprog.bin 0xE000\n");
    printf("  clock off             - Disable CPU clock (faster stepping)\n");
    printf("  clock freq 2000000    - 2 MHz\n");
}

/* -----------------------------------------------------------------------------
 * do_reset_cpu
 * -----------------------------------------------------------------------------
 */
static void do_reset_cpu(void)
{
    if (cpu6502_reset() == 0)
        printf("[OK] CPU reset done. PC = 0x%04X\n", cpu->pc);
    else
        printf("[ERROR] CPU reset failed.\n");
}

/* -----------------------------------------------------------------------------
 * do_step
 * -----------------------------------------------------------------------------
 */
static void do_step(int steps)
{
    for (int i = 0; i < steps; i++)
    {
        if (cpu->halted)
        {
            printf("[WARN] CPU is halted.\n");
            return;
        }

        int cycles = cpu6502_step();

        if (cycles < 0)
        {
            printf("[ERROR] CPU error or illegal opcode.\n");
            break;
        }

        acia_process_tx(g_acia);
        printf("Step #%d - PC=0x%04X - cycles=%d\n", i + 1, cpu->pc, cycles);
    }
}

/* -----------------------------------------------------------------------------
 * do_run
 * -----------------------------------------------------------------------------
 */
static void do_run(int steps)
{
    printf("[RUN] Start running. Press 'q' to break.\n");

    int count = 0;

    while (true)
    {
        if (cpu->halted)
        {
            printf("[INFO] CPU is halted.\n");
            break;
        }

        if (kbhit_nonblock())
        {
            int ch = read_char_nonblock();

            if (tolower(ch) == 'q')
            {
                printf("[RUN] Stopped by user.\n");
                break;
            }
        }

        int cycles = cpu6502_step();

        if (cycles < 0)
        {
            printf("[ERROR] CPU error or illegal opcode.\n");
            break;
        }

        acia_process_tx(g_acia);

        if (steps > 0)
        {
            count++;

            if (count >= steps)
            {
                printf("[RUN] Completed %d instructions.\n", steps);
                break;
            }
        }
    }
}

/* -----------------------------------------------------------------------------
 * do_memdump
 *
 * Prints 'count' bytes starting from 'start' in an ASCII table style.
 * -----------------------------------------------------------------------------
 */
static void do_memdump(uint16_t start, int count)
{
    const int bytes_per_line = 16;
    uint16_t end             = start + count - 1;

    printf("\n[Memory Dump] 0x%04X - 0x%04X (%d bytes)\n", start, end, count);

    /* Print ASCII table header */
    print_ascii_table_header(start);

    for (int i = 0; i < count; i += bytes_per_line)
    {
        uint8_t buffer[bytes_per_line];

        int line_len =
            (count - i < bytes_per_line) ? (count - i) : bytes_per_line;

        for (int b = 0; b < line_len; b++)
        {
            uint16_t addr = start + i + b;
            buffer[b]     = bus_read_memory(&g_bus, addr);
        }

        print_ascii_table_line(start + i, buffer, line_len);
    }

    printf("\n");
}

/* -----------------------------------------------------------------------------
 * do_show_registers
 *
 * Optional: you could also print registers in a neat ASCII table.
 * For now, we keep it simple with a plain text output.
 * -----------------------------------------------------------------------------
 */
static void do_show_registers(void)
{
    if (!cpu)
    {
        printf("[ERROR] CPU not initialized.\n");
        return;
    }

    printf("\n+====================== CPU Registers =====================+\n");
    printf(
        "| A = 0x%02X   X = 0x%02X   Y = 0x%02X   SP = 0x%02X   PC = 0x%04X |\n"
        "", cpu->a, cpu->x, cpu->y, cpu->sp, cpu->pc);
    printf(
        "| Status = N=%d V=%d U=%d B=%d D=%d I=%d Z=%d C=%d (0x%02X)          "
        "|\n",
        cpu->flag_n ? 1 : 0, cpu->flag_v ? 1 : 0,
        (cpu->status & 0x20) ? 1 : 0, /* Unused bit (U) */
        (cpu->status & 0x10) ? 1 : 0, /* Break (B) */
        cpu->flag_d ? 1 : 0, cpu->flag_i ? 1 : 0, cpu->flag_z ? 1 : 0,
        cpu->flag_c ? 1 : 0, cpu->status);
    printf("+==========================================================+\n\n");
}

/* -----------------------------------------------------------------------------
 * do_serial_in
 * -----------------------------------------------------------------------------
 */
static void do_serial_in(const char *input_str)
{
    if (!g_acia)
    {
        printf("[ERROR] No ACIA device.\n");
        return;
    }

    acia_provide_input(g_acia, input_str);
    printf("[OK] Provided serial input: \"%s\"\n", input_str);
}

/* -----------------------------------------------------------------------------
 * do_irq
 * -----------------------------------------------------------------------------
 */
static void do_irq(void)
{
    if (!cpu)
    {
        printf("[ERROR] CPU not initialized.\n");
        return;
    }

    int cycles = cpu6502_irq();

    if (cycles > 0)
        printf("[IRQ] IRQ triggered. PC=0x%04X, cycles=%d\n", cpu->pc, cycles);
    else if (cycles == 0)
        printf("[IRQ] IRQ ignored (I-flag is set).\n");
    else
        printf("[IRQ] Error triggering IRQ.\n");
}

/* -----------------------------------------------------------------------------
 * do_nmi
 * -----------------------------------------------------------------------------
 */
static void do_nmi(void)
{
    if (!cpu)
    {
        printf("[ERROR] CPU not initialized.\n");
        return;
    }

    int cycles = cpu6502_nmi();

    if (cycles >= 0)
        printf("[NMI] NMI triggered. PC=0x%04X, cycles=%d\n", cpu->pc, cycles);
    else
        printf("[NMI] Error triggering NMI.\n");
}

/* -----------------------------------------------------------------------------
 * do_stackdump
 *
 * Prints part of the stack in ASCII table style.
 * -----------------------------------------------------------------------------
 */
static void do_stackdump(int count)
{
    if (!cpu)
    {
        printf("[ERROR] CPU not initialized.\n");
        return;
    }

    uint16_t start_addr = 0x0100 + (uint16_t) (cpu->sp) + 1;
    uint16_t end_addr   = 0x01FF;

    if (start_addr > end_addr)
    {
        printf("[STACK] Stack empty. SP=0x%02X\n", cpu->sp);
        return;
    }

    /* Adjust 'count' if it exceeds the stack boundary */
    if (start_addr + count - 1 > end_addr)
        count = end_addr - start_addr + 1;

    if (count <= 0)
    {
        printf("[STACK] Nothing to dump. SP=0x%02X\n", cpu->sp);
        return;
    }

    uint16_t final_addr = start_addr + count - 1;
    printf("\n[STACK Dump] 0x%04X - 0x%04X (%d bytes)\n", start_addr,
           final_addr, count);

    /* Print ASCII table header */
    print_ascii_table_header(start_addr);

    /* We'll reuse the same style of printing as do_memdump */
    const int bytes_per_line = 16;

    for (int i = 0; i < count; i += bytes_per_line)
    {
        uint8_t buffer[bytes_per_line];
        int line_len =
            (count - i < bytes_per_line) ? (count - i) : bytes_per_line;

        for (int b = 0; b < line_len; b++)
        {
            uint16_t addr = start_addr + i + b;
            buffer[b]     = bus_read_memory(&g_bus, addr);
        }

        print_ascii_table_line(start_addr + i, buffer, line_len);
    }

    printf("\n");
}

/* -----------------------------------------------------------------------------
 * do_clear_screen
 * -----------------------------------------------------------------------------
 */
static void do_clear_screen(void)
{
    system(CLEAR_COMMAND);
}

/* -----------------------------------------------------------------------------
 * do_load_program
 * -----------------------------------------------------------------------------
 */
static int do_load_program(const char *filename, uint16_t address)
{
    int ret = bus_load_program(&g_bus, filename, address);

    if (ret != 0)
        printf("[ERROR] Failed to load '%s'.\n", filename);

    return ret;
}

/* -----------------------------------------------------------------------------
 * do_disassemble
 *
 * Very simple 6502 disassembler. You can expand this to handle more opcodes.
 * -----------------------------------------------------------------------------
 */
static void do_disassemble(uint16_t start, int count)
{
    uint16_t pc = start;

    printf("\n[Disassembly] Starting at 0x%04X, %d instructions...\n",
           start, count);

    for (int i = 0; i < count; i++)
    {
        uint8_t opcode = bus_read_memory(&g_bus, pc);
        printf("$%04X: ", pc);

        switch (opcode)
        {
            case 0xA9: // LDA #imm
            {
                uint8_t imm = bus_read_memory(&g_bus, pc + 1);
                printf("LDA #$%02X\n", imm);
                pc += 2;
            }
            break;

            case 0xA2: // LDX #imm
            {
                uint8_t imm = bus_read_memory(&g_bus, pc + 1);
                printf("LDX #$%02X\n", imm);
                pc += 2;
            }
            break;

            case 0xA0: // LDY #imm
            {
                uint8_t imm = bus_read_memory(&g_bus, pc + 1);
                printf("LDY #$%02X\n", imm);
                pc += 2;
            }
            break;

            case 0xEA: // NOP
            {
                printf("NOP\n");
                pc += 1;
            }
            break;

            case 0x00: // BRK
            {
                printf("BRK\n");
                pc += 1;
            }
            break;

            case 0x4C: // JMP absolute
            {
                uint8_t low   = bus_read_memory(&g_bus, pc + 1);
                uint8_t high  = bus_read_memory(&g_bus, pc + 2);
                uint16_t addr = (high << 8) | low;
                printf("JMP $%04X\n", addr);
                pc += 3;
            }
            break;

            default:
            {
                printf("??? (opcode $%02X)\n", opcode);
                pc += 1;
            }
            break;
        }
    }

    printf("\n");
}

/* -----------------------------------------------------------------------------
 * do_clock_command
 * -----------------------------------------------------------------------------
 */
static void do_clock_command(const char *args)
{
    char subcmd[64];

    if (sscanf(args, "%63s", subcmd) != 1)
    {
        printf("Usage:\n  clock on/off\n  clock freq <value>\n");
        return;
    }

    for (char *c = subcmd; *c; c++)
    {
        *c = (char) tolower(*c);
    }

    if (strcmp(subcmd, "on") == 0)
    {
        cpu->bus->clock_disabled = false;
        printf("[CLOCK] CPU clock enabled.\n");
    }
    else if (strcmp(subcmd, "off") == 0)
    {
        cpu->bus->clock_disabled = true;
        printf("[CLOCK] CPU clock disabled.\n");
    }
    else if (strcmp(subcmd, "freq") == 0)
    {
        double freq   = 0.0;
        const char *p = args + strlen("freq");

        /* skip spaces */
        while (*p && isspace((unsigned char) *p))
            p++;

        if (sscanf(p, "%lf", &freq) == 1 && freq > 0)
        {
            if (cpu->bus->clock)
            {
                cpu->bus->clock->frequency = freq;
                printf("[CLOCK] CPU clock frequency set to %f Hz.\n", freq);
            }
            else
            {
                printf("[ERROR] No clock object found in bus.\n");
            }
        }
        else
        {
            printf("Usage: clock freq <value_in_Hz>\n");
        }
    }
    else
    {
        printf("[ERROR] Unknown clock command '%s'.\n", subcmd);
        printf("Usage:\n  clock on/off\n  clock freq <value>\n");
    }
}

/* =============================================================================
 * ASCII Table Utility Functions
 * =============================================================================
 */

/**
 * @brief Print the header line for an ASCII memory table.
 *        Example:
 *           Address  | 00 01 02 03 04 05 ... 0F | ASCII
 *           ------------------------------------------------
 */
static void print_ascii_table_header(uint16_t start_addr)
{
    (void) start_addr; /* We don't necessarily need 'start_addr' here, but you
                          can use if needed. */
    printf("+---------+-------------------------------------------------+\n");
    printf("| Address |");
    for (int i = 0; i < 16; i++)
        printf(" %02X", i);
    printf(" | ASCII\n");
    printf("+---------+-------------------------------------------------+\n");
}

/**
 * @brief Print one line of an ASCII memory table, including hex and ASCII
 * columns.
 *
 * @param addr   The starting address for this line
 * @param data   Pointer to the bytes to print
 * @param length Number of valid bytes in 'data'
 *
 * Example line:
 *   0xC000 | 45 00 9A FF 20 20 ...       | E..  ...
 */
static void print_ascii_table_line(uint16_t addr, const uint8_t *data,
                                   int length)
{
    int bytes_per_line = 16;

    /* Print address in hex */
    printf("| 0x%04X: |", addr);

    /* Print hex values */
    for (int i = 0; i < bytes_per_line; i++)
    {
        if (i < length)
            printf(" %02X", data[i]);
        else
            printf("   "); /* spacing if no data */
    }

    printf(" | ");

    /* Print ASCII representation */
    for (int i = 0; i < length; i++)
    {
        uint8_t c = data[i];
        if (c < 0x20 || c > 0x7E)
            c = '.'; /* non-printable => '.' */
        printf("%c", c);
    }

    printf("\n");
}

/* -----------------------------------------------------------------------------
 * read_line_from_stdin
 * -----------------------------------------------------------------------------
 */
static char *read_line_from_stdin(void)
{
    char buffer[1024];

    if (!fgets(buffer, sizeof(buffer), stdin))
        return NULL;

    char *newline = strchr(buffer, '\n');

    if (newline)
        *newline = '\0';

    return strdup(buffer);
}
