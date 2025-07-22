/**
 * @file cpu.c
 * @brief MOS6502 CPU Emulator Core
 *
 * Derived from fake6502 by Mike Chambers, Ivo van Poorten and others.
 * Cycle-accurate emulation including undocumented opcodes.
 *
 * Licensed under the 2-clause BSD license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

/* -----------------------------------------------------------------------------
 * Global CPU Instance
 * -----------------------------------------------------------------------------
 */
cpu6502_t *cpu = NULL;

/* -----------------------------------------------------------------------------
 * Global Opcode Table Pointer, filled by opcode_table_init()
 * -----------------------------------------------------------------------------
 */
static opcode_t *opcode_table = NULL;

/* -----------------------------------------------------------------------------
 * Inline helpers: memory stack operations, reading words, etc.
 * -----------------------------------------------------------------------------
 */
static inline void push8(uint8_t value)
{
    bus_write_memory(cpu->bus, STACK_BASE + (cpu->sp--), value);
}

static inline uint8_t pull8(void)
{
    return bus_read_memory(cpu->bus, STACK_BASE + (++cpu->sp));
}

static inline void push16(uint16_t value)
{
    /* Push high byte first */
    bus_write_memory(cpu->bus, STACK_BASE + cpu->sp, (uint8_t)((value >> 8) & 0xFF));
    bus_write_memory(cpu->bus, STACK_BASE + ((cpu->sp - 1) & 0xFF),
                     (uint8_t)(value & 0xFF));
    cpu->sp -= 2;
}

static inline uint16_t pull16(void)
{
    /* Pull low byte first */
    cpu->sp += 2;
    uint8_t lo = bus_read_memory(cpu->bus, STACK_BASE + ((cpu->sp - 1) & 0xFF));
    uint8_t hi = bus_read_memory(cpu->bus, STACK_BASE + (cpu->sp & 0xFF));
    return (hi << 8) | lo;
}

static inline uint16_t read_word(uint16_t address)
{
    uint8_t lo = bus_read_memory(cpu->bus, address);
    uint8_t hi = bus_read_memory(cpu->bus, address + 1);
    return (uint16_t) (lo | (hi << 8));
}

/* -----------------------------------------------------------------------------
 * Flag Calculation Helpers
 * -----------------------------------------------------------------------------
 */
static inline void calc_flag_z(uint8_t val)
{
    cpu->flag_z = (val == 0);
}

static inline void calc_flag_n(uint8_t val)
{
    cpu->flag_n = ((val & 0x80) != 0);
}

static inline void calc_flags_zn(uint8_t val)
{
    calc_flag_z(val);
    calc_flag_n(val);
}

static inline void calc_flag_c(uint16_t val)
{
    cpu->flag_c = (val > 0xFF);
}

static inline void calc_flags_czn(uint16_t val)
{
    calc_flag_c(val);
    calc_flags_zn((uint8_t) val);
}

static inline void calc_flag_v(uint16_t result, uint8_t accu, uint8_t operand)
{
    /* Overflow occurs if sign bits of operands are the same and different from
     * result */
    cpu->flag_v = (((~(accu ^ operand)) & (accu ^ result)) & 0x80) != 0;
}

/* Compare helper: update flags by comparing register with operand */
static inline void compare(uint8_t reg, uint8_t operand)
{
    uint8_t result = reg - operand;
    cpu->flag_c    = (reg >= operand);
    calc_flags_zn(result);
}

/* -----------------------------------------------------------------------------
 * CPU Status Register Set/Get
 * -----------------------------------------------------------------------------
 */
void cpu6502_set_status(uint8_t value)
{
    cpu->flag_n = (value & 0x80) != 0;
    cpu->flag_v = (value & 0x40) != 0;
    /* bit 5 is ignored (always 1 when read). */
    /* bit 4 (Break) is ignored internally. */
    cpu->flag_d = (value & 0x08) != 0;
    cpu->flag_i = (value & 0x04) != 0;
    cpu->flag_z = (value & 0x02) != 0;
    cpu->flag_c = (value & 0x01) != 0;
}

/**
 * @brief Get the processor status flags as a single byte.
 * @return The status flags byte.
 */
uint8_t cpu6502_get_status(void)
{
    uint8_t status = 0;
    status |= cpu->flag_c ? 0x01 : 0x00;
    status |= cpu->flag_z ? 0x02 : 0x00;
    status |= cpu->flag_i ? 0x04 : 0x00;
    status |= cpu->flag_d ? 0x08 : 0x00;
    status |= 0x20; // Unused bit is always set
    status |= cpu->flag_v ? 0x40 : 0x00;
    status |= cpu->flag_n ? 0x80 : 0x00;
    return status;
}

/* -----------------------------------------------------------------------------
 * Addressing Mode Functions
 * -----------------------------------------------------------------------------
 * These functions compute cpu->effective_addr and may set penalty_address
 * if a page boundary is crossed on absolute_x/absolute_y/indirect_indexed.
 * -----------------------------------------------------------------------------
 */
static void am_implied(void)
{
    /* No effective address. */
}

static void am_accumulator(void)
{
    /* Operand is in cpu->a. */
}

static void am_immediate(void)
{
    cpu->effective_addr = cpu->pc++;
}

static void am_zero_page(void)
{
    cpu->effective_addr = bus_read_memory(cpu->bus, cpu->pc++);
}

static void am_zero_page_x(void)
{
    cpu->effective_addr =
        (uint8_t) (bus_read_memory(cpu->bus, cpu->pc++) + cpu->x);
}

static void am_zero_page_y(void)
{
    cpu->effective_addr =
        (uint8_t) (bus_read_memory(cpu->bus, cpu->pc++) + cpu->y);
}

static void am_absolute(void)
{
    cpu->effective_addr = read_word(cpu->pc);
    cpu->pc += 2;
}

static void am_relative(void)
{
    uint16_t branch_pc  = cpu->pc;
    int8_t offset       = (int8_t) bus_read_memory(cpu->bus, cpu->pc++);
    cpu->effective_addr = (uint16_t)(branch_pc + 1 + offset);
}

static void am_absolute_x(void)
{
    uint16_t base_addr = read_word(cpu->pc);
    uint16_t page      = base_addr & 0xFF00;
    base_addr += cpu->x;
    cpu->penalty_address = (page != (base_addr & 0xFF00));
    cpu->effective_addr  = base_addr;
    cpu->pc += 2;
}

static void am_absolute_y(void)
{
    uint16_t base_addr = read_word(cpu->pc);
    uint16_t page      = base_addr & 0xFF00;
    base_addr += cpu->y;
    cpu->penalty_address = (page != (base_addr & 0xFF00));
    cpu->effective_addr  = base_addr;
    cpu->pc += 2;
}

static void am_indirect(void)
{
    uint16_t pointer    = read_word(cpu->pc);
    uint16_t pointer_hi = (pointer & 0xFF00) | ((pointer + 1) & 0x00FF);
    cpu->effective_addr =
        (uint16_t) bus_read_memory(cpu->bus, pointer) |
        ((uint16_t) bus_read_memory(cpu->bus, pointer_hi) << 8);
    cpu->pc += 2;
}

static void am_indexed_indirect(void)
{
    uint8_t ptr = (uint8_t) (bus_read_memory(cpu->bus, cpu->pc++) + cpu->x);
    uint8_t lo  = bus_read_memory(cpu->bus, ptr);
    uint8_t hi  = bus_read_memory(cpu->bus, (ptr + 1) & 0xFF);
    cpu->effective_addr = (uint16_t) lo | ((uint16_t) hi << 8);
}

static void am_indirect_indexed(void)
{
    uint8_t ptr = bus_read_memory(cpu->bus, cpu->pc++);
    uint16_t base =
        (uint16_t) bus_read_memory(cpu->bus, ptr) |
        ((uint16_t) bus_read_memory(cpu->bus, (ptr + 1) & 0xFF) << 8);
    uint16_t page = base & 0xFF00;
    base += cpu->y;
    cpu->penalty_address = (page != (base & 0xFF00));
    cpu->effective_addr  = base;
}

/* -----------------------------------------------------------------------------
 * Operand Get/Put Helpers
 * -----------------------------------------------------------------------------
 */
static inline uint8_t get_operand(addr_mode_func_t mode)
{
    if (mode == am_accumulator)
        return cpu->a;
    return bus_read_memory(cpu->bus, cpu->effective_addr);
}

static inline void put_operand(addr_mode_func_t mode, uint8_t value)
{
    if (mode == am_accumulator)
    {
        cpu->a = value;
    }
    else
    {
        bus_write_memory(cpu->bus, cpu->effective_addr, value);
    }
}

/* -----------------------------------------------------------------------------
 * Branch Helper
 * -----------------------------------------------------------------------------
 */
static inline void branch(bool condition)
{
    if (condition)
    {
        uint16_t old_pc = cpu->pc;
        cpu->pc         = cpu->effective_addr;
        cpu->cycles += 1; /* Adds 1 cycle if branch is taken */

        /* Add +1 cycle if there was page crossing */
        if ((old_pc & 0xFF00) != (cpu->pc & 0xFF00))
        {
            cpu->cycles += 1;
        }
    }
}

/* -----------------------------------------------------------------------------
 * Opcode Implementations (Official + Unofficial)
 * -----------------------------------------------------------------------------
 * We'll list them all, including their combos.
 * Some combine multiple calls (e.g. op_slo = op_asl + op_ora).
 */

/* Forward Declarations for all op_??? used in the table */
static void op_brk(void);
static void op_php(void);
static void op_bpl(void);
static void op_clc(void);
static void op_jsr(void);
static void op_bit(void);
static void op_plp(void);
static void op_rol(void);
static void op_bmi(void);
static void op_sec(void);
static void op_rti(void);
static void op_and(void);
static void op_eor(void);
static void op_ora(void);
static void op_bcc(void);
static void op_bcs(void);
static void op_pha(void);
static void op_lsr(void);
static void op_jmp(void);
static void op_bvc(void);
static void op_cli(void);
static void op_rts(void);
static void op_adc(void);
static void op_pla(void);
static void op_ror(void);
static void op_bvs(void);
static void op_sei(void);
static void op_sty(void);
static void op_stx(void);
static void op_dey(void);
static void op_txa(void);
static void op_sta(void);
static void op_tya(void);
static void op_txs(void);
static void op_ldy(void);
static void op_lda(void);
static void op_ldx(void);
static void op_tay(void);
static void op_tsx(void);
static void op_tax(void);
static void op_clv(void);
static void op_cpy(void);
static void op_cmp(void);
static void op_dec(void);
static void op_iny(void);
static void op_dex(void);
static void op_bne(void);
static void op_cld(void);
static void op_cpx(void);
static void op_sbc(void);
static void op_inc(void);
static void op_asl(void);
static void op_inx(void);
static void op_beq(void);
static void op_sed(void);
static void op_nop(void);
static void op_anc(void);
static void op_alr(void);
static void op_arr(void);
static void op_ane(void);
static void op_lxa(void);
static void op_sbx(void);
static void op_jam(void);
static void op_slo(void);
static void op_rla(void);
static void op_sre(void);
static void op_rra(void);
static void op_sax(void);
static void op_lax(void);
static void op_dcp(void);
static void op_isc(void);
static void op_sha(void);
static void op_shx(void);
static void op_shy(void);
static void op_tas(void);
static void op_las(void);

/* ------------------------- Opcode Implementations ------------------------- */

/* Official: BRK - Force Interrupt */
static void op_brk(void)
{
    push16(++cpu->pc); /* Push address of next instruction */
    op_php();
    cpu->flag_i = true;
    cpu->pc     = read_word(IRQ_VECTOR); /* Use IRQ vector for BRK */
}

/* Official: PHP - Push Processor Status */
static void op_php(void)
{
    /* Push status with B=1 */
    push8(cpu6502_get_status() | 0x10);
}

/* Official: BPL - Branch on Plus */
static void op_bpl(void)
{
    branch(!cpu->flag_n);
}

/* Official: CLC - Clear Carry Flag */
static void op_clc(void)
{
    cpu->flag_c = false;
}

/* Official: JSR - Jump to Subroutine */
static void op_jsr(void)
{
    /* Push (PC - 1) then set PC to target address */
    push16(cpu->pc - 1);
    cpu->pc = cpu->effective_addr;
}

/* Official: BIT - Test Bits */
static void op_bit(void)
{
    uint8_t operand = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    calc_flags_zn(cpu->a & operand);
    cpu->flag_n = ((operand & 0x80) != 0);
    cpu->flag_v = ((operand & 0x40) != 0);
}

/* Official: PLP - Pull Processor Status */
static void op_plp(void)
{
    uint8_t p = pull8();
    cpu6502_set_status(p);
    /* Bit 5 is ignored, bit 4 is ignored. We keep them consistent though. */
}

/* Official: ROL - Rotate Left (Accumulator or Memory) */
static void op_rol(void)
{
    uint8_t operand = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    uint16_t result = ((uint16_t) operand << 1) | cpu->flag_c;
    calc_flags_czn(result);
    put_operand(opcode_table[cpu->current_opcode].addr_mode, (uint8_t) result);
}

/* Official: BMI - Branch on Minus */
static void op_bmi(void)
{
    branch(cpu->flag_n);
}

/* Official: SEC - Set Carry Flag */
static void op_sec(void)
{
    cpu->flag_c = true;
}

/* Official: RTI - Return from Interrupt */
static void op_rti(void)
{
    uint8_t p = pull8();
    cpu6502_set_status(p);
    cpu->pc = pull16();
}

/* Official: AND - Logical AND with Accumulator */
static void op_and(void)
{
    cpu->penalty_opcode = 1;
    cpu->a &= get_operand(opcode_table[cpu->current_opcode].addr_mode);
    calc_flags_zn(cpu->a);
}

/* Official: EOR - Exclusive OR with Accumulator */
static void op_eor(void)
{
    cpu->penalty_opcode = 1;
    cpu->a ^= get_operand(opcode_table[cpu->current_opcode].addr_mode);
    calc_flags_zn(cpu->a);
}

/* Official: ORA - Logical Inclusive OR with Accumulator */
static void op_ora(void)
{
    cpu->penalty_opcode = 1;
    cpu->a |= get_operand(opcode_table[cpu->current_opcode].addr_mode);
    calc_flags_zn(cpu->a);
}

/* Official: BCC - Branch on Carry Clear */
static void op_bcc(void)
{
    branch(!cpu->flag_c);
}

/* Official: BCS - Branch on Carry Set */
static void op_bcs(void)
{
    branch(cpu->flag_c);
}

/* Official: PHA - Push Accumulator */
static void op_pha(void)
{
    push8(cpu->a);
}

/* Official: LSR - Logical Shift Right */
static void op_lsr(void)
{
    uint8_t value  = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    uint8_t result = value >> 1;
    cpu->flag_c    = (value & 0x01) != 0;
    calc_flags_zn(result);
    put_operand(opcode_table[cpu->current_opcode].addr_mode, result);
}

/* Official: JMP - Jump */
static void op_jmp(void)
{
    cpu->pc = cpu->effective_addr;
}

/* Official: BVC - Branch on Overflow Clear */
static void op_bvc(void)
{
    branch(!cpu->flag_v);
}

/* Official: CLI - Clear Interrupt Disable */
static void op_cli(void)
{
    cpu->flag_i = false;
}

/* Official: RTS - Return from Subroutine */
static void op_rts(void)
{
    cpu->pc = pull16() + 1;
}

/* Official: ADC - Add with Carry */
static void op_adc(void)
{
    cpu->penalty_opcode = 1;
    uint8_t operand = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    uint16_t result = (uint16_t)(cpu->a + operand + cpu->flag_c);

    calc_flag_z((uint8_t) result);

    if (!cpu->flag_d)
    {
        calc_flag_c(result);
        calc_flag_v(result, cpu->a, operand);
        calc_flag_n((uint8_t) result);
    }
    else
    {
        /* Decimal mode (BCD) */
        result = (uint16_t)((cpu->a & 0x0F) + (operand & 0x0F) + cpu->flag_c);

        if (result >= 0x0A)
            result = ((result + 0x06) & 0x0F) + 0x10;

        result += (uint16_t)((cpu->a & 0xF0) + (operand & 0xF0));

        calc_flag_n((uint8_t) result);
        calc_flag_v(result, cpu->a, operand);

        cpu->cycles++; /* Extra cycle in BCD mode */

        if (result >= 0xA0)
            result += 0x60;

        calc_flag_c(result);
    }

    cpu->a = (uint8_t) result;
}

/* Official: PLA - Pull Accumulator */
static void op_pla(void)
{
    cpu->a = pull8();
    calc_flags_zn(cpu->a);
}

/* Official: ROR - Rotate Right */
static void op_ror(void)
{
    uint8_t value  = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    uint8_t result = (value >> 1) | (cpu->flag_c ? 0x80 : 0x00);
    cpu->flag_c    = (value & 0x01) != 0;
    calc_flags_zn(result);
    put_operand(opcode_table[cpu->current_opcode].addr_mode, result);
}

/* Official: BVS - Branch on Overflow Set */
static void op_bvs(void)
{
    branch(cpu->flag_v);
}

/* Official: SEI - Set Interrupt Disable */
static void op_sei(void)
{
    cpu->flag_i = true;
}

/* Official: STY - Store Y Register */
static void op_sty(void)
{
    put_operand(opcode_table[cpu->current_opcode].addr_mode, cpu->y);
}

/* Official: STX - Store X Register */
static void op_stx(void)
{
    put_operand(opcode_table[cpu->current_opcode].addr_mode, cpu->x);
}

/* Official: DEY - Decrement Y Register */
static void op_dey(void)
{
    cpu->y--;
    calc_flags_zn(cpu->y);
}

/* Official: TXA - Transfer X to Accumulator */
static void op_txa(void)
{
    cpu->a = cpu->x;
    calc_flags_zn(cpu->a);
}

/* Official: STA - Store Accumulator */
static void op_sta(void)
{
    put_operand(opcode_table[cpu->current_opcode].addr_mode, cpu->a);
}

/* Official: TYA - Transfer Y to Accumulator */
static void op_tya(void)
{
    cpu->a = cpu->y;
    calc_flags_zn(cpu->a);
}

/* Official: TXS - Transfer X to Stack Pointer */
static void op_txs(void)
{
    cpu->sp = cpu->x;
}

/* Official: LDY - Load Y Register */
static void op_ldy(void)
{
    cpu->penalty_opcode = 1;
    cpu->y = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    calc_flags_zn(cpu->y);
}

/* Official: LDA - Load Accumulator */
static void op_lda(void)
{
    cpu->penalty_opcode = 1;
    cpu->a = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    calc_flags_zn(cpu->a);
}

/* Official: LDX - Load X Register */
static void op_ldx(void)
{
    cpu->penalty_opcode = 1;
    cpu->x = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    calc_flags_zn(cpu->x);
}

/* Official: TAY - Transfer Accumulator to Y */
static void op_tay(void)
{
    cpu->y = cpu->a;
    calc_flags_zn(cpu->y);
}

/* Official: TSX - Transfer Stack Pointer to X */
static void op_tsx(void)
{
    cpu->x = cpu->sp;
    calc_flags_zn(cpu->x);
}

/* Official: TAX - Transfer Accumulator to X */
static void op_tax(void)
{
    cpu->x = cpu->a;
    calc_flags_zn(cpu->x);
}

/* Official: CLV - Clear Overflow Flag */
static void op_clv(void)
{
    cpu->flag_v = false;
}

/* Official: CPY - Compare Y Register */
static void op_cpy(void)
{
    compare(cpu->y, get_operand(opcode_table[cpu->current_opcode].addr_mode));
}

/* Official: CMP - Compare Accumulator */
static void op_cmp(void)
{
    compare(cpu->a, get_operand(opcode_table[cpu->current_opcode].addr_mode));
    cpu->penalty_opcode = 1;
}

/* Official: DEC - Decrement Memory */
static void op_dec(void)
{
    uint8_t v = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    v--;
    calc_flags_zn(v);
    put_operand(opcode_table[cpu->current_opcode].addr_mode, v);
}

/* Official: INY - Increment Y Register */
static void op_iny(void)
{
    cpu->y++;
    calc_flags_zn(cpu->y);
}

/* Official: DEX - Decrement X Register */
static void op_dex(void)
{
    cpu->x--;
    calc_flags_zn(cpu->x);
}

/* Official: BNE - Branch on Not Equal */
static void op_bne(void)
{
    branch(!cpu->flag_z);
}

/* Official: CLD - Clear Decimal Mode */
static void op_cld(void)
{
    cpu->flag_d = false;
}

/* Official: CPX - Compare X Register */
static void op_cpx(void)
{
    compare(cpu->x, get_operand(opcode_table[cpu->current_opcode].addr_mode));
}

/* Official: SBC - Subtract with Carry */
static void op_sbc(void)
{
    bool old_c          = cpu->flag_c;
    cpu->penalty_opcode = 1;
    uint8_t operand =
        get_operand(opcode_table[cpu->current_opcode].addr_mode) ^ 0xFF;
    uint16_t result = (uint16_t)(cpu->a + operand + cpu->flag_c);

    calc_flags_czn(result);
    calc_flag_v(result, cpu->a, operand);

    if (cpu->flag_d)
    {
        /* BCD subtraction */
        uint16_t al = (uint16_t)((cpu->a & 0x0F) - ((operand ^ 0xFF) & 0x0F) + old_c - 1);

        if ((int16_t) al < 0)
            al = ((al - 0x06) & 0x0F) - 0x10;

        result = (uint16_t)((cpu->a & 0xF0) - ((operand ^ 0xFF) & 0xF0) + al);

        if ((int16_t) result < 0)
            result -= 0x60;

        cpu->cycles++; /* Extra cycle in BCD mode */
    }

    cpu->a = (uint8_t) result;
}

/* Official: INC - Increment Memory */
static void op_inc(void)
{
    uint8_t result =
        get_operand(opcode_table[cpu->current_opcode].addr_mode) + 1;
    calc_flags_zn(result);
    put_operand(opcode_table[cpu->current_opcode].addr_mode, result);
}

/* Arithmetic Shift Left */
static void op_asl(void)
{
    uint8_t operand = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    uint16_t result = (uint16_t) operand << 1;
    calc_flags_czn(result);
    put_operand(opcode_table[cpu->current_opcode].addr_mode, (uint8_t) result);
}

/* Official: INX - Increment X Register */
static void op_inx(void)
{
    cpu->x++;
    calc_flags_zn(cpu->x);
}

/* Official: BEQ - Branch on Equal */
static void op_beq(void)
{
    branch(cpu->flag_z);
}

/* Official: SED - Set Decimal Mode */
static void op_sed(void)
{
    cpu->flag_d = true;
}

/* Official: NOP - No Operation (including illegal extended NOPs) */
static void op_nop(void)
{
    /* Some 'NOP' variants might add cycles if opcode is 0x1C, 0x3C, etc. */
    switch (cpu->current_opcode)
    {
        case 0x1C:
        case 0x3C:
        case 0x5C:
        case 0x7C:
        case 0xDC:
        case 0xFC:
            cpu->penalty_opcode = 1;
            break;
        default:
            break;
    }
}

/* Unofficial: ANC - AND with Carry */
static void op_anc(void)
{
    op_and(); /* a &= operand */
    cpu->flag_c = ((cpu->a & 0x80) != 0);
}

/* Unofficial: ALR - AND with Logical Shift Right */
static void op_alr(void)
{
    op_and();
    cpu->flag_c = (cpu->a & 0x01) != 0;
    cpu->a >>= 1;
    calc_flags_zn(cpu->a);
}

/* Unofficial: ARR (AND + ROR with special decimal fixups) */
static void op_arr(void)
{
    op_and();
    uint8_t old_a = cpu->a;
    cpu->a        = (cpu->a >> 1) | (cpu->flag_c ? 0x80 : 0);
    calc_flags_zn(cpu->a);

    if (!cpu->flag_d)
    {
        cpu->flag_c = ((cpu->a & 0x40) != 0);
        cpu->flag_v = cpu->flag_c ^ ((cpu->a >> 5) & 1);
    }
    else
    {
        cpu->flag_v = (((cpu->a ^ old_a) & 0x40) != 0);

        if (((old_a & 0x0F) + (old_a & 0x01)) > 0x05)
            cpu->a = (cpu->a & 0xF0) | ((cpu->a + 0x06) & 0x0F);

        if ((uint16_t) old_a + (old_a & 0x10) >= 0x60)
        {
            cpu->a += 0x60;
            cpu->flag_c = true;
        }
        else
        {
            cpu->flag_c = false;
        }
    }
}

/* Unofficial: ANE (sometimes called XAA or ANE) */
static void op_ane(void)
{
    /* (a | 0xEE or 0xEF) & x & operand -> a. Highly undefined on real chips. */
    cpu->a = (cpu->a | 0xEF) & cpu->x &
             get_operand(opcode_table[cpu->current_opcode].addr_mode);
    calc_flags_zn(cpu->a);
}

/* Unofficial: LXA (aka OAL) */
static void op_lxa(void)
{
    /* a = x = (a | 0xEE) & operand. Behavior depends on chip. */
    cpu->a = cpu->x =
        ((cpu->a | 0xEE) &
         get_operand(opcode_table[cpu->current_opcode].addr_mode));
    calc_flags_zn(cpu->a);
}

/* Unofficial: SBX (SAX, AXS) */
static void op_sbx(void)
{
    uint8_t operand = get_operand(opcode_table[cpu->current_opcode].addr_mode);
    cpu->x &= cpu->a;
    compare(cpu->x, operand);
    cpu->x -= operand;
}

/* KIL/JAM: freeze CPU */
static void op_jam(void)
{
    /* This illegal opcode halts the CPU */
    cpu->halted = true;
}

/* Unofficial: SLO = ASL + ORA */
static void op_slo(void)
{
    op_asl();
    op_ora();
}

/* Unofficial: RLA = ROL + AND */
static void op_rla(void)
{
    op_rol();
    op_and();
    cpu->penalty_opcode = 0;
}

/* Unofficial: SRE = LSR + EOR */
static void op_sre(void)
{
    op_lsr();
    op_eor();
    cpu->penalty_opcode = 0;
}

/* Unofficial: RRA = ROR + ADC */
static void op_rra(void)
{
    op_ror();
    op_adc();
    cpu->penalty_opcode = 0;

    if (cpu->flag_d)
    {
        cpu->cycles--;
    }
}

/* Unofficial: SAX = store (A & X) */
static void op_sax(void)
{
    put_operand(opcode_table[cpu->current_opcode].addr_mode, cpu->a & cpu->x);
}

/* Unofficial: LAX = load into A and X simultaneously */
static void op_lax(void)
{
    cpu->penalty_opcode = 1;
    op_lda();
    op_ldx();
}

/* Unofficial: DCP = DEC + CMP */
static void op_dcp(void)
{
    op_dec();
    op_cmp();
    cpu->penalty_opcode = 0;
}

/* Unofficial: ISC = INC + SBC */
static void op_isc(void)
{
    op_inc();
    op_sbc();
    cpu->penalty_opcode = 0;

    if (cpu->flag_d)
    {
        cpu->cycles--;
    }
}

/* Unofficial: SHA (stores A & X & (high-byte+1)) */
static void op_sha(void)
{
    put_operand(opcode_table[cpu->current_opcode].addr_mode,
                (uint8_t)(cpu->a & cpu->x & (((cpu->effective_addr >> 8) + 1) & 0xFF)));
}

/* Unofficial: SHX (store X & (high-byte+1)) with weird wrap */
static void op_shx(void)
{
    uint8_t value = (uint8_t)(cpu->x & ((((cpu->effective_addr - cpu->y)) >> 8) + 1));

    /* Emulate unstable behavior */
    if ((((cpu->effective_addr - cpu->y)) & 0xFF) + cpu->y > 0xFF)
        cpu->effective_addr = (cpu->effective_addr & 0xFF) | (value << 8);

    put_operand(opcode_table[cpu->current_opcode].addr_mode, value);
}

/* Unofficial: SHY (store Y & (high-byte+1)) */
static void op_shy(void)
{
    uint8_t value = (uint8_t)(cpu->y & ((((cpu->effective_addr - cpu->x)) >> 8) + 1));

    if ((((cpu->effective_addr - cpu->x)) & 0xFF) + cpu->x > 0xFF)
        cpu->effective_addr = (cpu->effective_addr & 0xFF) | (value << 8);

    put_operand(opcode_table[cpu->current_opcode].addr_mode, value);
}

/* Unofficial: TAS = set SP = A & X, then store SP & (high-byte+1) */
static void op_tas(void)
{
    cpu->sp = cpu->a & cpu->x;
    put_operand(opcode_table[cpu->current_opcode].addr_mode,
                (uint8_t)(cpu->sp & (((cpu->effective_addr >> 8) + 1) & 0xFF)));
}

/* Unofficial: LAS (aka LAE) = a, x, sp = mem & sp */
static void op_las(void)
{
    cpu->penalty_opcode = 1;
    cpu->sp = cpu->a = cpu->x =
        (get_operand(opcode_table[cpu->current_opcode].addr_mode) & cpu->sp);
    calc_flags_zn(cpu->a);
}

/* -----------------------------------------------------------------------------
 *  Initialize the opcode table with all 256 opcodes.
 * -----------------------------------------------------------------------------
 * This is essentially the big table mapping from official and illegal opcodes
 * to the correct addressing mode, function, and base cycle count.
 * @return 0 on success, non-zero on failure.
 */
int opcode_table_init(void)
{
    /* Allocate memory for 256 opcodes */
    opcode_table = malloc(sizeof(opcode_t) * 256);

    if (opcode_table == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for opcode table.\n");
        return 1;
    }

    /* Initialize all opcode entries to zero */
    memset(opcode_table, 0, sizeof(opcode_t) * 256);

    /*
     * Below is the complete mapping, matching the official and illegal opcodes
     * as widely documented.
     */

    /* 0x00 - 0x0F */
    opcode_table[0x00] = (opcode_t){ am_implied,             op_brk,    7 };
    opcode_table[0x01] = (opcode_t){ am_indexed_indirect,    op_ora,    6 };
    opcode_table[0x02] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0x03] = (opcode_t){ am_indexed_indirect,    op_slo,    8 };
    opcode_table[0x04] = (opcode_t){ am_zero_page,           op_nop,    3 };
    opcode_table[0x05] = (opcode_t){ am_zero_page,           op_ora,    3 };
    opcode_table[0x06] = (opcode_t){ am_zero_page,           op_asl,    5 };
    opcode_table[0x07] = (opcode_t){ am_zero_page,           op_slo,    5 };
    opcode_table[0x08] = (opcode_t){ am_implied,             op_php,    3 };
    opcode_table[0x09] = (opcode_t){ am_immediate,           op_ora,    2 };
    opcode_table[0x0A] = (opcode_t){ am_accumulator,         op_asl,    2 };
    opcode_table[0x0B] = (opcode_t){ am_immediate,           op_anc,    2 };
    opcode_table[0x0C] = (opcode_t){ am_absolute,            op_nop,    4 };
    opcode_table[0x0D] = (opcode_t){ am_absolute,            op_ora,    4 };
    opcode_table[0x0E] = (opcode_t){ am_absolute,            op_asl,    6 };
    opcode_table[0x0F] = (opcode_t){ am_absolute,            op_slo,    6 };

    /* 0x10 - 0x1F */
    opcode_table[0x10] = (opcode_t){ am_relative,            op_bpl,    2 };
    opcode_table[0x11] = (opcode_t){ am_indirect_indexed,    op_ora,    5 };
    opcode_table[0x12] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0x13] = (opcode_t){ am_indirect_indexed,    op_slo,    8 };
    opcode_table[0x14] = (opcode_t){ am_zero_page_x,         op_nop,    4 };
    opcode_table[0x15] = (opcode_t){ am_zero_page_x,         op_ora,    4 };
    opcode_table[0x16] = (opcode_t){ am_zero_page_x,         op_asl,    6 };
    opcode_table[0x17] = (opcode_t){ am_zero_page_x,         op_slo,    6 };
    opcode_table[0x18] = (opcode_t){ am_implied,             op_clc,    2 };
    opcode_table[0x19] = (opcode_t){ am_absolute_y,          op_ora,    4 };
    opcode_table[0x1A] = (opcode_t){ am_implied,             op_nop,    2 };
    opcode_table[0x1B] = (opcode_t){ am_absolute_y,          op_slo,    7 };
    opcode_table[0x1C] = (opcode_t){ am_absolute_x,          op_nop,    4 };
    opcode_table[0x1D] = (opcode_t){ am_absolute_x,          op_ora,    4 };
    opcode_table[0x1E] = (opcode_t){ am_absolute_x,          op_asl,    7 };
    opcode_table[0x1F] = (opcode_t){ am_absolute_x,          op_slo,    7 };

    /* 0x20 - 0x2F */
    opcode_table[0x20] = (opcode_t){ am_absolute,            op_jsr,    6 };
    opcode_table[0x21] = (opcode_t){ am_indexed_indirect,    op_and,    6 };
    opcode_table[0x22] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0x23] = (opcode_t){ am_indexed_indirect,    op_rla,    8 };
    opcode_table[0x24] = (opcode_t){ am_zero_page,           op_bit,    3 };
    opcode_table[0x25] = (opcode_t){ am_zero_page,           op_and,    3 };
    opcode_table[0x26] = (opcode_t){ am_zero_page,           op_rol,    5 };
    opcode_table[0x27] = (opcode_t){ am_zero_page,           op_rla,    5 };
    opcode_table[0x28] = (opcode_t){ am_implied,             op_plp,    4 };
    opcode_table[0x29] = (opcode_t){ am_immediate,           op_and,    2 };
    opcode_table[0x2A] = (opcode_t){ am_accumulator,         op_rol,    2 };
    opcode_table[0x2B] = (opcode_t){ am_immediate,           op_anc,    2 };
    opcode_table[0x2C] = (opcode_t){ am_absolute,            op_bit,    4 };
    opcode_table[0x2D] = (opcode_t){ am_absolute,            op_and,    4 };
    opcode_table[0x2E] = (opcode_t){ am_absolute,            op_rol,    6 };
    opcode_table[0x2F] = (opcode_t){ am_absolute,            op_rla,    6 };

    /* 0x30 - 0x3F */
    opcode_table[0x30] = (opcode_t){ am_relative,            op_bmi,    2 };
    opcode_table[0x31] = (opcode_t){ am_indirect_indexed,    op_and,    5 };
    opcode_table[0x32] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0x33] = (opcode_t){ am_indirect_indexed,    op_rla,    8 };
    opcode_table[0x34] = (opcode_t){ am_zero_page_x,         op_nop,    4 };
    opcode_table[0x35] = (opcode_t){ am_zero_page_x,         op_and,    4 };
    opcode_table[0x36] = (opcode_t){ am_zero_page_x,         op_rol,    6 };
    opcode_table[0x37] = (opcode_t){ am_zero_page_x,         op_rla,    6 };
    opcode_table[0x38] = (opcode_t){ am_implied,             op_sec,    2 };
    opcode_table[0x39] = (opcode_t){ am_absolute_y,          op_and,    4 };
    opcode_table[0x3A] = (opcode_t){ am_implied,             op_nop,    2 };
    opcode_table[0x3B] = (opcode_t){ am_absolute_y,          op_rla,    7 };
    opcode_table[0x3C] = (opcode_t){ am_absolute_x,          op_nop,    4 };
    opcode_table[0x3D] = (opcode_t){ am_absolute_x,          op_and,    4 };
    opcode_table[0x3E] = (opcode_t){ am_absolute_x,          op_rol,    7 };
    opcode_table[0x3F] = (opcode_t){ am_absolute_x,          op_rla,    7 };

    /* 0x40 - 0x4F */
    opcode_table[0x40] = (opcode_t){ am_implied,             op_rti,    6 };
    opcode_table[0x41] = (opcode_t){ am_indexed_indirect,    op_eor,    6 };
    opcode_table[0x42] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0x43] = (opcode_t){ am_indexed_indirect,    op_sre,    8 };
    opcode_table[0x44] = (opcode_t){ am_zero_page,           op_nop,    3 };
    opcode_table[0x45] = (opcode_t){ am_zero_page,           op_eor,    3 };
    opcode_table[0x46] = (opcode_t){ am_zero_page,           op_lsr,    5 };
    opcode_table[0x47] = (opcode_t){ am_zero_page,           op_sre,    5 };
    opcode_table[0x48] = (opcode_t){ am_implied,             op_pha,    3 };
    opcode_table[0x49] = (opcode_t){ am_immediate,           op_eor,    2 };
    opcode_table[0x4A] = (opcode_t){ am_accumulator,         op_lsr,    2 };
    opcode_table[0x4B] = (opcode_t){ am_immediate,           op_alr,    2 };
    opcode_table[0x4C] = (opcode_t){ am_absolute,            op_jmp,    3 };
    opcode_table[0x4D] = (opcode_t){ am_absolute,            op_eor,    4 };
    opcode_table[0x4E] = (opcode_t){ am_absolute,            op_lsr,    6 };
    opcode_table[0x4F] = (opcode_t){ am_absolute,            op_sre,    6 };

    /* 0x50 - 0x5F */
    opcode_table[0x50] = (opcode_t){ am_relative,            op_bvc,    2 };
    opcode_table[0x51] = (opcode_t){ am_indirect_indexed,    op_eor,    5 };
    opcode_table[0x52] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0x53] = (opcode_t){ am_indirect_indexed,    op_sre,    8 };
    opcode_table[0x54] = (opcode_t){ am_zero_page_x,         op_nop,    4 };
    opcode_table[0x55] = (opcode_t){ am_zero_page_x,         op_eor,    4 };
    opcode_table[0x56] = (opcode_t){ am_zero_page_x,         op_lsr,    6 };
    opcode_table[0x57] = (opcode_t){ am_zero_page_x,         op_sre,    6 };
    opcode_table[0x58] = (opcode_t){ am_implied,             op_cli,    2 };
    opcode_table[0x59] = (opcode_t){ am_absolute_y,          op_eor,    4 };
    opcode_table[0x5A] = (opcode_t){ am_implied,             op_nop,    2 };
    opcode_table[0x5B] = (opcode_t){ am_absolute_y,          op_sre,    7 };
    opcode_table[0x5C] = (opcode_t){ am_absolute_x,          op_nop,    4 };
    opcode_table[0x5D] = (opcode_t){ am_absolute_x,          op_eor,    4 };
    opcode_table[0x5E] = (opcode_t){ am_absolute_x,          op_lsr,    7 };
    opcode_table[0x5F] = (opcode_t){ am_absolute_x,          op_sre,    7 };

    /* 0x60 - 0x6F */
    opcode_table[0x60] = (opcode_t){ am_implied,             op_rts,    6 };
    opcode_table[0x61] = (opcode_t){ am_indexed_indirect,    op_adc,    6 };
    opcode_table[0x62] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0x63] = (opcode_t){ am_indexed_indirect,    op_rra,    8 };
    opcode_table[0x64] = (opcode_t){ am_zero_page,           op_nop,    3 };
    opcode_table[0x65] = (opcode_t){ am_zero_page,           op_adc,    3 };
    opcode_table[0x66] = (opcode_t){ am_zero_page,           op_ror,    5 };
    opcode_table[0x67] = (opcode_t){ am_zero_page,           op_rra,    5 };
    opcode_table[0x68] = (opcode_t){ am_implied,             op_pla,    4 };
    opcode_table[0x69] = (opcode_t){ am_immediate,           op_adc,    2 };
    opcode_table[0x6A] = (opcode_t){ am_accumulator,         op_ror,    2 };
    opcode_table[0x6B] = (opcode_t){ am_immediate,           op_arr,    2 };
    opcode_table[0x6C] = (opcode_t){ am_indirect,            op_jmp,    5 };
    opcode_table[0x6D] = (opcode_t){ am_absolute,            op_adc,    4 };
    opcode_table[0x6E] = (opcode_t){ am_absolute,            op_ror,    6 };
    opcode_table[0x6F] = (opcode_t){ am_absolute,            op_rra,    6 };

    /* 0x70 - 0x7F */
    opcode_table[0x70] = (opcode_t){ am_relative,            op_bvs,    2 };
    opcode_table[0x71] = (opcode_t){ am_indirect_indexed,    op_adc,    5 };
    opcode_table[0x72] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0x73] = (opcode_t){ am_indirect_indexed,    op_rra,    8 };
    opcode_table[0x74] = (opcode_t){ am_zero_page_x,         op_nop,    4 };
    opcode_table[0x75] = (opcode_t){ am_zero_page_x,         op_adc,    4 };
    opcode_table[0x76] = (opcode_t){ am_zero_page_x,         op_ror,    6 };
    opcode_table[0x77] = (opcode_t){ am_zero_page_x,         op_rra,    6 };
    opcode_table[0x78] = (opcode_t){ am_implied,             op_sei,    2 };
    opcode_table[0x79] = (opcode_t){ am_absolute_y,          op_adc,    4 };
    opcode_table[0x7A] = (opcode_t){ am_implied,             op_nop,    2 };
    opcode_table[0x7B] = (opcode_t){ am_absolute_y,          op_rra,    7 };
    opcode_table[0x7C] = (opcode_t){ am_absolute_x,          op_nop,    4 };
    opcode_table[0x7D] = (opcode_t){ am_absolute_x,          op_adc,    4 };
    opcode_table[0x7E] = (opcode_t){ am_absolute_x,          op_ror,    7 };
    opcode_table[0x7F] = (opcode_t){ am_absolute_x,          op_rra,    7 };

    /* 0x80 - 0x8F */
    opcode_table[0x80] = (opcode_t){ am_immediate,           op_nop,    2 };
    opcode_table[0x81] = (opcode_t){ am_indexed_indirect,    op_sta,    6 };
    opcode_table[0x82] = (opcode_t){ am_immediate,           op_nop,    2 };
    opcode_table[0x83] = (opcode_t){ am_indexed_indirect,    op_sax,    6 };
    opcode_table[0x84] = (opcode_t){ am_zero_page,           op_sty,    3 };
    opcode_table[0x85] = (opcode_t){ am_zero_page,           op_sta,    3 };
    opcode_table[0x86] = (opcode_t){ am_zero_page,           op_stx,    3 };
    opcode_table[0x87] = (opcode_t){ am_zero_page,           op_sax,    3 };
    opcode_table[0x88] = (opcode_t){ am_implied,             op_dey,    2 };
    opcode_table[0x89] = (opcode_t){ am_immediate,           op_nop,    2 };
    opcode_table[0x8A] = (opcode_t){ am_implied,             op_txa,    2 };
    opcode_table[0x8B] = (opcode_t){ am_immediate,           op_ane,    2 };
    opcode_table[0x8C] = (opcode_t){ am_absolute,            op_sty,    4 };
    opcode_table[0x8D] = (opcode_t){ am_absolute,            op_sta,    4 };
    opcode_table[0x8E] = (opcode_t){ am_absolute,            op_stx,    4 };
    opcode_table[0x8F] = (opcode_t){ am_absolute,            op_sax,    4 };

    /* 0x90 - 0x9F */
    opcode_table[0x90] = (opcode_t){ am_relative,            op_bcc,    2 };
    opcode_table[0x91] = (opcode_t){ am_indirect_indexed,    op_sta,    6 };
    opcode_table[0x92] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0x93] = (opcode_t){ am_indirect_indexed,    op_sha,    6 };
    opcode_table[0x94] = (opcode_t){ am_zero_page_x,         op_sty,    4 };
    opcode_table[0x95] = (opcode_t){ am_zero_page_x,         op_sta,    4 };
    opcode_table[0x96] = (opcode_t){ am_zero_page_y,         op_stx,    4 };
    opcode_table[0x97] = (opcode_t){ am_zero_page_y,         op_sax,    4 };
    opcode_table[0x98] = (opcode_t){ am_implied,             op_tya,    2 };
    opcode_table[0x99] = (opcode_t){ am_absolute_y,          op_sta,    5 };
    opcode_table[0x9A] = (opcode_t){ am_implied,             op_txs,    2 };
    opcode_table[0x9B] = (opcode_t){ am_absolute_y,          op_tas,    5 };
    opcode_table[0x9C] = (opcode_t){ am_absolute_x,          op_shy,    5 };
    opcode_table[0x9D] = (opcode_t){ am_absolute_x,          op_sta,    5 };
    opcode_table[0x9E] = (opcode_t){ am_absolute_y,          op_shx,    5 };
    opcode_table[0x9F] = (opcode_t){ am_absolute_y,          op_sha,    5 };

    /* 0xA0 - 0xAF */
    opcode_table[0xA0] = (opcode_t){ am_immediate,           op_ldy,    2 };
    opcode_table[0xA1] = (opcode_t){ am_indexed_indirect,    op_lda,    6 };
    opcode_table[0xA2] = (opcode_t){ am_immediate,           op_ldx,    2 };
    opcode_table[0xA3] = (opcode_t){ am_indexed_indirect,    op_lax,    6 };
    opcode_table[0xA4] = (opcode_t){ am_zero_page,           op_ldy,    3 };
    opcode_table[0xA5] = (opcode_t){ am_zero_page,           op_lda,    3 };
    opcode_table[0xA6] = (opcode_t){ am_zero_page,           op_ldx,    3 };
    opcode_table[0xA7] = (opcode_t){ am_zero_page,           op_lax,    3 };
    opcode_table[0xA8] = (opcode_t){ am_implied,             op_tay,    2 };
    opcode_table[0xA9] = (opcode_t){ am_immediate,           op_lda,    2 };
    opcode_table[0xAA] = (opcode_t){ am_implied,             op_tax,    2 };
    opcode_table[0xAB] = (opcode_t){ am_immediate,           op_lxa,    2 };
    opcode_table[0xAC] = (opcode_t){ am_absolute,            op_ldy,    4 };
    opcode_table[0xAD] = (opcode_t){ am_absolute,            op_lda,    4 };
    opcode_table[0xAE] = (opcode_t){ am_absolute,            op_ldx,    4 };
    opcode_table[0xAF] = (opcode_t){ am_absolute,            op_lax,    4 };

    /* 0xB0 - 0xBF */
    opcode_table[0xB0] = (opcode_t){ am_relative,            op_bcs,    2 };
    opcode_table[0xB1] = (opcode_t){ am_indirect_indexed,    op_lda,    5 };
    opcode_table[0xB2] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0xB3] = (opcode_t){ am_indirect_indexed,    op_lax,    5 };
    opcode_table[0xB4] = (opcode_t){ am_zero_page_x,         op_ldy,    4 };
    opcode_table[0xB5] = (opcode_t){ am_zero_page_x,         op_lda,    4 };
    opcode_table[0xB6] = (opcode_t){ am_zero_page_y,         op_ldx,    4 };
    opcode_table[0xB7] = (opcode_t){ am_zero_page_y,         op_lax,    4 };
    opcode_table[0xB8] = (opcode_t){ am_implied,             op_clv,    2 };
    opcode_table[0xB9] = (opcode_t){ am_absolute_y,          op_lda,    4 };
    opcode_table[0xBA] = (opcode_t){ am_implied,             op_tsx,    2 };
    opcode_table[0xBB] = (opcode_t){ am_absolute_y,          op_las,    4 };
    opcode_table[0xBC] = (opcode_t){ am_absolute_x,          op_ldy,    4 };
    opcode_table[0xBD] = (opcode_t){ am_absolute_x,          op_lda,    4 };
    opcode_table[0xBE] = (opcode_t){ am_absolute_y,          op_ldx,    4 };
    opcode_table[0xBF] = (opcode_t){ am_absolute_y,          op_lax,    4 };

    /* 0xC0 - 0xCF */
    opcode_table[0xC0] = (opcode_t){ am_immediate,           op_cpy,    2 };
    opcode_table[0xC1] = (opcode_t){ am_indexed_indirect,    op_cmp,    6 };
    opcode_table[0xC2] = (opcode_t){ am_immediate,           op_nop,    2 };
    opcode_table[0xC3] = (opcode_t){ am_indexed_indirect,    op_dcp,    8 };
    opcode_table[0xC4] = (opcode_t){ am_zero_page,           op_cpy,    3 };
    opcode_table[0xC5] = (opcode_t){ am_zero_page,           op_cmp,    3 };
    opcode_table[0xC6] = (opcode_t){ am_zero_page,           op_dec,    5 };
    opcode_table[0xC7] = (opcode_t){ am_zero_page,           op_dcp,    5 };
    opcode_table[0xC8] = (opcode_t){ am_implied,             op_iny,    2 };
    opcode_table[0xC9] = (opcode_t){ am_immediate,           op_cmp,    2 };
    opcode_table[0xCA] = (opcode_t){ am_implied,             op_dex,    2 };
    opcode_table[0xCB] = (opcode_t){ am_immediate,           op_sbx,    2 };
    opcode_table[0xCC] = (opcode_t){ am_absolute,            op_cpy,    4 };
    opcode_table[0xCD] = (opcode_t){ am_absolute,            op_cmp,    4 };
    opcode_table[0xCE] = (opcode_t){ am_absolute,            op_dec,    6 };
    opcode_table[0xCF] = (opcode_t){ am_absolute,            op_dcp,    6 };

    /* 0xD0 - 0xDF */
    opcode_table[0xD0] = (opcode_t){ am_relative,            op_bne,    2 };
    opcode_table[0xD1] = (opcode_t){ am_indirect_indexed,    op_cmp,    5 };
    opcode_table[0xD2] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0xD3] = (opcode_t){ am_indirect_indexed,    op_dcp,    8 };
    opcode_table[0xD4] = (opcode_t){ am_zero_page_x,         op_nop,    4 };
    opcode_table[0xD5] = (opcode_t){ am_zero_page_x,         op_cmp,    4 };
    opcode_table[0xD6] = (opcode_t){ am_zero_page_x,         op_dec,    6 };
    opcode_table[0xD7] = (opcode_t){ am_zero_page_x,         op_dcp,    6 };
    opcode_table[0xD8] = (opcode_t){ am_implied,             op_cld,    2 };
    opcode_table[0xD9] = (opcode_t){ am_absolute_y,          op_cmp,    4 };
    opcode_table[0xDA] = (opcode_t){ am_implied,             op_nop,    2 };
    opcode_table[0xDB] = (opcode_t){ am_absolute_y,          op_dcp,    7 };
    opcode_table[0xDC] = (opcode_t){ am_absolute_x,          op_nop,    4 };
    opcode_table[0xDD] = (opcode_t){ am_absolute_x,          op_cmp,    4 };
    opcode_table[0xDE] = (opcode_t){ am_absolute_x,          op_dec,    7 };
    opcode_table[0xDF] = (opcode_t){ am_absolute_x,          op_dcp,    7 };

    /* 0xE0 - 0xEF */
    opcode_table[0xE0] = (opcode_t){ am_immediate,           op_cpx,    2 };
    opcode_table[0xE1] = (opcode_t){ am_indexed_indirect,    op_sbc,    6 };
    opcode_table[0xE2] = (opcode_t){ am_immediate,           op_nop,    2 };
    opcode_table[0xE3] = (opcode_t){ am_indexed_indirect,    op_isc,    8 };
    opcode_table[0xE4] = (opcode_t){ am_zero_page,           op_cpx,    3 };
    opcode_table[0xE5] = (opcode_t){ am_zero_page,           op_sbc,    3 };
    opcode_table[0xE6] = (opcode_t){ am_zero_page,           op_inc,    5 };
    opcode_table[0xE7] = (opcode_t){ am_zero_page,           op_isc,    5 };
    opcode_table[0xE8] = (opcode_t){ am_implied,             op_inx,    2 };
    opcode_table[0xE9] = (opcode_t){ am_immediate,           op_sbc,    2 };
    opcode_table[0xEA] = (opcode_t){ am_implied,             op_nop,    2 };
    opcode_table[0xEB] = (opcode_t){ am_immediate,           op_sbc,    2 };
    opcode_table[0xEC] = (opcode_t){ am_absolute,            op_cpx,    4 };
    opcode_table[0xED] = (opcode_t){ am_absolute,            op_sbc,    4 };
    opcode_table[0xEE] = (opcode_t){ am_absolute,            op_inc,    6 };
    opcode_table[0xEF] = (opcode_t){ am_absolute,            op_isc,    6 };

    /* 0xF0 - 0xFF */
    opcode_table[0xF0] = (opcode_t){ am_relative,            op_beq,    2 };
    opcode_table[0xF1] = (opcode_t){ am_indirect_indexed,    op_sbc,    5 };
    opcode_table[0xF2] = (opcode_t){ am_implied,             op_jam,    2 };
    opcode_table[0xF3] = (opcode_t){ am_indirect_indexed,    op_isc,    8 };
    opcode_table[0xF4] = (opcode_t){ am_zero_page_x,         op_nop,    4 };
    opcode_table[0xF5] = (opcode_t){ am_zero_page_x,         op_sbc,    4 };
    opcode_table[0xF6] = (opcode_t){ am_zero_page_x,         op_inc,    6 };
    opcode_table[0xF7] = (opcode_t){ am_zero_page_x,         op_isc,    6 };
    opcode_table[0xF8] = (opcode_t){ am_implied,             op_sed,    2 };
    opcode_table[0xF9] = (opcode_t){ am_absolute_y,          op_sbc,    4 };
    opcode_table[0xFA] = (opcode_t){ am_implied,             op_nop,    2 };
    opcode_table[0xFB] = (opcode_t){ am_absolute_y,          op_isc,    7 };
    opcode_table[0xFC] = (opcode_t){ am_absolute_x,          op_nop,    4 };
    opcode_table[0xFD] = (opcode_t){ am_absolute_x,          op_sbc,    4 };
    opcode_table[0xFE] = (opcode_t){ am_absolute_x,          op_inc,    7 };
    opcode_table[0xFF] = (opcode_t){ am_absolute_x,          op_isc,    7 };

    return 0;
}

/**
 * @brief Destroy the opcode table if dynamically allocated.
 */
void opcode_table_destroy(void)
{
    if (opcode_table != NULL)
    {
        /* Free the opcode table memory */
        free(opcode_table);
        opcode_table = NULL;

/* Optional: Log the destruction for debugging purposes */
#ifdef DEBUG
        printf("Opcode table successfully destroyed.\n");
#endif
    }
    else
    {
/* Optional: Warn if destruction is attempted on an already destroyed table */
#ifdef DEBUG
        printf("Warning: Attempted to destroy an already NULL opcode table.\n");
#endif
    }
}

/* -----------------------------------------------------------------------------
 * Initialization
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Allocate and initialize the CPU instance, set PC from RESET_VECTOR,
 *        and build the opcode table.
 * @param bus Pointer to the system bus.
 * @return 0 on success, non-zero on failure.
 */
int cpu6502_init(bus_t *bus)
{
    if (cpu != NULL)
    {
        fprintf(stderr, "Warning: CPU is already initialized.\n");
        return 0;
    }

    cpu = calloc(1, sizeof(cpu6502_t));

    if (cpu == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for CPU.\n");
        return 1;
    }

    cpu->bus = bus; // Connect the CPU to the bus

    /* Initialize Registers */
    cpu->a      = 0;
    cpu->x      = 0;
    cpu->y      = 0;
    cpu->sp     = 0xFD;
    cpu->status = 0;

    /* Initialize the opcode table */
    if (opcode_table_init() != 0)
    {
        fprintf(stderr, "Error: Failed to initialize opcode table.\n");
        free(cpu);
        cpu = NULL;
        return 1;
    }

    /* Set PC to the reset vector (located at 0xFFFC/0xFFFD) */
    cpu->pc = read_word(RESET_VECTOR);

    if (cpu->pc == 0xFFFF) // Assuming 0xFFFF is an invalid address
    {
        fprintf(stderr, "Error: Failed to read RESET_VECTOR.\n");
        free(cpu);
        cpu = NULL;
        return 1;
    }

    /* Optional: Initialize additional CPU state if necessary */

    return 0;
}

/* -----------------------------------------------------------------------------
 * Reset & Interrupts
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Reset the CPU state (registers, flags, PC from RESET_VECTOR).
 * @return Number of clock cycles used for reset.
 */
int cpu6502_reset(void)
{
    if (cpu == NULL)
    {
        fprintf(stderr, "Error: CPU not initialized.\n");
        return -1;
    }

    /* Reset Registers */
    cpu->pc = read_word(RESET_VECTOR);
    cpu->sp = 0xFD;
    cpu->a  = 0;
    cpu->x  = 0;
    cpu->y  = 0;

    /* Reset Status Flags */
    cpu->flag_c = false;
    cpu->flag_z = false;
    cpu->flag_i = false;
    cpu->flag_d = false;
    cpu->flag_v = false;
    cpu->flag_n = false;
    cpu->halted = false;

    /* Optional: Reset additional CPU state if necessary */

    return 7; /* Typical reset cycle cost */
}

/**
 * @brief Handle Non-Maskable Interrupt (NMI).
 * @return Number of clock cycles used.
 */
int cpu6502_nmi(void)
{
    if (cpu == NULL)
    {
        fprintf(stderr, "Error: CPU not initialized.\n");
        return -1;
    }

    push16(cpu->pc);
    push8(cpu6502_get_status());
    cpu->flag_i = true;
    cpu->pc     = read_word(NMI_VECTOR);
    return 7;
}

/**
 * @brief Handle Interrupt Request (IRQ) if interrupts are not disabled.
 * @return Number of clock cycles used, or 0 if IRQ was ignored.
 */
int cpu6502_irq(void)
{
    if (cpu == NULL)
    {
        fprintf(stderr, "Error: CPU not initialized.\n");
        return -1;
    }

    if (!cpu->flag_i)
    {
        push16(cpu->pc);
        push8(cpu6502_get_status());
        cpu->flag_i = true;
        cpu->pc     = read_word(IRQ_VECTOR);
        return 7;
    }

    return 0;
}

/* -----------------------------------------------------------------------------
 * Execution & Destroy
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Execute a single instruction at the current PC.
 * @return Number of clock cycles used, or -1 on error.
 */
int cpu6502_step(void)
{
    if (cpu == NULL)
    {
        fprintf(stderr, "Error: CPU not initialized.\n");
        return -1;
    }

    if (cpu->halted)
    {
        /* CPU is halted by JAM (KIL). We return 0 cycles. */
        return 0;
    }

    /* Fetch the current opcode */
    uint8_t opcode_byte  = bus_read_memory(cpu->bus, cpu->pc++);
    cpu->current_opcode  = opcode_byte;
    cpu->penalty_opcode  = 0;
    cpu->penalty_address = 0;

    /* Retrieve the opcode entry */
    const opcode_t *opcode = &opcode_table[cpu->current_opcode];

    if (opcode->opcode_func == NULL || opcode->addr_mode == NULL)
    {
        fprintf(stderr, "Error: Undefined opcode 0x%02X at PC=0x%04X.\n",
                cpu->current_opcode, cpu->pc - 1);
        cpu->halted = true;
        return -1;
    }

    /* Initialize the base cycles from the opcode table */
    cpu->cycles = (double) opcode->cycles;

    /* Execute addressing mode routine */
    opcode->addr_mode();

    /* Execute the opcode function */
    opcode->opcode_func();

    /* Add one cycle if both penalty flags are set */
    if (cpu->penalty_opcode && cpu->penalty_address)
    {
        cpu->cycles += 1.0;
    }

    /* Handle clock cycles if clock is not disabled */
    if (!cpu->bus->clock_disabled)
    {
        for (int i = 0; i < (int) cpu->cycles; i++)
        {
            clock_wait_next_cycle(cpu->bus->clock);
        }
    }

    /* Optional: Logging for debugging */
#ifdef DEBUG
    printf("Executed opcode 0x%02X at PC=0x%04X. Cycles: %d\n", opcode_byte,
           cpu->pc - 1, (int) cpu->cycles);
#endif

    return (int) cpu->cycles;
}

/**
 * @brief Destroy the CPU instance (free memory).
 */
void cpu6502_destroy(void)
{
    opcode_table_destroy();
    free(cpu);
    cpu = NULL;
}
