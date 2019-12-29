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
 * Timer.cpp: class which encapsulates the Atari 2600's PIA timer.
 */
#include "Atari.hpp"
#include "Timer.hpp"

/**
 * @brief Perform one action for the PIA timer. If the interval has been changed
 * by the CPU, update it and restart the inner clock.
 */
void Timer::step() {
    if (atari->tim1t) {
        atari->tim1t = 0;
        setInterval(atari->read8(TIM1T), 1);
    } else if (atari->tim8t) {
        atari->tim8t = 0;
        setInterval(atari->read8(TIM8T), 8);
    } else if (atari->tim64t) {
        atari->tim64t = 0;
        setInterval(atari->read8(TIM64T), 64);
    } else if (atari->t1024t) {
        atari->t1024t = 0;
        setInterval(atari->read8(T1024T), 1024);
    }
    if (--count == 0) {
        pulse();
    }
}

/**
 * @brief Whenever the timer's inner clock has reached 0, update the CPU's INTIM
 * memory variable. If that has reached 0, then an underflow has occurred.
 * Here, the inner clock and interval are set to 1 so that the CPU can count
 * how many cycles ago the underflow occurred.
 */
void Timer::pulse() {
    if (atari->read8(INTIM) == 0x00) {
        // Underflow
        atari->write8(INSTAT, atari->read8(INSTAT) | 0xC0);
        atari->write8(INTIM, 0xFF);
        interval = 1;
        count = 1;
    } else {
        count = interval;
        atari->write8(INTIM, atari->read8(INTIM) - 1);
    }
}

/**
 * @brief Whenever the CPU changes its timer interval, update the timer's inner
 * timer interval and update the CPU's INTIM value to reflect this.
 */
void Timer::setInterval(uint8_t value, uint16_t clockInterval) {
    atari->write8(INTIM, value);
    atari->write8(INSTAT, atari->read8(INSTAT) & 0x3F);
    count = clockInterval;
    interval = clockInterval;
    pulse();
}

