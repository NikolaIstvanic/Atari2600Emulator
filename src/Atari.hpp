#pragma once

#include <cstdint>

#include "CPU.hpp"
#include "TIA.hpp"

#define VSYNC  0x00
#define VBLANK 0x01
#define WSYNC  0x02
#define NUSIZ0 0x04
#define NUSIZ1 0x05
#define COLUP0 0x06
#define COLUP1 0x07
#define COLUPF 0x08
#define COLUBK 0x09
#define CTRLPF 0x0A
#define REFP0  0x0B
#define REFP1  0x0C
#define PF0    0x0D
#define PF1    0x0E
#define PF2    0x0F
#define RESP0  0x10
#define RESP1  0x11
#define RESM0  0x12
#define RESM1  0x13
#define RESBL  0x14
#define GRP0   0x1B
#define GRP1   0x1C
#define ENAM0  0x1D
#define ENAM1  0x1E
#define ENABL  0x1F
#define HMP0   0x20
#define HMP1   0x21
#define HMM0   0x22
#define HMM1   0x23
#define HMBL   0x24
#define HMOVE  0x2A
#define HMCLR  0x2B
#define CXCLR  0x2C
#define INPT4  0x3C
#define INPT5  0x3D

#define SWCHA  0x280
#define SWCHB  0x282
#define INTIM  0x284
#define INSTAT 0x285
#define TIM1T  0x294
#define TIM8T  0x295
#define TIM64T 0x296
#define T1024T 0x297

#define SIZE_RAM 0x10000

class Atari {
public:
    Atari();
    ~Atari();

    void reset();
    void step();
    uint8_t read8(uint16_t addr);
    uint16_t read16(uint16_t addr);
    void write8(uint16_t addr, uint8_t data);
    void write16(uint16_t addr, uint16_t data);

    TIA m_tia;
    CPU m_cpu;
    std::array<uint8_t, SIZE_RAM> m_ram;

    // Strobe registers
    uint8_t m_wsync = 0;
    uint8_t m_resp0 = 0;
    uint8_t m_resp1 = 0;
    uint8_t m_resm0 = 0;
    uint8_t m_resm1 = 0;
    uint8_t m_resbl = 0;
    uint8_t m_hmove = 0;
    uint8_t m_hmclr = 0;
    uint8_t m_tim1t = 0;
    uint8_t m_tim8t = 0;
    uint8_t m_tim64t = 0;
    uint8_t m_t1024t = 0;

private:
    uint16_t m_clocks = 0;
};

