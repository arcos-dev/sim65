/**
 * @file cpu.h
 * @brief MOS6502 CPU Emulator (header)
 *
 * Derived from fake6502 by Mike Chambers, Ivo van Poorten and others.
 * Cycle-accurate emulation including undocumented opcodes.
 *
 * Licensed under the 2-clause BSD license.
 */

#ifndef CPU_H
#define CPU_H

#include "bus.h"
#include <stdbool.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * Constants: base addresses and vectors
 * -----------------------------------------------------------------------------
 * Using static const instead of #define for typed constants.
 */
static const uint16_t STACK_BASE   = 0x0100;
static const uint16_t RESET_VECTOR = 0xFFFC;
static const uint16_t NMI_VECTOR   = 0xFFFA;
static const uint16_t IRQ_VECTOR   = 0xFFFE;

/**
 * @brief Each entry in this unified opcode table holds:
 *  - The addressing mode function (addr_mode)
 *  - The opcode function (opcode_func)
 *  - The base clock cycle count (cycles)
 */
typedef void (*addr_mode_func_t)(void);
typedef void (*opcode_func_t)(void);

typedef struct
{
    addr_mode_func_t addr_mode;
    opcode_func_t opcode_func;
    uint8_t cycles;
} opcode_t;

/**
 * @brief 6502 CPU structure containing all CPU registers and state.
 *
 * The CPU core has:
 *  - 16-bit Program Counter (pc)
 *  - 8-bit Stack Pointer (sp)
 *  - 8-bit Accumulator (a)
 *  - 8-bit X Register (x)
 *  - 8-bit Y Register (y)
 *  - 8-bit status flags (status) [NV-BDIZC]
 *
 * In addition, we store:
 *  - internal flag copies (flag_c, flag_z, etc.) for fast checking
 *  - some cycle / addressing state
 *  - halted flag for illegal opcodes that freeze the CPU
 */
typedef struct
{
    bus_t *bus;              ///< Pointer to the bus
    uint8_t a, x, y;         ///< Registers
    uint8_t sp;              ///< Stack pointer
    uint16_t pc;             ///< Program counter
    uint8_t status;          ///< Processor status flags

    /* Internal flags (mirror bits from status, used for quick access) */
    bool flag_c;
    bool flag_z;
    bool flag_i;
    bool flag_d;
    bool flag_v;
    bool flag_n;

    uint16_t effective_addr; ///< Computed by addressing modes
    uint8_t current_opcode;  ///< Current opcode being executed
    int penalty_opcode;      ///< Penalty for opcode
    int penalty_address;     ///< Penalty for address mode
    double cycles;           ///< Cycles used by current instruction
    bool halted;             ///< Halt flag
} cpu6502_t;

/* Global pointer to the CPU instance (optional if you want a single CPU). */
extern cpu6502_t *cpu;

/**
 * @brief CPU status manipulation
 *
 * Set or get the CPU status register as an 8-bit (NV-BDIZC).
 * - bit 5 is always '1' when read
 * - bit 4 (Break) is not stored internally, returned as 0 on read
 */
void cpu6502_set_status(uint8_t value);
uint8_t cpu6502_get_status(void);

/*
 * CPU lifecycle:
 * - cpu6502_init: allocate and initialize CPU, set PC from reset vector
 * - cpu6502_destroy: free CPU instance
 * - cpu6502_reset: reset CPU state
 * - cpu6502_nmi, cpu6502_irq: handle interrupts
 * - cpu6502_step: execute a single opcode
 */

/**
 * @brief Allocate and initialize the CPU instance, set PC from reset vector.
 * @param bus Pointer to the bus structure.
 * @return 0 on success, nonzero on failure (malloc error).
 */
int cpu6502_init(bus_t *bus);

/**
 * @brief Reset the CPU. Clears registers and reloads PC from reset vector.
 * @return Number of cycles that the reset operation takes (7).
 */
int cpu6502_reset(void);

/**
 * @brief NMI (Non-Maskable Interrupt).
 * @return Number of cycles used by the NMI (7).
 */
int cpu6502_nmi(void);

/**
 * @brief IRQ (Interrupt Request), only if I=0.
 * @return Number of cycles used by the IRQ (7).
 */
int cpu6502_irq(void);

/**
 * @brief Execute a single instruction at the current PC.
 * @return Number of clock cycles used by that instruction (converted to int).
 */
int cpu6502_step(void);

/**
 * @brief Free the CPU instance. Must be called when you are done with the CPU.
 */
void cpu6502_destroy(void);

#endif /* CPU_H */
