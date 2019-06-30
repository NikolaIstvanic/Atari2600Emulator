/*
 *                                 $$ $$$$$ $$
 *                                 $$ $$$$$ $$
 *                                .$$ $$$$$ $$.
 *                                :$$ $$$$$ $$:
 *                                $$$ $$$$$ $$$
 *                                $$$ $$$$$ $$$
 *                               ,$$$ $$$$$ $$$.
 *                              ,$$$$ $$$$$ $$$$.
 *                             ,$$$$; $$$$$ :$$$$.
 *                            ,$$$$$  $$$$$  $$$$$.
 *                          ,$$$$$$'  $$$$$  `$$$$$$.
 *                        ,$$$$$$$'   $$$$$   `$$$$$$$.
 *                     ,s$$$$$$$'     $$$$$     `$$$$$$$s.
 *                   $$$$$$$$$'       $$$$$       `$$$$$$$$$
 *                   $$$$$Y'          $$$$$          `Y$$$$$
 *
 * CPU.c: C file which programmatically recreates the Atari 2600's MOS 6502 CPU.
 */
//#include <stdio.h>

#include "CPU.h"
#include "MMU.h"

/*
 * Given a CPU struct pointer, initialize all of its contents to correct values
 * which conform to the standards set by the Atari 2600's processor.
 *
 * This method will setup the CPU's RAM, interrupt vectors, interrupt handlers,
 * and register contents. All interrupt vectors are written to their respective
 * locations in CPU memory. At the memory location for the reset interrupt (the
 * reset interrupt service routine), the value of the location of the start of
 * code is stored. This ensures whenever a emulator is turned on, the PC will be
 * loaded with the value of the start of executable code immediately. For Non-
 * Maskable Interrupt and Interrupt Request, the service routines are a simple
 * RETI (return from interrupt) call.
 */
void cpu_init(CPU* cpu)
{
    /* Setup RAM, interrupt vectors, and interrupt service routines */
    mem_init(cpu);

    /* Initialize registers to appropriate contents */
    cpu->A = 0x0;
    cpu->X = 0x0;
    cpu->Y = 0x0;
    cpu->PC = read16(cpu, RESET_VECTOR);
    cpu->S = 0xFF;
    CLRSTATUS(cpu->P);
}

/*
 * Fetch the next instruction from the CPU's memory at the PC, decode it, and
 * execute it. Depending on the instruction, another call or two to fetch may be
 * required as the instruction's parameters.
 */
void cpu_step(CPU* cpu)
{
    // printf("PC = 0x%04X: ", cpu->PC);
    /* Fetch next instruction opcode */
    cpu->opcode = fetch(cpu);
    cpu->cycles = cycle_rom[cpu->opcode];
    /* Decode and execute */
    instruction_rom[cpu->opcode](cpu);
    // printf(" (0x%02X), S = 0x%02X, A = 0x%02X, X = 0x%02X, Y = 0x%02X, "
    //    "P = 0x%02X, Cycles = %d\n", cpu->opcode, cpu->S, cpu->A, cpu->X,
    //    cpu->Y, cpu->P, cpu->cycles);
}

/*
 * Fetch the next machine code byte from the CPU's memory at the index pointed
 * to by the CPU's PC register.
 */
inline uint8_t fetch(CPU* cpu)
{
    return read8(cpu, cpu->PC++);
}

/*******************************************************************************
 *                              HELPER METHODS                                 *
 ******************************************************************************/
/*
 * If negative, return offset and subtract 0x0100; otherwise just return
 * offset
 */
static inline uint16_t relative_offset(uint8_t offset)
{
    return (uint16_t) offset + (((offset & SIGN) >> 7) * -0x0100);
}

/*
 * Set the ZERO and NEGATIVE flags of the status register, P, for the
 * given parameter. Other flags will be set on a case-by-case basis.
 */
static void set_flags(CPU* cpu, uint8_t value)
{
    value ? CLRZERO(cpu->P) : SETZERO(cpu->P);
    value & SIGN ? SETSIGN(cpu->P) : CLRSIGN(cpu->P);
}

/*******************************************************************************
 *                     MEMORY ADDRESSING MODE METHODS                          *
 ******************************************************************************/
/*
 * Implied addressing mode. Here, the instruction is only one byte which
 * is the opcode, so it requires no addressing. That's why this method
 * only returns 0.
 */
static inline uint16_t IMP(CPU* cpu)
{
    return 0;
}

/*
 * Indirect, X addressing mode. The next byte offset by the value in the
 * X register is the address to either use or read from. Depending on
 * instruction, the address or value at that address is used; this is
 * why the address is returned.
 */
static inline uint16_t IDX(CPU* cpu)
{
    return read16(cpu, (fetch(cpu) + cpu->X) & 0xFF);
}

/*
 * Zero Page addressing mode. The byte after the opcode resides in low
 * memory or the Zero Page of memory. Because different instructions use
 * this address differently (like load/store vs OR), this method only
 * returns the address and not the value at that address.
 */
static inline uint16_t ZRP(CPU* cpu)
{
    return fetch(cpu);
}

/*
 * Immediate addressing mode. The next byte is the address to be read
 * from and used as the operand of the instruction. Value from memory
 * is returned because no load/store operation uses immediate
 * addressing.
 */
static inline uint16_t IMM(CPU* cpu)
{
    return fetch(cpu);
}

/*
 * Accumulator addressing mode. Instruction that use this addressing
 * mode are 1 byte, and the A register is implicitly used as an argument
 * without having to read in another byte, much like IMP. This is why
 * this method only returns 0.
 */
static inline uint16_t ACC(CPU* cpu)
{
    return 0;
}

/*
 * Absolute addressing mode. Here, the two bytes after the opcode are
 * used as an address to be used to load/store to, or the address of the
 * operand to be used in the instruction.
 */
static inline uint16_t ABS(CPU* cpu)
{
    uint16_t address = read16(cpu, cpu->PC);
    cpu->PC += 2;
    return address;
}

/*
 * Indirect addressing mode. Used only by the JMP instruction, this mode
 * requires that the next two bytes following the opcode be loaded from
 * memory and then stored into the PC register.
 */
static inline uint16_t IND(CPU* cpu)
{
    uint16_t address = read16(cpu, cpu->PC);
    cpu->PC += 2;
    return read16(cpu, address);
}

/*
 * Relative addressing mode. Used only by branching instructions, this
 * mode uses the byte after the opcode as a signed offset for the PC if
 * the branch is taken.
 */
static inline uint16_t REL(CPU* cpu)
{
    return fetch(cpu);
}

/*
 * Indirect Index addressing mode. In this mode the next byte is read
 * from memory, and the value in the Y register is added to the read.
 * This value is now the address to be used in load/store instructions,
 * or the value of the operand to use in other instructions.
 */
static inline uint16_t IDY(CPU* cpu)
{
    return read16(cpu, fetch(cpu)) + cpu->Y;
}

/*
 * Zero-Page Indexed addressing mode. Here the value of the next byte
 * is added to the X register. This address is returned and not read
 * from memory because instructions like STA required the address, not
 * the value at the address.
 */
static inline uint16_t ZPX(CPU* cpu)
{
    return (fetch(cpu) + cpu->X) & 0xFF;
}

/*
 * Zero-Page Index addressing mode. This mode is the same as the
 * previous, only the Y register is added to the next byte and ANDed
 * with 0xFF before being read.
 */
static inline uint16_t ZPY(CPU* cpu)
{
    return (fetch(cpu) + cpu->Y) & 0xFF;
}

/*
 * Absolute Indexed addressing mode. This mode uses the two bytes
 * following the opcode be used as a base address. This is then offset
 * by the Y register.
 */
static inline uint16_t ABY(CPU* cpu)
{
    uint16_t address = read16(cpu, cpu->PC);
    cpu->PC += 2;
    return address + cpu->Y;
}

/*
 * Absolute Indexed addressing mode. This mode is the same as ABY, but
 * instead of offsetting by the value in the Y register, it offsets by
 * the value in the X register.
 */
static inline uint16_t ABX(CPU* cpu)
{
    uint16_t address = read16(cpu, cpu->PC);
    cpu->PC += 2;
    return address + cpu->X;
}

/***********************************************************************
 *                       INSTRUCTION SET METHODS                       *
 **********************************************************************/
/*
 * Increment the Accumulator register by the CARRY bit and however the
 * next operand is stored in memory. The CARRY bit is set depending if
 * the operation was over 255.
 *
 * For example if CARRY was 1, the byte following the ADC opcode was
 * #8, and A = 35, the operation would be A = 35 + 1 + 8 and the CARRY
 * bit will be cleared.
 */
void ADC(CPU* cpu)
{
    // printf("ADC");
    uint8_t operand = cpu->opcode == 0x69 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    uint16_t sum = cpu->A + (cpu->P & CARRY) + operand;
    sum & CARRY ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
    set_flags(cpu, sum);
    !((cpu->A ^ operand) & SIGN) && ((cpu->A ^ sum) & SIGN)
        ? SETOVERFLOW(cpu->P) : CLROVERFLOW(cpu->P);
    cpu->A = sum;
}

void AND(CPU* cpu)
{
    // printf("AND");
    /* IMM addressing mode has next byte as immediate value, not address */
    cpu->A &= cpu->opcode == 0x39 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    set_flags(cpu, cpu->A);
}

void ASL(CPU* cpu)
{
    // printf("ASL");
    /* Different operand if accumulator addressing mode is used */
    if (cpu->opcode == 0x0A) {
        cpu->A & SIGN ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
        cpu->A <<= 1;
        set_flags(cpu, cpu->A);
    } else {
        uint16_t address = addressing_rom[cpu->opcode](cpu);
        uint8_t operand = read8(cpu, address);
        operand & SIGN ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
        operand <<= 1;
        set_flags(cpu, operand);
        write8(cpu, address, operand);
    }
}

/*
 * Branch on CARRY clear.
 */
void BCC(CPU* cpu)
{
    // printf("BCC");
    uint8_t offset = addressing_rom[cpu->opcode](cpu);
    if (!(cpu->P & CARRY)) {
        cpu->PC += relative_offset(offset);
    }
}

/*
 * Branch on CARRY set.
 */
void BCS(CPU* cpu)
{
    // printf("BCS");
    uint8_t offset = addressing_rom[cpu->opcode](cpu);
    if (cpu->P & CARRY) {
        cpu->PC += relative_offset(offset);
    }
}

void BEQ(CPU* cpu)
{
    // printf("BEQ");
    uint8_t offset = addressing_rom[cpu->opcode](cpu);
    if (cpu->P & ZERO) {
        cpu->PC += relative_offset(offset);
    }
}

void BIT(CPU* cpu)
{
    // printf("BIT");
    uint8_t operand = read8(cpu, addressing_rom[cpu->opcode](cpu));
    operand & SIGN ? SETSIGN(cpu->P) : CLRSIGN(cpu->P);
    operand & OVERFLOW ? SETOVERFLOW(cpu->P) : CLROVERFLOW(cpu->P);
    operand & cpu->A ? CLRZERO(cpu->P) : SETZERO(cpu->P);
}

/*
 * Branch on negative.
 */
void BMI(CPU* cpu)
{
    // printf("BMI");
    uint8_t offset = addressing_rom[cpu->opcode](cpu);
    if (cpu->P & SIGN) {
        cpu->PC += relative_offset(offset);
    }
}

/*
 * Branch on ZERO not set.
 */
void BNE(CPU* cpu)
{
    // printf("BNE");
    uint8_t offset = addressing_rom[cpu->opcode](cpu);
    if (!(cpu->P & ZERO)) {
        cpu->PC += relative_offset(offset);
    }
}

/*
 * Branch on not negative.
 */
void BPL(CPU* cpu)
{
    // printf("BPL");
    uint8_t offset = addressing_rom[cpu->opcode](cpu);
    if (!(cpu->P & SIGN)) {
        cpu->PC += relative_offset(offset);
    }
}

void BRK(CPU* cpu)
{
    // printf("BRK");
    SETBREAK(cpu->P);
    SETCONSTANT(cpu->P);
    push16(cpu, cpu->PC + 1);
    push8(cpu, cpu->P);
    SETINTERRUPT(cpu->P);
    cpu->PC = read16(cpu, IRQ_VECTOR);
}

/*
 * Branch on OVERFLOW clear.
 */
void BVC(CPU* cpu)
{
    // printf("BVC");
    uint8_t offset = addressing_rom[cpu->opcode](cpu);
    if (!(cpu->P & OVERFLOW)) {
        cpu->PC += relative_offset(offset);
    }
}

void BVS(CPU* cpu)
{
    // printf("BVS");
    uint8_t offset = addressing_rom[cpu->opcode](cpu);
    if (cpu->P & OVERFLOW) {
        cpu->PC += relative_offset(offset);
    }
}

void CLC(CPU* cpu)
{
    // printf("CLC");
    CLRCARRY(cpu->P);
}

/*
 * Clear DECIMAL flag.
 */
void CLD(CPU* cpu)
{
    // printf("CLD");
    CLRDECIMAL(cpu->P);
}

/*
 * Clear INTERRUPT bit of status register.
 */
void CLI(CPU* cpu)
{
    // printf("CLI");
    CLRINTERRUPT(cpu->P);
}

/*
 * Clear OVERFLOW flag.
 */
void CLV(CPU* cpu)
{
    // printf("CLV");
    CLROVERFLOW(cpu->P);
}

void CMP(CPU* cpu)
{
    // printf("CMP");
    /* IMM means use the next byte as an 8-bit value, not an address */
    uint8_t operand = cpu->opcode == 0xC9 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    cpu->A == operand ? SETZERO(cpu->P) : CLRZERO(cpu->P);
    cpu->A >= operand ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
}

void CPX(CPU* cpu)
{
    // printf("CPX");
    /* IMM means use the next byte as an 8-bit value, not an address */
    uint8_t operand = cpu->opcode == 0xE0 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    cpu->X == operand ? SETZERO(cpu->P) : CLRZERO(cpu->P);
    cpu->X >= operand ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
}

void CPY(CPU* cpu)
{
    // printf("CPY");
    /* IMM means use the next byte as an 8-bit value, not an address */
    uint8_t operand = cpu->opcode == 0xC0 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    cpu->Y == operand ? SETZERO(cpu->P) : CLRZERO(cpu->P);
    cpu->Y >= operand ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
}

void DEC(CPU* cpu)
{
    // printf("DEC");
    uint16_t address = addressing_rom[cpu->opcode](cpu);
    uint8_t value = read8(cpu, address) - 1;
    write8(cpu, address, value);
    set_flags(cpu, value);
}

/*
 * Decrement value in X register.
 */
void DEX(CPU* cpu)
{
    // printf("DEX");
    set_flags(cpu, --cpu->X);
}

/*
 * Decrement value stored in Y register.
 */
void DEY(CPU* cpu)
{
    // printf("DEY");
    set_flags(cpu, --cpu->Y);
}

void EOR(CPU* cpu)
{
    // printf("EOR");
    /* Immediate addressing mode returns a value, not an address */
    cpu->A ^= cpu->opcode == 0x49 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    set_flags(cpu, cpu->A);
}

void INC(CPU* cpu)
{
    // printf("INC");
    uint16_t address = addressing_rom[cpu->opcode](cpu);
    uint8_t value = read8(cpu, address) + 1;
    write8(cpu, address, value);
    set_flags(cpu, value);
}

/*
 * Increment the contents of the X register.
 */
void INX(CPU* cpu)
{
    // printf("INX");
    set_flags(cpu, ++cpu->X);
}

/*
 * Increment value in Y register.
 */
void INY(CPU* cpu)
{
    // printf("INY");
    set_flags(cpu, ++cpu->Y);
}

/*
 * Jump to the address specified by the next two bytes.
 */
void JMP(CPU* cpu)
{
    // printf("JMP");
    cpu->PC = addressing_rom[cpu->opcode](cpu);
}

void JSR(CPU* cpu)
{
    // printf("JSR");
    /* Push address of next instruction + 1 onto the stack in little endian */
    push16(cpu, cpu->PC + 1);
    cpu->PC = addressing_rom[cpu->opcode](cpu);
}

/*
 * Load value into A register.
 */
void LDA(CPU* cpu)
{
    // printf("LDA");
    /* IMM addressing means A is loaded with next byte, not MEM[next byte] */
    cpu->A = cpu->opcode == 0xA9 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    set_flags(cpu, cpu->A);
}

/*
 * Load value into X register.
 */
void LDX(CPU* cpu)
{
    // printf("LDX");
    /* IMM addressing means X is loaded with next byte, not MEM[next byte] */
    cpu->X = cpu->opcode == 0xA2 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    set_flags(cpu, cpu->X);
}

/*
 * Load value into Y register.
 */
void LDY(CPU* cpu)
{
    // printf("LDY");
    /* IMM addressing mode means next byte is the value for Y, not an address */
    cpu->Y = cpu->opcode == 0xA0 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    set_flags(cpu, cpu->Y);
}

void LSR(CPU* cpu)
{
    // printf("LSR");
    /* Different operand if accumulator addressing mode is used */
    if (cpu->opcode == 0x4A) {
        cpu->A & CARRY ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
        cpu->A >>= 1;
        set_flags(cpu, cpu->A);
    } else {
        uint16_t address = addressing_rom[cpu->opcode](cpu);
        uint8_t operand = read8(cpu, address);
        operand & CARRY ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
        operand >>= 1;
        set_flags(cpu, operand);
        write8(cpu, address, operand);
    }
}

void NOP(CPU* cpu)
{
    // printf("NOP");
}

void ORA(CPU* cpu)
{
    // printf("ORA");
    /* IMM addressing is immediate value, not an address */
    cpu->A |= cpu->opcode == 0x09 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    set_flags(cpu, cpu->A);
}

/*
 * Push A onto the stack.
 */
void PHA(CPU* cpu)
{
    // printf("PHA");
    push8(cpu, cpu->A);
}

/*
 * Push status register on the stack.
 */
void PHP(CPU* cpu)
{
    // printf("PHP");
    push8(cpu, cpu->P);
}

void PLA(CPU* cpu)
{
    // printf("PLA");
    cpu->A = pop8(cpu);
    set_flags(cpu, cpu->A);
}

/*
 * Pop from stack new status register value.
 */
void PLP(CPU* cpu)
{
    // printf("PLP");
    cpu->P = pop8(cpu);
}

/*
 * Perform one rotation.
 */
void ROL(CPU* cpu)
{
    // printf("ROL");
    /* Different operand if accumulator addressing mode is used */
    if (cpu->opcode == 0x2A) {
        uint8_t carry = (cpu->A & SIGN) >> 7;
        carry ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
        cpu->A <<= 1;
        cpu->A |= carry;
        set_flags(cpu, cpu->A);
    } else {
        uint16_t address = addressing_rom[cpu->opcode](cpu);
        uint8_t operand = read8(cpu, address);
        uint8_t carry = (operand & SIGN) >> 7;
        carry ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
        operand <<= 1;
        operand |= carry;
        set_flags(cpu, operand);
        write8(cpu, address, operand);
    }
}

void ROR(CPU* cpu)
{
    // printf("ROR");
    /* Accumulator addressing mode means A is the operand */
    if (cpu->opcode == 0x6A) {
        uint8_t carry = cpu->A & CARRY;
        cpu->A >>= 1;
        cpu->A |= carry << 7;
        cpu->A & CARRY ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
        set_flags(cpu, cpu->A);
    } else {
        uint16_t address = addressing_rom[cpu->opcode](cpu);
        uint8_t operand = read8(cpu, address);
        uint8_t carry = operand & CARRY;
        operand >>= 1;
        operand |= carry << 7;
        operand & CARRY ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
        set_flags(cpu, operand);
        write8(cpu, address, operand);
    }
}

/*
 * Return from interrupt.
 */
void RTI(CPU* cpu)
{
    // printf("RTI");
    cpu->P = pop8(cpu);
    cpu->PC = pop16(cpu);
}

/*
 * Return from subroutine.
 */
void RTS(CPU* cpu)
{
    // printf("RTS");
    cpu->PC = pop16(cpu) + 1;
}

void SBC(CPU* cpu)
{
    // printf("SBC");
    /* IMM addressing mode means next byte is value, not address */
    uint8_t operand = cpu->opcode == 0xE9 ? addressing_rom[cpu->opcode](cpu)
        : read8(cpu, addressing_rom[cpu->opcode](cpu));
    uint16_t difference = cpu->A - (1 - (cpu->P & CARRY)) - operand;
    set_flags(cpu, difference);
    ((cpu->A ^ difference) & SIGN) && ((cpu->A ^ operand) & SIGN)
        ? SETOVERFLOW(cpu->P) : CLROVERFLOW(cpu->P);
    (int16_t) cpu->A - (int16_t) operand - (1 - (cpu->P & CARRY)) < 0x0
        ? SETCARRY(cpu->P) : CLRCARRY(cpu->P);
    cpu->A = difference;
}

/*
 * Set the CARRY bit to 1.
 */
void SEC(CPU* cpu)
{
    // printf("SEC");
    SETCARRY(cpu->P);
}

void SED(CPU* cpu)
{
    // printf("SED");
    SETDECIMAL(cpu->P);
}

void SEI(CPU* cpu)
{
    // printf("SEI");
    SETINTERRUPT(cpu->P);
}

void STA(CPU* cpu)
{
    // printf("STA");
    write8(cpu, addressing_rom[cpu->opcode](cpu), cpu->A);
}

void STX(CPU* cpu)
{
    // printf("STX");
    write8(cpu, addressing_rom[cpu->opcode](cpu), cpu->X);
}

void STY(CPU* cpu)
{
    // printf("STY");
    write8(cpu, addressing_rom[cpu->opcode](cpu), cpu->Y);
}

/*
 * Copy value in register A into register X.
 */
void TAX(CPU* cpu)
{
    // printf("TAX");
    cpu->X = cpu->A;
    set_flags(cpu, cpu->X);
}

/*
 * Copy value in register A into register Y.
 */
void TAY(CPU* cpu)
{
    // printf("TAY");
    cpu->Y = cpu->A;
    set_flags(cpu, cpu->Y);
}

/*
 * Copy value of Stack Pointer register into X register.
 */
void TSX(CPU* cpu)
{
    // printf("TSX");
    cpu->X = cpu->S;
    set_flags(cpu, cpu->X);
}

/*
 * Copy value in X register to A register.
 */
void TXA(CPU* cpu)
{
    // printf("TXA");
    cpu->A = cpu->X;
    set_flags(cpu, cpu->A);
}

/*
 * Copy value in Y register to A register.
 */
void TYA(CPU* cpu)
{
    // printf("TYA");
    cpu->A = cpu->Y;
    set_flags(cpu,cpu->A);
}

/*
 * Stack Pointer now points to address which is stored in X.
 */
void TXS(CPU* cpu)
{
    // printf("TXS");
    cpu->S = cpu->X;
}

/*
 * ROM which is indexed into by the opcode of the current instruction in order
 * to get the number of machine cycles that instruction takes. This number will
 * then be used by the Picture Processing Unit in order to accurately display
 * pixels to the screen at the right time.
 */
uint8_t cycle_rom[0x100] = {
    /*0x  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
    /*0*/ 7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
    /*1*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*2*/ 6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
    /*3*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*4*/ 6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
    /*5*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*6*/ 6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
    /*7*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*8*/ 1, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    /*9*/ 2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
    /*A*/ 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    /*B*/ 2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
    /*C*/ 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    /*D*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    /*E*/ 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    /*F*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
};

/*
 * Function pointer array which will be indexed into in order to decode
 * valid 6502 instructions. Each instruction in the array is void because
 * instructions have variable length.
 */
void (*instruction_rom[0x100])(CPU* cpu) = {
    /*0x   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F*/
    /*0*/ BRK, ORA, NOP, NOP, NOP, ORA, ASL, NOP, PHP, ORA, ASL, NOP, NOP, ORA, ASL, NOP,
    /*1*/ BPL, ORA, NOP, NOP, NOP, ORA, ASL, NOP, CLC, ORA, NOP, NOP, NOP, ORA, ASL, NOP,
    /*2*/ JSR, AND, NOP, NOP, BIT, AND, ROL, NOP, PLP, AND, ROL, NOP, BIT, AND, ROL, NOP,
    /*3*/ BMI, AND, NOP, NOP, NOP, AND, ROL, NOP, SEC, AND, NOP, NOP, NOP, AND, ROL, NOP,
    /*4*/ RTI, EOR, NOP, NOP, NOP, EOR, LSR, NOP, PHA, EOR, LSR, NOP, JMP, EOR, LSR, NOP,
    /*5*/ BVC, EOR, NOP, NOP, NOP, EOR, LSR, NOP, CLI, EOR, NOP, NOP, NOP, EOR, LSR, NOP,
    /*6*/ RTS, ADC, NOP, NOP, NOP, ADC, ROR, NOP, PLA, ADC, ROR, NOP, JMP, ADC, ROR, NOP,
    /*7*/ BVS, ADC, NOP, NOP, NOP, ADC, ROR, NOP, SEI, ADC, NOP, NOP, NOP, ADC, ROR, NOP,
    /*8*/ NOP, STA, NOP, NOP, STY, STA, STX, NOP, DEY, NOP, TXA, NOP, STY, STA, STX, NOP,
    /*9*/ BCC, STA, NOP, NOP, STY, STA, STX, NOP, TYA, STA, TXS, NOP, NOP, STA, NOP, NOP,
    /*A*/ LDY, LDA, LDX, NOP, LDY, LDA, LDX, NOP, TAY, LDA, TAX, NOP, LDY, LDA, LDX, NOP,
    /*B*/ BCS, LDA, NOP, NOP, LDY, LDA, LDX, NOP, CLV, LDA, TSX, NOP, LDY, LDA, LDX, NOP,
    /*C*/ CPY, CMP, NOP, NOP, CPY, CMP, DEC, NOP, INY, CMP, DEX, NOP, CPY, CMP, DEC, NOP,
    /*D*/ BNE, CMP, NOP, NOP, NOP, CMP, DEC, NOP, CLD, CMP, NOP, NOP, NOP, CMP, DEC, NOP,
    /*E*/ CPX, SBC, NOP, NOP, CPX, SBC, INC, NOP, INX, SBC, NOP, NOP, CPX, SBC, INC, NOP,
    /*F*/ BEQ, SBC, NOP, NOP, NOP, SBC, INC, NOP, SED, SBC, NOP, NOP, NOP, SBC, INC, NOP
};

/*
 * Function pointer array which will be indexed into in order to determine which
 * memory addressing mode an instruction has based on its opcode.
 */
uint16_t (*addressing_rom[0x100])(CPU* cpu) = {
    /*0x   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F*/
    /*0*/ IMP, IDX, IMP, IMP, IMP, ZRP, ZRP, IMP, IMP, IMM, ACC, IMP, IMP, ABS, ABS, IMP,
    /*1*/ REL, IDY, IMP, IMP, IMP, ZPX, ZPX, IMP, IMP, ABY, IMP, IMP, IMP, ABX, ABX, IMP,
    /*2*/ ABS, IDX, IMP, IMP, ZRP, ZRP, ZRP, IMP, IMP, IMM, ACC, IMP, ABS, ABS, ABS, IMP,
    /*3*/ REL, IDY, IMP, IMP, IMP, ZPX, ZPX, IMP, IMP, ABY, IMP, IMP, IMP, ABX, ABX, IMP,
    /*4*/ IMP, IDX, IMP, IMP, IMP, ZRP, ZRP, IMP, IMP, IMM, ACC, IMP, ABS, ABS, ABS, IMP,
    /*5*/ REL, IDY, IMP, IMP, IMP, ZPX, ZPX, IMP, IMP, ABY, IMP, IMP, IMP, ABX, ABX, IMP,
    /*6*/ IMP, IDX, IMP, IMP, IMP, ZRP, ZRP, IMP, IMP, IMM, ACC, IMP, IND, ABS, ABS, IMP,
    /*7*/ REL, IDY, IMP, IMP, IMP, ZPX, ZPX, IMP, IMP, ABY, IMP, IMP, IMP, ABX, ABX, IMP,
    /*8*/ IMP, IDX, IMP, IMP, ZRP, ZRP, ZRP, IMP, IMP, IMP, IMP, IMP, ABS, ABS, ABS, IMP,
    /*9*/ REL, IDY, IMP, IMP, ZPX, ZPX, ZPY, IMP, IMP, ABY, IMP, IMP, IMP, ABX, IMP, IMP,
    /*A*/ IMM, IDX, IMM, IMP, ZRP, ZRP, ZRP, IMP, IMP, IMM, IMP, IMP, ABS, ABS, ABS, IMP,
    /*B*/ REL, IDY, IMP, IMP, ZPX, ZPX, ZPY, IMP, IMP, ABY, IMP, IMP, ABX, ABX, ABY, IMP,
    /*C*/ IMM, IDX, IMP, IMP, ZRP, ZRP, ZRP, IMP, IMP, IMM, IMP, IMP, ABS, ABS, ABS, IMP,
    /*D*/ REL, IDY, IMP, IMP, IMP, ZPX, ZPX, IMP, IMP, ABY, IMP, IMP, IMP, ABX, ABX, IMP,
    /*E*/ IMM, IDX, IMP, IMP, ZRP, ZRP, ZRP, IMP, IMP, IMM, IMP, IMP, ABS, ABS, ABS, IMP,
    /*F*/ REL, IDY, IMP, IMP, IMP, ZPX, ZPX, IMP, IMP, ABY, IMP, IMP, IMP, ABX, ABX, IMP
};

