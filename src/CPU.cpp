/**
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
 * CPU.cpp: class which programmatically recreates the Atari 2600's MOS 6507 CPU
 */
#include <iostream>

#include "Atari.hpp"
#include "CPU.hpp"

CPU::CPU() {
    m_instRom = {{
        /*                   0x0                               0x1                               0x2                               0x3                               0x4                               0x5                               0x6                               0x7                               0x8                               0x9                               0xA                               0xB                               0xC                               0xD                               0xE                               0xF              */
        /*0x0*/{"BRK", &CPU::BRK, &CPU::IMP, 7}, {"ORA", &CPU::ORA, &CPU::IDX, 6}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"SLO", &CPU::SLO, &CPU::IDX, 8}, {"DOP", &CPU::DOP, &CPU::ZRP, 3}, {"ORA", &CPU::ORA, &CPU::ZRP, 3}, {"ASL", &CPU::ASL, &CPU::ZRP, 5}, {"SLO", &CPU::SLO, &CPU::ZRP, 5}, {"PHP", &CPU::PHP, &CPU::IMP, 3}, {"ORA", &CPU::ORA, &CPU::IMM, 2}, {"ASL", &CPU::ASL, &CPU::ACC, 2}, {"ANC", &CPU::ANC, &CPU::IMM, 2}, {"TOP", &CPU::TOP, &CPU::ZRP, 4}, {"ORA", &CPU::ORA, &CPU::ABS, 4}, {"ASL", &CPU::ASL, &CPU::ABS, 6}, {"SLO", &CPU::SLO, &CPU::ABS, 6},
        /*0x1*/{"BPL", &CPU::BPL, &CPU::REL, 2}, {"ORA", &CPU::ORA, &CPU::IDY, 5}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"SLO", &CPU::SLO, &CPU::IDY, 8}, {"DOP", &CPU::DOP, &CPU::ZPX, 4}, {"ORA", &CPU::ORA, &CPU::ZPX, 4}, {"ASL", &CPU::ASL, &CPU::ZPX, 6}, {"SLO", &CPU::SLO, &CPU::ZPX, 6}, {"CLC", &CPU::CLC, &CPU::IMP, 2}, {"ORA", &CPU::ORA, &CPU::ABY, 4}, {"NOP", &CPU::NOP, &CPU::IMP, 2}, {"SLO", &CPU::SLO, &CPU::ABY, 7}, {"TOP", &CPU::TOP, &CPU::ZPX, 4}, {"ORA", &CPU::ORA, &CPU::ABX, 4}, {"ASL", &CPU::ASL, &CPU::ABX, 7}, {"SLO", &CPU::SLO, &CPU::ABX, 7},
        /*0x2*/{"JSR", &CPU::JSR, &CPU::ABS, 6}, {"AND", &CPU::AND, &CPU::IDX, 6}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"RLA", &CPU::RLA, &CPU::IDX, 8}, {"BIT", &CPU::BIT, &CPU::ZRP, 3}, {"AND", &CPU::AND, &CPU::ZRP, 3}, {"ROL", &CPU::ROL, &CPU::ZRP, 5}, {"RLA", &CPU::RLA, &CPU::ZRP, 5}, {"PLP", &CPU::PLP, &CPU::IMP, 4}, {"AND", &CPU::AND, &CPU::IMM, 2}, {"ROL", &CPU::ROL, &CPU::ACC, 2}, {"ANC", &CPU::ANC, &CPU::IMM, 2}, {"BIT", &CPU::BIT, &CPU::ABS, 4}, {"AND", &CPU::AND, &CPU::ABS, 4}, {"ROL", &CPU::ROL, &CPU::ABS, 6}, {"RLA", &CPU::RLA, &CPU::ABS, 6},
        /*0x3*/{"BMI", &CPU::BMI, &CPU::REL, 2}, {"AND", &CPU::AND, &CPU::IDY, 5}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"RLA", &CPU::RLA, &CPU::IDY, 8}, {"DOP", &CPU::DOP, &CPU::ZPX, 4}, {"AND", &CPU::AND, &CPU::ZPX, 4}, {"ROL", &CPU::ROL, &CPU::ZPX, 6}, {"RLA", &CPU::RLA, &CPU::ZPX, 6}, {"SEC", &CPU::SEC, &CPU::IMP, 2}, {"AND", &CPU::AND, &CPU::ABY, 4}, {"NOP", &CPU::NOP, &CPU::IMP, 2}, {"RLA", &CPU::RLA, &CPU::ABY, 7}, {"TOP", &CPU::TOP, &CPU::ABX, 4}, {"AND", &CPU::AND, &CPU::ABX, 4}, {"ROL", &CPU::ROL, &CPU::ABX, 7}, {"RLA", &CPU::RLA, &CPU::ABX, 7},
        /*0x4*/{"RTI", &CPU::RTI, &CPU::IMP, 6}, {"EOR", &CPU::EOR, &CPU::IDX, 6}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"SRE", &CPU::SRE, &CPU::IDX, 8}, {"DOP", &CPU::DOP, &CPU::ZRP, 3}, {"EOR", &CPU::EOR, &CPU::ZRP, 3}, {"LSR", &CPU::LSR, &CPU::ZRP, 5}, {"SRE", &CPU::SRE, &CPU::ZRP, 5}, {"PHA", &CPU::PHA, &CPU::IMP, 3}, {"EOR", &CPU::EOR, &CPU::IMM, 2}, {"LSR", &CPU::LSR, &CPU::ACC, 2}, {"ASR", &CPU::ASR, &CPU::IMM, 2}, {"JMP", &CPU::JMP, &CPU::ABS, 3}, {"EOR", &CPU::EOR, &CPU::ABS, 4}, {"LSR", &CPU::LSR, &CPU::ABS, 6}, {"SRE", &CPU::SRE, &CPU::ABS, 6},
        /*0x5*/{"BVC", &CPU::BVC, &CPU::REL, 2}, {"EOR", &CPU::EOR, &CPU::IDY, 5}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"SRE", &CPU::SRE, &CPU::IDY, 8}, {"DOP", &CPU::DOP, &CPU::ZPX, 4}, {"EOR", &CPU::EOR, &CPU::ZPX, 4}, {"LSR", &CPU::LSR, &CPU::ZPX, 6}, {"SRE", &CPU::SRE, &CPU::ZPX, 6}, {"CLI", &CPU::CLI, &CPU::IMP, 2}, {"EOR", &CPU::EOR, &CPU::ABY, 4}, {"NOP", &CPU::NOP, &CPU::IMP, 2}, {"SRE", &CPU::SRE, &CPU::ABY, 7}, {"TOP", &CPU::TOP, &CPU::ABX, 4}, {"EOR", &CPU::EOR, &CPU::ABX, 4}, {"LSR", &CPU::LSR, &CPU::ABX, 7}, {"SRE", &CPU::SRE, &CPU::ABX, 7},
        /*0x6*/{"RTS", &CPU::RTS, &CPU::IMP, 6}, {"ADC", &CPU::ADC, &CPU::IDX, 6}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"RRA", &CPU::RRA, &CPU::IDX, 8}, {"DOP", &CPU::DOP, &CPU::ZRP, 3}, {"ADC", &CPU::ADC, &CPU::ZRP, 3}, {"ROR", &CPU::ROR, &CPU::ZRP, 5}, {"RRA", &CPU::RRA, &CPU::ZRP, 5}, {"PLA", &CPU::PLA, &CPU::IMP, 4}, {"ADC", &CPU::ADC, &CPU::IMM, 2}, {"ROR", &CPU::ROR, &CPU::ACC, 2}, {"ARR", &CPU::ARR, &CPU::IMM, 2}, {"JMP", &CPU::JMP, &CPU::IND, 5}, {"ADC", &CPU::ADC, &CPU::ABS, 4}, {"ROR", &CPU::ROR, &CPU::ABS, 6}, {"RRA", &CPU::RRA, &CPU::ABS, 6},
        /*0x7*/{"BVS", &CPU::BVS, &CPU::REL, 2}, {"ADC", &CPU::ADC, &CPU::IDY, 5}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"RRA", &CPU::RRA, &CPU::IDY, 8}, {"DOP", &CPU::DOP, &CPU::ZPX, 4}, {"ADC", &CPU::ADC, &CPU::ZPX, 4}, {"ROR", &CPU::ROR, &CPU::ZPX, 6}, {"RRA", &CPU::RRA, &CPU::ZPX, 6}, {"SEI", &CPU::SEI, &CPU::IMP, 2}, {"ADC", &CPU::ADC, &CPU::ABY, 4}, {"NOP", &CPU::NOP, &CPU::IMP, 2}, {"RRA", &CPU::RRA, &CPU::ABY, 7}, {"TOP", &CPU::TOP, &CPU::ABX, 4}, {"ADC", &CPU::ADC, &CPU::ABX, 4}, {"ROR", &CPU::ROR, &CPU::ABX, 7}, {"RRA", &CPU::RRA, &CPU::ABX, 7},
        /*0x8*/{"DOP", &CPU::DOP, &CPU::IMM, 2}, {"STA", &CPU::STA, &CPU::IDX, 6}, {"DOP", &CPU::DOP, &CPU::IMM, 2}, {"SAX", &CPU::SAX, &CPU::IDX, 6}, {"STY", &CPU::STY, &CPU::ZRP, 3}, {"STA", &CPU::STA, &CPU::ZRP, 3}, {"STX", &CPU::STX, &CPU::ZRP, 3}, {"SAX", &CPU::SAX, &CPU::ZRP, 3}, {"DEY", &CPU::DEY, &CPU::IMP, 2}, {"DOP", &CPU::DOP, &CPU::IMM, 2}, {"TXA", &CPU::TXA, &CPU::IMP, 2}, {"ANE", &CPU::ANE, &CPU::IMM, 2}, {"STY", &CPU::STY, &CPU::ABS, 4}, {"STA", &CPU::STA, &CPU::ABS, 4}, {"STX", &CPU::STX, &CPU::ABS, 4}, {"SAX", &CPU::SAX, &CPU::ABS, 4},
        /*0x9*/{"BCC", &CPU::BCC, &CPU::REL, 2}, {"STA", &CPU::STA, &CPU::IDY, 6}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"SHA", &CPU::SHA, &CPU::IDY, 6}, {"STY", &CPU::STY, &CPU::ZPX, 4}, {"STA", &CPU::STA, &CPU::ZPX, 4}, {"STX", &CPU::STX, &CPU::ZPY, 4}, {"SAX", &CPU::SAX, &CPU::ZPY, 4}, {"TYA", &CPU::TYA, &CPU::IMP, 2}, {"STA", &CPU::STA, &CPU::ABY, 5}, {"TXS", &CPU::TXS, &CPU::IMP, 2}, {"SHS", &CPU::SHS, &CPU::ABY, 5}, {"SHY", &CPU::SHY, &CPU::ABX, 5}, {"STA", &CPU::STA, &CPU::ABX, 5}, {"SHX", &CPU::SHX, &CPU::ABY, 5}, {"SHA", &CPU::SHA, &CPU::ABY, 5},
        /*0xA*/{"LDY", &CPU::LDY, &CPU::IMM, 2}, {"LDA", &CPU::LDA, &CPU::IDX, 6}, {"LDX", &CPU::LDX, &CPU::IMM, 2}, {"LAX", &CPU::LAX, &CPU::IDX, 6}, {"LDY", &CPU::LDY, &CPU::ZRP, 3}, {"LDA", &CPU::LDA, &CPU::ZRP, 3}, {"LDX", &CPU::LDX, &CPU::ZRP, 3}, {"LAX", &CPU::LAX, &CPU::ZRP, 3}, {"TAY", &CPU::TAY, &CPU::IMP, 2}, {"LDA", &CPU::LDA, &CPU::IMM, 2}, {"TAX", &CPU::TAX, &CPU::IMP, 2}, {"LXA", &CPU::LXA, &CPU::IMM, 2}, {"LDY", &CPU::LDY, &CPU::ABS, 4}, {"LDA", &CPU::LDA, &CPU::ABS, 4}, {"LDX", &CPU::LDX, &CPU::ABS, 4}, {"LAX", &CPU::LAX, &CPU::ABS, 4},
        /*0xB*/{"BCS", &CPU::BCS, &CPU::REL, 2}, {"LDA", &CPU::LDA, &CPU::IDY, 5}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"LAX", &CPU::LAX, &CPU::IDY, 5}, {"LDY", &CPU::LDY, &CPU::ZPX, 4}, {"LDA", &CPU::LDA, &CPU::ZPX, 4}, {"LDX", &CPU::LDX, &CPU::ZPY, 4}, {"LAX", &CPU::LAX, &CPU::ZPY, 4}, {"CLV", &CPU::CLV, &CPU::IMP, 2}, {"LDA", &CPU::LDA, &CPU::ABY, 4}, {"TSX", &CPU::TSX, &CPU::IMP, 2}, {"LAS", &CPU::LAS, &CPU::ABY, 4}, {"LDY", &CPU::LDY, &CPU::ABX, 4}, {"LDA", &CPU::LDA, &CPU::ABX, 4}, {"LDX", &CPU::LDX, &CPU::ABY, 4}, {"LAX", &CPU::LAX, &CPU::ABY, 4},
        /*0xC*/{"CPY", &CPU::CPY, &CPU::IMM, 2}, {"CMP", &CPU::CMP, &CPU::IDX, 6}, {"DOP", &CPU::DOP, &CPU::IMM, 2}, {"DCP", &CPU::DCP, &CPU::IDX, 8}, {"CPY", &CPU::CPY, &CPU::ZRP, 3}, {"CMP", &CPU::CMP, &CPU::ZRP, 3}, {"DEC", &CPU::DEC, &CPU::ZRP, 5}, {"DCP", &CPU::DCP, &CPU::ZRP, 5}, {"INY", &CPU::INY, &CPU::IMP, 2}, {"CMP", &CPU::CMP, &CPU::IMM, 2}, {"DEX", &CPU::DEX, &CPU::IMP, 2}, {"SBX", &CPU::SBX, &CPU::IMP, 2}, {"CPY", &CPU::CPY, &CPU::ABS, 4}, {"CMP", &CPU::CMP, &CPU::ABS, 4}, {"DEC", &CPU::DEC, &CPU::ABS, 6}, {"DCP", &CPU::DCP, &CPU::ABS, 6},
        /*0xE*/{"BNE", &CPU::BNE, &CPU::REL, 2}, {"CMP", &CPU::CMP, &CPU::IDY, 5}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"DCP", &CPU::DCP, &CPU::IDY, 8}, {"DOP", &CPU::DOP, &CPU::ZPX, 4}, {"CMP", &CPU::CMP, &CPU::ZPX, 4}, {"DEC", &CPU::DEC, &CPU::ZPX, 6}, {"DCP", &CPU::DCP, &CPU::ZPX, 6}, {"CLD", &CPU::CLD, &CPU::IMP, 2}, {"CMP", &CPU::CMP, &CPU::ABY, 4}, {"NOP", &CPU::NOP, &CPU::IMP, 2}, {"DCP", &CPU::DCP, &CPU::ABY, 7}, {"TOP", &CPU::TOP, &CPU::ABX, 4}, {"CMP", &CPU::CMP, &CPU::ABX, 4}, {"DEC", &CPU::DEC, &CPU::ABX, 7}, {"DCP", &CPU::DCP, &CPU::ABX, 7},
        /*0xD*/{"CPX", &CPU::CPX, &CPU::IMM, 2}, {"SBC", &CPU::SBC, &CPU::IDX, 6}, {"DOP", &CPU::DOP, &CPU::IMM, 2}, {"ISB", &CPU::ISB, &CPU::IDX, 8}, {"CPX", &CPU::CPX, &CPU::ZRP, 3}, {"SBC", &CPU::SBC, &CPU::ZRP, 3}, {"INC", &CPU::INC, &CPU::ZRP, 5}, {"ISB", &CPU::ISB, &CPU::ZRP, 5}, {"INX", &CPU::INX, &CPU::IMP, 2}, {"SBC", &CPU::SBC, &CPU::IMM, 2}, {"NOP", &CPU::NOP, &CPU::IMM, 2}, {"SBC", &CPU::SBC, &CPU::IMM, 2}, {"CPX", &CPU::CPX, &CPU::ABS, 4}, {"SBC", &CPU::SBC, &CPU::ABS, 4}, {"INC", &CPU::INC, &CPU::ABS, 6}, {"ISB", &CPU::ISB, &CPU::ABS, 6},
        /*0xF*/{"BEQ", &CPU::BEQ, &CPU::REL, 2}, {"SBC", &CPU::SBC, &CPU::IDY, 5}, {"KIL", &CPU::KIL, &CPU::IMP, 2}, {"ISB", &CPU::ISB, &CPU::IDY, 8}, {"DOP", &CPU::DOP, &CPU::ZPX, 4}, {"SBC", &CPU::SBC, &CPU::ZPX, 4}, {"INC", &CPU::INC, &CPU::ZPX, 6}, {"ISB", &CPU::ISB, &CPU::ZPX, 6}, {"SED", &CPU::SED, &CPU::IMP, 2}, {"SBC", &CPU::SBC, &CPU::ABY, 4}, {"NOP", &CPU::NOP, &CPU::IMP, 2}, {"ISB", &CPU::ISB, &CPU::ABY, 7}, {"TOP", &CPU::TOP, &CPU::ABX, 4}, {"SBC", &CPU::SBC, &CPU::ABX, 4}, {"INC", &CPU::INC, &CPU::ABX, 7}, {"ISB", &CPU::ISB, &CPU::ABX, 7}
    }};
}

CPU::~CPU() = default;

void CPU::connectAtari(Atari* atari) {
    m_atari = atari;
    m_timer.connectAtari(atari);
}

/**
 * @brief Operations to perform upon receiving a reset signal
 */
void CPU::reset() {
    m_pc = read16(RESET_VECTOR);
    m_a = 0x00;
    m_x = 0x00;
    m_y = 0x00;
    m_s = 0xFD;
    m_p = CONSTANT;
    m_cycles = 8; 
}

/**
 * Handle the next instruction.
 *
 * First the next opcode is fetched from memory, then this opcode is decoded and executed using a
 * jump table of function pointers which is indexed by the opcode. After this, the number of cycles
 * that instruction takes on the actual MOS 6507 microprocessor is saved; this value is used for
 * timing purposes.
 */
void CPU::step() {
    if (m_cycles == 0) {
        m_opcode = fetch();
        m_additionalCycle = 0;
        m_cycles = m_instRom[m_opcode].cycles;
        m_cycles += (this->*m_instRom[m_opcode].op)() & m_additionalCycle;
        logInfo();
    }

    m_timer.step();
    m_cycles--;
}

/**
 * @brief Interrupt service routine for a non-maskable interrupt.
 */
void CPU::nmi() {
    push16(m_pc);
    clrBit(BREAK);
    setBit(CONSTANT);
    setBit(INTERRUPT);
    push8(m_p);
    m_pc = read16(NMI_VECTOR);
    m_cycles = 8;
}

/**
 * @brief Interrupt service routine for a maskable interrupt.
 */
void CPU::irq() {
    if (!(m_p & INTERRUPT)) {
        push16(m_pc);
        clrBit(BREAK);
        setBit(CONSTANT);
        setBit(INTERRUPT);
        push8(m_p);
        m_pc = read16(IRQ_VECTOR);
        m_cycles = 7;
    }
}

/**
 * @brief Print diagnostic information for debugging CPU execution.
 */
inline void CPU::logInfo() {
#ifdef DEBUG
    std::cout << "PC = 0x" << std::hex << PC << ": "
        << m_instRom[m_opcode].name << " (0x" << std::hex << static_cast<int>(m_opcode)
        << ") A: 0x" << std::hex << static_cast<int>(m_a) << " X: 0x" << std::hex
        << static_cast<int>(m_x) << " Y: 0x" << std::hex << static_cast<int>(m_y) << std::endl;
#endif
}

/**
 * @brief Fetch the next byte from RAM pointed to by the CPU's PC register.
 */
inline uint8_t CPU::fetch() { return read8(m_pc++); }

/**
 * @brief If negative, return offset and subtract 0x0100; otherwise just return offset.
 */
inline uint16_t CPU::relativeOffset(uint8_t offset) const {
    return static_cast<uint16_t>(offset) + (((offset & SIGN) >> 7) * -0x0100);
}

inline void CPU::setBit(CPUFLAG f) { m_p |= f; }
inline void CPU::clrBit(CPUFLAG f) { m_p &= ~f; }

/**
 * @brief Set the ZERO and SIGN flags of the status register for the given parameter.
 */
inline void CPU::setZEROSIGN(uint8_t value) {
    value ? clrBit(ZERO) : setBit(ZERO);
    value & SIGN ? setBit(SIGN) : clrBit(SIGN);
}

uint8_t CPU::read8(uint16_t addr) { return m_atari->read8(addr); }
uint16_t CPU::read16(uint16_t addr) { return m_atari->read16(addr); }
void CPU::write8(uint16_t addr, uint8_t data) { m_atari->write8(addr, data); }
void CPU::write16(uint16_t addr, uint16_t data) { m_atari->write16(addr, data); }
uint8_t CPU::pop8() { return read8(++m_s | 0x100); }

uint16_t CPU::pop16() {
    uint16_t data = read16(++m_s | 0x100);
    m_s++;
    return data;
}

void CPU::push8(uint8_t data) { write8(m_s-- | 0x100, data); }

void CPU::push16(uint16_t data) {
    write16(--m_s | 0x100, data);
    m_s--;
}

/*******************************************************************************
 *                         Memory addressing modes                             *
 ******************************************************************************/
/**
 * Absolute addressing mode. Here, the two bytes after the opcode are used as an address to be used
 * to load/store to, or the address of the operand to be used in the instruction.
 */
inline uint16_t CPU::ABS() {
    uint16_t address = read16(m_pc);

    m_pc += 2;
    return address;
}

/**
 * Absolute Indexed addressing mode. This mode is the same as ABY, but instead of offsetting by
 * the value in the Y register, it offsets by the value in the X register.
 */
inline uint16_t CPU::ABX() {
    uint16_t address = read16(m_pc);

    m_pc += 2;
    if (address < 0x100 && address + m_y >= 0x100) {
        // Page boundary crossed
        m_additionalCycle = 1;
    }

    return address + m_x;
}

/**
 * Absolute Indexed addressing mode. This mode uses the two bytes following the opcode be used as
 * a base address. This is then offset by the Y register.
 */
inline uint16_t CPU::ABY() {
    uint16_t address = read16(m_pc);

    m_pc += 2;
    if (address < 0x100 && address + m_y >= 0x100) {
        // Page boundary crossed
        m_additionalCycle = 1;
    }

    return address + m_y;
}

/**
 * Accumulator addressing mode. Instruction that use this addressing mode are 1 byte, and the A
 * register is implicitly used as an argument without having to read in another byte, much like
 * IMP. This is why this method only returns 0.
 */
inline uint16_t CPU::ACC() { return 0; }

/**
 * Indirect, X addressing mode. The next byte offset by the value in the X register is the address
 * to either use or read from. Depending on instruction, the address or value at that address is
 * used; this is why the address is returned.
 */
inline uint16_t CPU::IDX() { return read16((fetch() + m_x) & 0xFF); }

/**
 * Indirect Index addressing mode. In this mode the next byte is read from memory, and the value
 * in the Y register is added to the read. This value is now the address to be used in load/store
 * instructions, or the value of the operand to use in other instructions.
 */
inline uint16_t CPU::IDY() {
    uint16_t address = read16(fetch());

    if (address < 0x100 && address + m_y >= 0x100) {
        // Page boundary crossed
        m_additionalCycle = 1;
    }

    return address + m_y;
}

/**
 * Immediate addressing mode. The next byte is the address to be read from and used as the operand
 * of the instruction. Value from memory is returned because no load/store operation uses immediate
 * addressing.
 */
inline uint16_t CPU::IMM() { return fetch(); }

/**
 * Implied addressing mode. Here, the instruction is only one byte which is the opcode, so it
 * requires no addressing. That's why this method only returns 0.
 */
inline uint16_t CPU::IMP() { return 0; }

/**
 * Indirect addressing mode. Used only by the JMP instruction, this mode requires that the next two
 * bytes following the opcode be loaded from memory and then stored into the PC register.
 */
inline uint16_t CPU::IND() {
    uint16_t address = read16(m_pc);

    m_pc += 2;
    if ((address & 0x00FF) == 0x00FF) {
        return (read8(address & 0xFF00) << 8) | read8(address);
    }

    return read16(address);
}

/**
 * Relative addressing mode. Used only by branching instructions, this mode uses the byte after
 * the opcode as a signed offset for the PC if the branch is taken.
 */
inline uint16_t CPU::REL() { return fetch(); }

/**
 * Zero-Page Indexed addressing mode. Here the value of the next byte is added to the X register.
 * This address is returned and not read from memory because instructions like STA required the
 * address, not the value at the address.
 */
inline uint16_t CPU::ZPX() { return (uint16_t) ((uint8_t) fetch() + m_x); }

/**
 * Zero-Page Index addressing mode. This mode is the same as the previous, only the Y register
 * is added to the next byte and ANDed with 0xFF before being read.
 */
inline uint16_t CPU::ZPY() { return (fetch() + m_y) & 0xFF; }

/**
 * Zero Page addressing mode. The byte after the opcode resides in low memory or the Zero Page
 * of memory. Because different instructions use this address differently (like load/store
 * vs OR), this method only returns the address and not the value at that address.
 */
inline uint16_t CPU::ZRP() { return fetch(); }

/*******************************************************************************
 *                              Instruction Set                                *
 ******************************************************************************/
uint8_t CPU::ADC() {
    uint8_t operand = m_opcode == 0x69 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    uint16_t sum = m_a + (m_p & CARRY) + operand;

    sum > 0xFF ? setBit(CARRY) : clrBit(CARRY);
    (~((uint16_t) m_a ^ operand) & ((uint16_t) m_a ^ sum)) & 0x0080
        ? setBit(OVERFLOW) : clrBit(OVERFLOW);
    m_a = sum;
    setZEROSIGN(m_a);
    return 1;
}

uint8_t CPU::ANC() {
    uint8_t operand = (this->*m_instRom[m_opcode].addr)();

    m_a &= operand;
    setZEROSIGN(m_a);
    m_a & SIGN ? setBit(CARRY) : clrBit(CARRY);
    return 0;
}

uint8_t CPU::AND() {
    /* IMM addressing mode has next byte as immediate value, not address */
    m_a &= m_opcode == 0x29 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    setZEROSIGN(m_a);
    return 1;
}

uint8_t CPU::ANE() { return 0; }

uint8_t CPU::ARR() {
    uint8_t flags;
    uint8_t operand = (this->*m_instRom[m_opcode].addr)() & m_a;

    m_a = (operand >> 1) | ((m_p & CARRY) ? 0x80 : 0x00);
    setZEROSIGN(m_a);
    flags = m_a & 0x60;

    switch (flags) {
        case 0x00:
            clrBit(CARRY);
            clrBit(OVERFLOW);
            break;

        case 0x20:
            clrBit(CARRY);
            setBit(OVERFLOW);
            break;

        case 0x40:
            setBit(CARRY);
            setBit(OVERFLOW);
            break;

        case 0x60:
            setBit(CARRY);
            clrBit(OVERFLOW);
            break;
    }

    return 0;
}

uint8_t CPU::ASL() {
    if (m_opcode == 0x0A) {
        m_a & SIGN ? setBit(CARRY) : clrBit(CARRY);
        m_a <<= 1;
        setZEROSIGN(m_a);
    } else {
        uint16_t address = (this->*m_instRom[m_opcode].addr)();
        uint8_t operand = read8(address);

        operand & SIGN ? setBit(CARRY) : clrBit(CARRY);
        operand <<= 1;
        setZEROSIGN(operand);
        write8(address, operand);
    }

    return 0;
}

uint8_t CPU::ASR() {
    uint8_t operand = (this->*m_instRom[m_opcode].addr)() & m_a;

    operand & 0x01 ? setBit(CARRY) : clrBit(CARRY);
    m_a = operand >> 1;
    m_a ? clrBit(ZERO) : setBit(ZERO);
    clrBit(SIGN);
    return 0;
}

uint8_t CPU::BCC() {
    uint8_t offset = (this->*m_instRom[m_opcode].addr)();

    if (!(m_p & CARRY)) {
        if (m_pc < 0x100 && m_pc + relativeOffset(offset) >= 0x100) {
            m_cycles++;
        }

        m_cycles++;
        m_pc += relativeOffset(offset);
    }
    return 0;
}

uint8_t CPU::BCS() {
    uint8_t offset = (this->*m_instRom[m_opcode].addr)();

    if (m_p & CARRY) {
        if (((m_pc + relativeOffset(offset)) & 0xFF00) != (m_pc & 0xFF00)) {
            m_cycles++;
        }

        m_cycles++;
        m_pc += relativeOffset(offset);
    }

    return 0;
}

uint8_t CPU::BEQ() {
    uint8_t offset = (this->*m_instRom[m_opcode].addr)();

    if (m_p & ZERO) {
        if (((m_pc + relativeOffset(offset)) & 0xFF00) != (m_pc & 0xFF00)) {
            m_cycles++;
        }

        m_cycles++;
        m_pc += relativeOffset(offset);
    }

    return 0;
}

uint8_t CPU::BIT() {
    uint8_t operand = read8((this->*m_instRom[m_opcode].addr)());

    operand & SIGN ? setBit(SIGN) : clrBit(SIGN);
    operand & OVERFLOW ? setBit(OVERFLOW) : clrBit(OVERFLOW);
    operand & m_a ? clrBit(ZERO) : setBit(ZERO);
    return 0;
}

uint8_t CPU::BMI() {
    uint8_t offset = (this->*m_instRom[m_opcode].addr)();

    if (m_p & SIGN) {
        if (((m_pc + relativeOffset(offset)) & 0xFF00) != (m_pc & 0xFF00)) {
            m_cycles++;
        }

        m_cycles++;
        m_pc += relativeOffset(offset);
    }

    return 0;
}

uint8_t CPU::BNE() {
    uint8_t offset = (this->*m_instRom[m_opcode].addr)();

    if (!(m_p & ZERO)) {
        if (((m_pc + relativeOffset(offset)) & 0xFF00) != (m_pc & 0xFF00)) {
            m_cycles++;
        }

        m_cycles++;
        m_pc += relativeOffset(offset);
    }

    return 0;
}

uint8_t CPU::BPL() {
    uint8_t offset = (this->*m_instRom[m_opcode].addr)();

    if (!(m_p & SIGN)) {
        if (((m_pc + relativeOffset(offset)) & 0xFF00) != (m_pc & 0xFF00)) {
            m_cycles++;
        }

        m_cycles++;
        m_pc += relativeOffset(offset);
    }

    return 0;
}

uint8_t CPU::BRK() {
    setBit(INTERRUPT);
    push16(m_pc + 1);
    setBit(BREAK);
    setBit(CONSTANT);
    push8(m_p);
    clrBit(BREAK);
    m_pc = read16(IRQ_VECTOR);
    return 0;
}

uint8_t CPU::BVC() {
    uint8_t offset = (this->*m_instRom[m_opcode].addr)();

    if (!(m_p & OVERFLOW)) {
        if (((m_pc + relativeOffset(offset)) & 0xFF00) != (m_pc & 0xFF00)) {
            m_cycles++;
        }

        m_cycles++;
        m_pc += relativeOffset(offset);
    }

    return 0;
}

uint8_t CPU::BVS() {
    uint8_t offset = (this->*m_instRom[m_opcode].addr)();

    if (m_p & OVERFLOW) {
        if (((m_pc + relativeOffset(offset)) & 0xFF00) != (m_pc & 0xFF00)) {
            m_cycles++;
        }

        m_cycles++;
        m_pc += relativeOffset(offset);
    }

    return 0;
}

uint8_t CPU::CLC() { clrBit(CARRY); return 0; }
uint8_t CPU::CLD() { clrBit(DECIMAL); return 0; }
uint8_t CPU::CLI() { clrBit(INTERRUPT); return 0; }
uint8_t CPU::CLV() { clrBit(OVERFLOW); return 0; }

uint8_t CPU::CMP() {
    /* IMM means use the next byte as an 8-bit value, not an address */
    uint8_t operand = m_opcode == 0xC9 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    uint16_t diff = static_cast<uint16_t>(m_a) - operand;

    m_a >= operand ? setBit(CARRY) : clrBit(CARRY);
    setZEROSIGN(diff);
    return 1;
}

uint8_t CPU::CPX() {
    /* IMM means use the next byte as an 8-bit value, not an address */
    uint8_t operand = m_opcode == 0xE0 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    uint16_t diff = static_cast<uint16_t>(m_x) - operand;

    m_x >= operand ? setBit(CARRY) : clrBit(CARRY);
    setZEROSIGN(diff);
    return 0;
}

uint8_t CPU::CPY() {
    /* IMM means use the next byte as an 8-bit value, not an address */
    uint8_t operand = m_opcode == 0xC0 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    uint16_t diff = static_cast<uint16_t>(m_y) - operand;

    m_y >= operand ? setBit(CARRY) : clrBit(CARRY);
    setZEROSIGN(diff);
    return 0;
}

uint8_t CPU::DCP() {
    uint16_t address = (this->*m_instRom[m_opcode].addr)();
    uint8_t operand = read8(address) - 1;

    write8(operand, address);
    m_a - operand >= 0 ? setBit(CARRY) : clrBit(CARRY);
    setZEROSIGN(m_a - operand);
    return 0;
}

uint8_t CPU::DEC() {
    uint16_t address = (this->*m_instRom[m_opcode].addr)();
    uint8_t value = read8(address) - 1;

    write8(address, value);
    setZEROSIGN(value);
    return 0;
}

uint8_t CPU::DEX() { setZEROSIGN(--m_x); return 0; }
uint8_t CPU::DEY() { setZEROSIGN(--m_y); return 0; }
uint8_t CPU::DOP() { m_pc++; return 0; }

uint8_t CPU::EOR() {
    /* Immediate addressing mode returns a value, not an address */
    m_a ^= m_opcode == 0x49 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    setZEROSIGN(m_a);
    return 1;
}

uint8_t CPU::INC() {
    uint16_t address = (this->*m_instRom[m_opcode].addr)();
    uint8_t value = read8(address) + 1;

    write8(address, value);
    setZEROSIGN(value);
    return 0;
}

uint8_t CPU::INX() { setZEROSIGN(++m_x); return 0; }
uint8_t CPU::INY() { setZEROSIGN(++m_y); return 0; }

uint8_t CPU::ISB() {
    uint8_t address = (this->*m_instRom[m_opcode].addr)();
    uint8_t operand = read8(address) + 1;

    write8(address, operand);
    if (m_p & DECIMAL) {
        uint16_t difference;
        uint8_t a = (m_a & 0x0F) - (1 - ((m_p & CARRY) ? 1 : 0));
        uint8_t h = (m_a >> 4) - (operand >> 4) - (a < 0);

        if (a < 0) {
            a -= 6;
        }
        if (h < 0) {
            h -= 6;
        }

        difference = m_a - operand - (1 - ((m_p & CARRY) ? 1 : 0));
        -difference & 0x0100 ? setBit(CARRY) : clrBit(CARRY);
        (((m_a ^ operand) & (m_a ^ difference)) & 0x80) ? setBit(OVERFLOW) : clrBit(OVERFLOW);
        setZEROSIGN(difference);
        m_a = ((h << 4) | (a & 0x0F)) & 0xFF;
    } else {
        operand = ~operand;
        uint16_t difference = m_a + operand + ((m_p & CARRY) ? 1 : 0);
        difference > 0xFF ? setBit(CARRY) : clrBit(CARRY);
        ((m_a ^ difference) & (operand ^ difference) & 0x80) ? setBit(OVERFLOW) : clrBit(OVERFLOW);
        m_a = difference;
        setZEROSIGN(m_a);
    }
    return 0;
}

uint8_t CPU::JMP() { m_pc = (this->*m_instRom[m_opcode].addr)(); return 0; }

uint8_t CPU::JSR() {
    /* Push address of next instruction + 1 onto the stack in little endian */
    push16(m_pc + 1);
    m_pc = (this->*m_instRom[m_opcode].addr)();
    return 0;
}

/**
 * @brief Halt all execution of instructions.
 */
uint8_t CPU::KIL() { m_pc--; return 0; }

uint8_t CPU::LAS() {
    uint8_t operand = read8((this->*m_instRom[m_opcode].addr)()) & m_s;

    m_pc--;
    m_a = operand;
    m_x = operand;
    m_s = operand;
    setZEROSIGN(operand);
    return 1;
}

uint8_t CPU::LAX() {
    uint8_t operand = read8((this->*m_instRom[m_opcode].addr)());

    m_a = operand;
    m_x = operand;
    setZEROSIGN(operand);
    return 1;
}

uint8_t CPU::LDA() {
    /* IMM addressing means A is loaded with next byte, not MEM[next byte] */
    m_a = m_opcode == 0xA9 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    setZEROSIGN(m_a);
    return 1;
}

uint8_t CPU::LDX() {
    /* IMM addressing means X is loaded with next byte, not MEM[next byte] */
    m_x = m_opcode == 0xA2 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    setZEROSIGN(m_x);
    return 1;
}

uint8_t CPU::LDY() {
    /* IMM addressing mode means next byte is the value for Y, not an address */
    m_y = m_opcode == 0xA0 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    setZEROSIGN(m_y);
    return 1;
}

uint8_t CPU::LSR() {
    /* Different operand if accumulator addressing mode is used */
    if (m_opcode == 0x4A) {
        m_a & CARRY ? setBit(CARRY) : clrBit(CARRY);
        m_a >>= 1;
        setZEROSIGN(m_a);
    } else {
        uint16_t address = (this->*m_instRom[m_opcode].addr)();
        uint8_t operand = read8(address);

        operand & CARRY ? setBit(CARRY) : clrBit(CARRY);
        operand >>= 1;
        setZEROSIGN(operand);
        write8(address, operand);
    }

    return 0;
}

uint8_t CPU::LXA() {
    m_a &= (this->*m_instRom[m_opcode].addr)();
    m_x = m_a;
    setZEROSIGN(m_a);
    return 1;
}

uint8_t CPU::NOP() { return 0; }

uint8_t CPU::ORA() {
    /* IMM addressing is immediate value, not an address */
    m_a |= m_opcode == 0x09 ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());
    setZEROSIGN(m_a);
    return 1;
}

uint8_t CPU::PHA() { push8(m_a); return 0; }

uint8_t CPU::PHP() {
    push8(m_p | BREAK | CONSTANT);
    clrBit(BREAK);
    return 0;
}

uint8_t CPU::PLA() {
    m_a = pop8();
    setZEROSIGN(m_a);
    return 0;
}

uint8_t CPU::PLP() { m_p = pop8(); return 0; }

uint8_t CPU::RLA() {
    uint8_t carry;
    uint16_t address = (this->*m_instRom[m_opcode].addr)();
    uint8_t operand = read8(address);

    carry = (m_p & CARRY) >> 7;
    operand & carry ? setBit(CARRY) : clrBit(CARRY);
    operand = (operand << 1) | carry;
    write8(address, operand);
    m_a &= operand;
    setZEROSIGN(operand);
    return 0;
}

uint8_t CPU::ROL() {
    /* Different operand if accumulator addressing mode is used */
    if (m_opcode == 0x2A) {
        uint8_t carry = m_a & SIGN;

        m_a <<= 1;
        m_a |= (m_p & CARRY) ? 1 : 0;
        carry ? setBit(CARRY) : clrBit(CARRY);
        setZEROSIGN(m_a);
    } else {
        uint16_t address = (this->*m_instRom[m_opcode].addr)();
        uint8_t operand = read8(address);
        uint8_t carry = operand & SIGN;

        operand <<= 1;
        operand |= m_p & CARRY;
        carry ? setBit(CARRY) : clrBit(CARRY);
        setZEROSIGN(operand);
        write8(address, operand);
    }

    return 0;
}

uint8_t CPU::ROR() {
    /* Accumulator addressing mode means A is the operand */
    if (m_opcode == 0x6A) {
        uint8_t carry = m_a & CARRY;

        m_a >>= 1;
        m_a |= (m_p & CARRY) ? 1 << 7 : 0;
        carry ? setBit(CARRY) : clrBit(CARRY);
        setZEROSIGN(m_a);
    } else {
        uint16_t address = (this->*m_instRom[m_opcode].addr)();
        uint8_t operand = read8(address);
        uint8_t carry = operand & CARRY;

        operand >>= 1;
        operand |= (m_p & CARRY) ? 1 << 7 : 0;
        carry ? setBit(CARRY) : clrBit(CARRY);
        setZEROSIGN(operand);
        write8(address, operand);
    }

    return 0;
}

uint8_t CPU::RRA() {
    uint8_t carry;
    uint16_t address = (this->*m_instRom[m_opcode].addr)();
    uint8_t operand = read8(address);

    carry = (m_p & CARRY);
    operand & CARRY ? setBit(CARRY) : clrBit(CARRY);
    operand = (operand >> 1) | carry;
    write8(address, operand);

    if (m_p & DECIMAL) {
        uint8_t al = (m_a & 0x0F) + (operand & 0x0F) + ((m_p & CARRY) ? 1 : 0);
        if (al > 9) {
            al += 6;
        }

        uint8_t ah = ((m_a >> 4) + (operand >> 4) + (al > 15 ? 1 : 0)) << 4;

        (m_a + operand + ((m_p & CARRY) ? 1 : 0)) ? clrBit(ZERO) : setBit(ZERO);
        ah ? setBit(SIGN) : clrBit(SIGN);
        (((m_a ^ ah) & ~(m_a ^ operand)) & 0x80) ? setBit(OVERFLOW) : clrBit(OVERFLOW);
        if (ah > 0x9F) {
            ah += 0x60;
        }

        m_a = ah | (al & 0x0F);
    } else {
        uint16_t sum = m_a + operand + ((m_p & CARRY) ? 1 : 0);

        sum > 0xFF ? setBit(CARRY) : clrBit(CARRY);
        ((m_a ^ sum) & (operand ^ sum)) & 0x80 ? setBit(OVERFLOW) : clrBit(OVERFLOW);
        m_a = sum;
        setZEROSIGN(m_a);
    }

    return 0;
}

uint8_t CPU::RTI() {
    m_p = pop8();
    clrBit(BREAK);
    m_pc = pop16();
    return 0;
}

uint8_t CPU::RTS() { m_pc = pop16() + 1; return 0; }

uint8_t CPU::SAX() {
    uint8_t operand = read8((this->*m_instRom[m_opcode].addr)());

    write8(operand, m_x & m_a);
    return 0;
}

uint8_t CPU::SBC() {
    /* IMM addressing mode means next byte is value, not address */
    uint8_t operand = m_opcode == 0xE8 || m_opcode == 0xE9
        ? (this->*m_instRom[m_opcode].addr)()
        : read8((this->*m_instRom[m_opcode].addr)());

    if (m_p & DECIMAL) {
        uint8_t al = (m_a & 0x0F) - (operand & 0x0F) - (1 - ((m_p & CARRY) ? 1 : 0));
        uint8_t ah = (m_a >> 4) - (operand >> 4) - (al < 0 ? 1 : 0);

        if (al < 0) {
            al -= 6;
        }
        if (ah < 0) {
            ah -= 6;
        }

        uint16_t sub = m_a - operand - (1 - (m_p & CARRY));
        sub > 0xFF ? setBit(CARRY) : clrBit(CARRY);
        (((m_a ^ operand) & (m_a ^ sub)) & 0x80) ? setBit(OVERFLOW) : clrBit(OVERFLOW);
        setZEROSIGN(sub);
        m_a = (ah << 4) | al;
    } else {
        operand = ~operand;
        uint16_t sum = m_a + (m_p & CARRY) + operand;
        sum & 0xFF00 ? setBit(CARRY) : clrBit(CARRY);
        ((sum ^ m_a) & (sum ^ operand) & 0x0080) ? setBit(OVERFLOW) : clrBit(OVERFLOW);
        m_a = sum;
        setZEROSIGN(m_a);
    }

    return 1;
}

uint8_t CPU::SBX() {
    uint8_t operand = (this->*m_instRom[m_opcode].addr)();

    (m_a & m_x) >= operand ? setBit(CARRY) : clrBit(CARRY);
    m_x = ((m_a & m_x) - operand) & 0xFF;
    setZEROSIGN(m_x);
    return 0;
}

uint8_t CPU::SEC() { setBit(CARRY); return 0; }
uint8_t CPU::SED() { setBit(DECIMAL); return 0; }
uint8_t CPU::SEI() { setBit(INTERRUPT); return 0; }

uint8_t CPU::SHA() {
    uint16_t operand = (this->*m_instRom[m_opcode].addr)();

    write8(m_a & m_x & ((operand >> 8) + 1), operand);
    return 0;
}

uint8_t CPU::SHS() {
    uint16_t address = (this->*m_instRom[m_opcode].addr)();

    m_s = m_a & m_x;
    write8(address, m_a & m_x & ((address >> 8) + 1));
    return 0;
}

uint8_t CPU::SHX() {
    uint16_t address = (this->*m_instRom[m_opcode].addr)();

    write8(address, m_x & ((address >> 8) + 1));
    return 0;
}

uint8_t CPU::SHY() {
    uint16_t address = (this->*m_instRom[m_opcode].addr)();

    write8(address, m_y & ((address >> 8) + 1));
    return 0;
}

uint8_t CPU::SLO() {
    uint16_t address = (this->*m_instRom[m_opcode].addr)();
    uint8_t operand = read8(address);

    operand & CARRY ? setBit(CARRY) : clrBit(CARRY);
    operand <<= 1;
    write8(address, operand);
    m_a |= operand;
    setZEROSIGN(m_a);
    return 0;
}

uint8_t CPU::SRE() {
    uint16_t address = (this->*m_instRom[m_opcode].addr)();
    uint8_t operand = read8(address);

    operand & 0x01 ? setBit(CARRY) : clrBit(CARRY);
    operand >>= 1;
    write8(address, operand);
    m_a ^= operand;
    setZEROSIGN(m_a);
    return 0;
}

uint8_t CPU::STA() { write8((this->*m_instRom[m_opcode].addr)(), m_a); return 0; }
uint8_t CPU::STX() { write8((this->*m_instRom[m_opcode].addr)(), m_x); return 0; }
uint8_t CPU::STY() { write8((this->*m_instRom[m_opcode].addr)(), m_y); return 0; }

uint8_t CPU::TAX() {
    m_x = m_a;
    setZEROSIGN(m_x);
    return 0;
}

uint8_t CPU::TAY() {
    m_y = m_a;
    setZEROSIGN(m_y);
    return 0;
}

uint8_t CPU::TOP() { m_pc += 2; return 1; }

uint8_t CPU::TSX() {
    m_x = m_s;
    setZEROSIGN(m_x);
    return 0;
}

uint8_t CPU::TXA() {
    m_a = m_x;
    setZEROSIGN(m_a);
    return 0;
}

/*
 * Stack Pointer now points to address which is stored in X.
 */
uint8_t CPU::TXS() { m_s = m_x; return 0; }

uint8_t CPU::TYA() {
    m_a = m_y;
    setZEROSIGN(m_a);
    return 0;
}

