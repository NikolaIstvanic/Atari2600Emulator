#include "Atari.hpp"
#include "Timer.hpp"

Timer::Timer() { }

void Timer::step() {
    if (atari->tim1t) {
        atari->tim1t = 0;
        setInterval(atari->read8(TIM1T, true), 1);
    } else if (atari->tim8t) {
        atari->tim8t = 0;
        setInterval(atari->read8(TIM8T, true), 8);
    } else if (atari->tim64t) {
        atari->tim64t = 0;
        setInterval(atari->read8(TIM64T, true), 64);
    } else if (atari->t1024t) {
        atari->t1024t = 0;
        setInterval(atari->read8(T1024T, true), 1024);
    } else {
        if (--count == 0) {
            pulse();
        }
    }
}

void Timer::pulse() {
    if (atari->read8(INTIM) == 0x00) {
        // Underflow
        atari->write8(INSTAT, atari->read8(INSTAT, true) | 0xC0);
        atari->write8(INTIM, 0xFF);
        currTimer = 1;
        count = 0xFF;
    } else {
        count = currTimer;
        atari->write8(INTIM, atari->read8(INTIM, true) - 1);
    }
}

void Timer::setInterval(uint8_t value, uint16_t interval) {
    atari->write8(INTIM, value);
    atari->write8(INSTAT, atari->read8(INSTAT) & 0x3F);
    count = interval;
    currTimer = interval;
    pulse();
}

