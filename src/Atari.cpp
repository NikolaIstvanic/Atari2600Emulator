#include <algorithm>

#include "Atari.hpp"

Atari::Atari() {
    cpu.connectAtari(this);
    tia.connectAtari(this);
}

void Atari::reset() {
    cpu.reset();
    tia.reset();
}

void Atari::step() {
    tia.step();

    if (clocks % 3 == 0) {
        cpu.step();
    }
    clocks++;
}

uint8_t Atari::read8(uint16_t addr, bool bReadOnly) {
    if (bReadOnly) {
        return RAM[addr];
    }
    // TODO: fill this in
    return RAM[addr];
}

uint16_t Atari::read16(uint16_t addr, bool bReadOnly) {
    return (read8(addr + 1, bReadOnly) << 8) | read8(addr, bReadOnly);
}

void Atari::write8(uint16_t addr, uint8_t data) {
    RAM[addr] = data;

    if (addr == WSYNC) {
        wsync = 1;
    } else if (addr == RESP0) {
        resp0 = 1;
    } else if (addr == RESP1) {
        resp1 = 1;
    } else if (addr == RESM0) {
        resm0 = 1;
    } else if (addr == RESM1) {
        resm1 = 1;
    } else if (addr == RESBL) {
        resbl = 1;
    } else if (addr == HMOVE) {
        hmove = 1;
    } else if (addr == TIM1T) {
        tim1t = 1;
    } else if (addr == TIM8T) {
        tim8t = 1;
    } else if (addr == TIM64T) {
        tim64t = 1;
    } else if (addr == T1024T) {
        t1024t = 1;
    }
}

void Atari::write16(uint16_t addr, uint16_t data) {
    write8(addr, data & 0x00FF);
    write8(addr + 1, data >> 8);
}

