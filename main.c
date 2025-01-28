/*******************************************************************************
 * main.c
 *
 * Advanced example of an interactive “monitor” for a 6502 emulator.
 *
 * It allows:
 *  - Loading a binary into memory
 *  - Setting and changing the Reset Vector
 *  - Resetting the CPU
 *  - Stepping one or more instructions
 *  - Running continuously
 *  - Viewing/modifying memory
 *  - Viewing/modifying CPU registers
 *  - Triggering IRQ / NMI interrupts
 *  - Viewing the stack content
 *  - Injecting serial data into ACIA
 *  - Processing ACIA TX data to stdout
 *  - Exiting the emulator
 *
 * Author: Anderson Costa
 * Date: 2025-01-27
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
  #include <conio.h>      // For _kbhit() and _getch() on Windows
#else
  #include <sys/select.h> // For select() on Unix-like
  #include <unistd.h>     // For read(), STDIN_FILENO, etc.
#endif

#include "acia.h"
#include "bus.h"
#include "cpu.h"
#include "palette.h"
#include "tia.h"

/* -----------------------------------------------------------------------------
 * Global Pointers
 * -----------------------------------------------------------------------------
 * The bus connects the CPU, memory, clock, and optional I/O devices.
 */
static bus_t g_bus;
static Acia6550 *g_acia = NULL;
static TIA *g_tia       = NULL;

/* -----------------------------------------------------------------------------
 * Forward Declarations
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Start the interactive monitor (REPL).
 */
static void monitor_loop(void);

/**
 * @brief Print a brief list of commands.
 */
static void print_help(void);

/**
 * @brief Execute a single CPU instruction (step), printing optional info.
 */
static void do_step(int steps);

/**
 * @brief Run the CPU for many instructions or until halted.
 */
static void do_run(int steps);

/**
 * @brief Display memory in a hex dump format.
 */
static void do_memdump(uint16_t start, int count);

/**
 * @brief Display CPU registers (A, X, Y, SP, PC, Status).
 */
static void do_show_registers(void);

/**
 * @brief Provide user input string to ACIA (serial RX).
 */
static void do_serial_in(const char *input_str);

/**
 * @brief Trigger an IRQ interrupt if not masked.
 */
static void do_irq(void);

/**
 * @brief Trigger an NMI interrupt.
 */
static void do_nmi(void);

/**
 * @brief Display stack content from the current SP to the top of page 1.
 */
static void do_stackdump(int count);

/**
 * @brief Read one command line from stdin (blocking).
 */
static char *read_line(void);

/**
 * @brief Parse and execute a command typed in the monitor.
 * @return true if user wants to quit.
 */
static bool parse_command(const char *line);

/* -----------------------------------------------------------------------------
 * Cross-platform "kbhit" and "read_char" for run loop
 * -----------------------------------------------------------------------------
 */
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
 * Main
 * -----------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
    // Basic usage
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <program.bin> <start_address>\n", argv[0]);
        fprintf(stderr, "Example: %s hello.bin 0xC000\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse input
    const char *filename = argv[1];
    char *endptr;

    uint16_t start_address = (uint16_t) strtol(argv[2], &endptr, 16);
    if (*endptr != '\0')
    {
        fprintf(stderr, "Invalid start address format (use e.g. 0x8000)\n");
        return EXIT_FAILURE;
    }

    // Initialize ACIA
    g_acia = acia_init();

    if (!g_acia)
    {
        fprintf(stderr, "Error: Failed to allocate ACIA.\n");
        return EXIT_FAILURE;
    }

    // Initialize TIA (optional)
    g_tia = tia_init(TV_SYSTEM_NTSC);

    if (!g_tia)
    {
        fprintf(stderr, "Error: Failed to allocate TIA.\n");
        acia_destroy(g_acia);
        return EXIT_FAILURE;
    }

    // Define memory size and clock frequency
    uint32_t memory_size   = 0x10000;   // 64KB
    double clock_frequency = 1000000.0; // 1 MHz

    // Initialize the Bus
    if (bus_init(&g_bus, memory_size, clock_frequency, g_acia, g_tia) != 0)
    {
        fprintf(stderr, "Error: Failed to initialize the Bus.\n");
        acia_destroy(g_acia);
        tia_destroy(g_tia);
        return EXIT_FAILURE;
    }

    // Load the program into memory
    if (bus_load_program(&g_bus, filename, start_address) != 0)
    {
        fprintf(stderr, "Error: Failed to load the program.\n");
        bus_destroy(&g_bus);
        return EXIT_FAILURE;
    }

    // Set the Reset Vector to 'start_address'
    bus_write_memory(&g_bus, 0xFFFC, (uint8_t) (start_address & 0xFF));
    bus_write_memory(&g_bus, 0xFFFD, (uint8_t) ((start_address >> 8) & 0xFF));
    fprintf(stderr, "[Info] Reset Vector set to 0x%04X\n", start_address);

    // Initialize the CPU
    if (cpu6502_init(&g_bus) != 0)
    {
        fprintf(stderr, "Error: Failed to initialize the CPU.\n");
        bus_destroy(&g_bus);
        return EXIT_FAILURE;
    }

    // Reset the CPU (loads PC from 0xFFFC)
    if (cpu6502_reset() < 0)
    {
        fprintf(stderr, "Error: Failed to reset the CPU.\n");
        cpu6502_destroy();
        bus_destroy(&g_bus);
        return EXIT_FAILURE;
    }

    // Enter the interactive monitor loop
    fprintf(stderr, "\n[Info] 6502 Emulator Monitor\n");
    fprintf(stderr, "Type 'help' for available commands.\n");
    monitor_loop();

    // Cleanup
    cpu6502_destroy();
    bus_destroy(&g_bus);

    return EXIT_SUCCESS;
}

/* -----------------------------------------------------------------------------
 * Runs an interactive command prompt (REPL). It waits for user commands and
 * executes them until the user requests to quit.
 * -----------------------------------------------------------------------------
 */
static void monitor_loop(void)
{
    while (true)
    {
        printf("6502> ");
        fflush(stdout);

        char *line = read_line();

        if (!line)
        {
            // EOF or error
            printf("\n");
            break;
        }

        // Trim whitespace
        char *p = line;
        while (*p && isspace((unsigned char) *p))
            p++;

        if (*p == '\0')
        {
            // Empty line
            free(line);
            continue;
        }

        // Parse and execute command
        bool exit_requested = parse_command(p);
        free(line);

        if (exit_requested)
        {
            break; // user typed quit/exit
        }
    }
}

/* -----------------------------------------------------------------------------
 * Parses the command line and executes the corresponding function.
 * Returns true if the user wants to exit, false otherwise.
 * -----------------------------------------------------------------------------
 */
static bool parse_command(const char *line)
{
    // We'll tokenize the line
    // e.g. "step 10", "mem 0xC000 32", "pc 0x2000", "run", etc.
    char cmd[64];
    int n = 0;

    // Copy first token into cmd
    if (sscanf(line, "%63s%n", cmd, &n) != 1)
        return false; // can't parse

    // Convert cmd to lowercase for easier matching
    for (char *c = cmd; *c; c++)
        *c = (char) tolower(*c);

    // Move pointer forward
    const char *args = line + n;
    while (*args && isspace((unsigned char) *args))
        args++;

    // Check recognized commands
    if (strcmp(cmd, "help") == 0)
    {
        print_help();
    }
    else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0)
    {
        return true;
    }
    else if (strcmp(cmd, "reset") == 0)
    {
        cpu6502_reset();
        printf("[OK] CPU reset done. PC = 0x%04X\n", cpu->pc);
    }
    else if (strcmp(cmd, "step") == 0)
    {
        // step N
        int steps = 1;
        sscanf(args, "%d", &steps);
        if (steps < 1)
            steps = 1;
        do_step(steps);
    }
    else if (strcmp(cmd, "run") == 0)
    {
        // run N or run indefinite
        int steps = 0;
        if (sscanf(args, "%d", &steps) == 1 && steps > 0)
        {
            do_run(steps);
        }
        else
        {
            // run until CPU halted
            do_run(-1);
        }
    }
    else if (strcmp(cmd, "mem") == 0)
    {
        // mem <addr> <count>
        uint16_t addr = 0;
        int count     = 16;
        if (sscanf(args, "%hx %d", &addr, &count) >= 1)
        {
            do_memdump(addr, count);
        }
        else
        {
            printf("Usage: mem <hex_address> [count]\n");
        }
    }
    else if (strcmp(cmd, "pc") == 0)
    {
        // pc <addr>
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
        // serial <string to feed>
        if (*args)
        {
            do_serial_in(args);
        }
        else
        {
            printf("Usage: serial <string>\n");
        }
    }
    else if (strcmp(cmd, "irq") == 0)
    {
        // Trigger IRQ
        do_irq();
    }
    else if (strcmp(cmd, "nmi") == 0)
    {
        // Trigger NMI
        do_nmi();
    }
    else if (strcmp(cmd, "stack") == 0)
    {
        // stack <count>
        int count = 16;
        sscanf(args, "%d", &count);
        if (count < 1)
            count = 16;
        do_stackdump(count);
    }
    else
    {
        printf("Unknown command: %s\n", cmd);
    }

    return false; // continue in monitor
}

/* -----------------------------------------------------------------------------
 * Print help in Terminal
 * -----------------------------------------------------------------------------
 */
static void print_help(void)
{
    printf("Available commands:\n");
    printf("  help           - Show this help message\n");
    printf("  quit | exit    - Quit the emulator\n");
    printf("  reset          - Reset the CPU (PC from Reset Vector)\n");
    printf("  step [N]       - Execute N instructions (default = 1)\n");
    printf(
        "  run [N]        - Run continuously, optionally for N instructions\n");
    printf(
        "                   (if N is -1 or omitted, run until CPU halted)\n");
    printf("  mem <addr> [c] - Hex dump memory from <addr>; c=16 by default\n");
    printf("  pc <addr>      - Set CPU PC to <addr>\n");
    printf("  reg            - Show CPU registers\n");
    printf("  irq            - Trigger an IRQ interrupt (if not masked)\n");
    printf("  nmi            - Trigger a Non-Maskable Interrupt\n");
    printf(
        "  stack [N]      - Show top N bytes of the CPU stack (default=16)\n");
    printf(
        "  serial <str>   - Feed <str> into ACIA as if received on serial\n");
    printf("\nExamples:\n");
    printf("  step 10        - Step 10 instructions\n");
    printf("  pc 0xC000      - Set PC to 0xC000\n");
    printf("  mem 0xC000 32  - Hex dump 32 bytes from 0xC000\n");
    printf("  run 1000       - Run 1000 instructions\n");
    printf("  run            - Run indefinitely until halted\n");
    printf("  irq            - Manually trigger an IRQ\n");
    printf("  nmi            - Manually trigger an NMI\n");
    printf("  stack 32       - Show 32 bytes of the stack\n");
    printf("  serial Hello   - Simulate receiving 'Hello' on serial\n");
}

/* -----------------------------------------------------------------------------
 * Steps the CPU for 'steps' instructions, printing info about each step.
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

        // Process ACIA TX
        acia_process_tx(g_acia);

        // Optional: show the current PC and/or opcode info
        printf("Step #%d - PC=0x%04X - cycles=%d\n", i + 1, cpu->pc, cycles);
    }
}

/* -----------------------------------------------------------------------------
 * Runs the CPU either for 'steps' instructions, or indefinitely if steps = -1.
 * While running, we check if the user presses 'q' to break.
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

        // Optionally check if user pressed 'q'
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

        // Process ACIA TX buffer
        acia_process_tx(g_acia);

        // If we are running for a fixed number of instructions, check count
        if (steps > 0)
        {
            count++;
            if (count >= steps)
            {
                printf("[RUN] Completed %d instructions.\n", steps);
                break;
            }
        }

        // If steps < 0, we run indefinitely until CPU halts or user breaks
    }
}

/* -----------------------------------------------------------------------------
 * Prints 'count' bytes from 'start' in a hex dump style (16 bytes per line).
 * -----------------------------------------------------------------------------
 */
static void do_memdump(uint16_t start, int count)
{
    const int bytes_per_line = 16;

    for (int i = 0; i < count; i++)
    {
        uint16_t addr = start + i;
        if ((i % bytes_per_line) == 0)
            printf("\n0x%04X:", addr);

        uint8_t val = bus_read_memory(&g_bus, addr);
        printf(" %02X", val);
    }

    printf("\n");
}

/* -----------------------------------------------------------------------------
 * Shows the CPU registers (A, X, Y, SP, PC, Status).
 * -----------------------------------------------------------------------------
 */
static void do_show_registers(void)
{
    if (!cpu)
    {
        printf("[ERROR] CPU not initialized.\n");
        return;
    }

    printf("A=0x%02X  X=0x%02X  Y=0x%02X  SP=0x%02X  PC=0x%04X\n", cpu->a,
           cpu->x, cpu->y, cpu->sp, cpu->pc);

    // We can expand to show status flags
    printf("Status = N=%d V=%d -=%d B=%d D=%d I=%d Z=%d C=%d (0x%02X)\n",
           cpu->flag_n ? 1 : 0, cpu->flag_v ? 1 : 0,
           (cpu->status & 0x20) ? 1 : 0, // bit 5
           (cpu->status & 0x10) ? 1 : 0, // bit 4
           cpu->flag_d ? 1 : 0, cpu->flag_i ? 1 : 0, cpu->flag_z ? 1 : 0,
           cpu->flag_c ? 1 : 0, cpu->status);
}

/* -----------------------------------------------------------------------------
 * Provide user-provided string as if it arrived via serial.
 * This appends the string to the ACIA RX buffer.
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
 * Manually trigger an IRQ interrupt. Will only take effect if I=0
 * (flag_i=false).
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
    {
        printf("[IRQ] IRQ triggered. PC=0x%04X, cycles=%d\n", cpu->pc, cycles);
    }
    else if (cycles == 0)
    {
        printf("[IRQ] IRQ ignored (I-flag is set).\n");
    }
    else
    {
        printf("[IRQ] Error triggering IRQ.\n");
    }
}

/* -----------------------------------------------------------------------------
 * Manually trigger an NMI interrupt. Always takes effect.
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
    {
        printf("[NMI] NMI triggered. PC=0x%04X, cycles=%d\n", cpu->pc, cycles);
    }
    else
    {
        printf("[NMI] Error triggering NMI.\n");
    }
}

/* -----------------------------------------------------------------------------
 * Display some bytes from the stack page (0x0100..0x01FF).
 * By default it shows 'count' bytes starting from SP+1 up to the top of page 1.
 * -----------------------------------------------------------------------------
 */
static void do_stackdump(int count)
{
    if (!cpu)
    {
        printf("[ERROR] CPU not initialized.\n");
        return;
    }

    // The 6502 stack is in page 1 (0x0100..0x01FF)
    // SP points to the next free byte (descending)
    // The topmost pushed byte is at 0x0100 + SP + 1
    uint16_t start_addr = 0x0100 + (uint16_t) (cpu->sp) + 1;

    // Ensure we don't go past 0x01FF
    uint16_t end_addr = 0x01FF;

    // Adjust count if it exceeds the remaining stack space
    if (start_addr + count - 1 > end_addr)
    {
        count = (int) (end_addr - start_addr + 1);
    }

    if (count <= 0)
    {
        printf("[STACK] Nothing to dump. SP=0x%02X\n", cpu->sp);
        return;
    }

    printf("[STACK] Dumping %d bytes from 0x%04X to 0x%04X\n", count,
           start_addr, start_addr + count - 1);

    do_memdump(start_addr, count);
}

/* -----------------------------------------------------------------------------
 * Read a line from stdin (blocking). Returns a heap-allocated string that must
 * be freed by the caller, or NULL on EOF or error.
 * -----------------------------------------------------------------------------
 */
static char *read_line(void)
{
    char buffer[1024];

    if (!fgets(buffer, sizeof(buffer), stdin))
    {
        // EOF or error
        return NULL;
    }

    // Strip newline
    char *newline = strchr(buffer, '\n');

    if (newline)
        *newline = '\0';

    return strdup(buffer);
}
