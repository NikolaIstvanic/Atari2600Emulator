#pragma once

#include <array>
#include <cstdint>

#include "Timer.hpp"

#define RESET_VECTOR 0xFFFC
#define NMI_VECTOR 0xFFFA
#define IRQ_VECTOR 0xFFFE

class Atari;

class CPU {
    public:
        CPU();
        ~CPU() = default;
        void connectAtari(Atari* a) { atari = a; timer.connectAtari(a); }
        void reset();
        void step();
        void nmi();
        void irq();
        
        uint8_t cycles = 0;

        // STATUS REGISTER VALUES
        enum CPUFLAG {
            SIGN = 0x80,
            OVERFLOW = 0x40,
            CONSTANT = 0x20,
            BREAK = 0x10,
            DECIMAL= 0x08,
            INTERRUPT = 0x04,
            ZERO = 0x02,
            CARRY = 0x01
        };

    private:
        inline void logInfo();
        inline uint8_t fetch();
        inline uint16_t relativeOffset(uint8_t offset) const;
        inline void setBit(CPUFLAG f);
        inline void clrBit(CPUFLAG f);
        inline void setZEROSIGN(uint8_t value);

        void write8(uint16_t addr, uint8_t data);
        void write16(uint16_t addr, uint16_t data);
        uint8_t read8(uint16_t addr);
        uint16_t read16(uint16_t addr);
        uint8_t pop8();
        uint16_t pop16();
        void push8(uint8_t data);
        void push16(uint16_t data);

        // Memory addressing modes
        inline uint16_t ABS(); inline uint16_t ABX(); inline uint16_t ABY();
        inline uint16_t ACC(); inline uint16_t IDX(); inline uint16_t IDY();
        inline uint16_t IMM(); inline uint16_t IMP(); inline uint16_t IND();
        inline uint16_t REL(); inline uint16_t ZPX(); inline uint16_t ZPY(); 
        inline uint16_t ZRP();

        // Instruction set methods (including illegal opcodes)
        uint8_t ADC(); uint8_t ANC(); uint8_t AND(); uint8_t ANE(); uint8_t ARR(); uint8_t ASL();
        uint8_t ASR(); uint8_t BCC(); uint8_t BCS(); uint8_t BEQ(); uint8_t BIT(); uint8_t BMI();
        uint8_t BNE(); uint8_t BPL(); uint8_t BRK(); uint8_t BVC(); uint8_t BVS(); uint8_t CLC();
        uint8_t CLD(); uint8_t CLI(); uint8_t CLV(); uint8_t CMP(); uint8_t CPX(); uint8_t CPY();
        uint8_t DCP(); uint8_t DEC(); uint8_t DEX(); uint8_t DEY(); uint8_t DOP(); uint8_t EOR();
        uint8_t INC(); uint8_t INX(); uint8_t INY(); uint8_t ISB(); uint8_t JMP(); uint8_t JSR();
        uint8_t KIL(); uint8_t LAS(); uint8_t LAX(); uint8_t LDA(); uint8_t LDX(); uint8_t LDY();
        uint8_t LSR(); uint8_t LXA(); uint8_t NOP(); uint8_t ORA(); uint8_t PHA(); uint8_t PHP();
        uint8_t PLA(); uint8_t PLP(); uint8_t RLA(); uint8_t ROL(); uint8_t ROR(); uint8_t RRA();
        uint8_t RTI(); uint8_t RTS(); uint8_t SAX(); uint8_t SBC(); uint8_t SBX(); uint8_t SEC();
        uint8_t SED(); uint8_t SEI(); uint8_t SHA(); uint8_t SHS(); uint8_t SHX(); uint8_t SHY();
        uint8_t SLO(); uint8_t SRE(); uint8_t STA(); uint8_t STX(); uint8_t STY(); uint8_t TAX();
        uint8_t TAY(); uint8_t TOP(); uint8_t TSX(); uint8_t TXA(); uint8_t TXS(); uint8_t TYA();

        Timer timer;
        Atari* atari = nullptr;
        uint8_t additional_cycle = 0;

        // Accumulator register A
        uint8_t A = 0x00;

        // 8-bit index registers X and Y
        uint8_t X = 0x00;
        uint8_t Y = 0x00;

        // 8-bit processor status flag register
        uint8_t P = 0x00;

        // 8-bit stack pointer
        uint8_t S = 0x00;

        // Program Counter address register
        uint16_t PC = 0x0000;

        // Current opcode
        uint8_t opcode = 0x00;

        struct instruction {
            std::string name;
            uint8_t (CPU::*op)(void);
            uint16_t (CPU::*addr)(void);
            uint8_t cycles;
        };

        std::array<struct instruction, 0x100> instruction_rom;
};

