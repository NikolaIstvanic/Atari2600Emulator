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

Timer::Timer() = default;
Timer::~Timer() = default;

void Timer::connectAtari(Atari* atari) {
    m_atari = atari;
}

/**
 * @brief Perform one action for the PIA timer. If the interval has been changed by the CPU, update
 * it and restart the inner clock.
 */
void Timer::step() {
    if (m_atari->m_tim1t) {
        m_atari->m_tim1t = 0;
        setInterval(m_atari->read8(TIM1T), 1);
    } else if (m_atari->m_tim8t) {
        m_atari->m_tim8t = 0;
        setInterval(m_atari->read8(TIM8T), 8);
    } else if (m_atari->m_tim64t) {
        m_atari->m_tim64t = 0;
        setInterval(m_atari->read8(TIM64T), 64);
    } else if (m_atari->m_t1024t) {
        m_atari->m_t1024t = 0;
        setInterval(m_atari->read8(T1024T), 1024);
    }

    if (--m_count == 0) {
        pulse();
    }
}

/**
 * @brief Whenever the timer's inner clock has reached 0, update the CPU's INTIM memory variable.
 * If that has reached 0, then an underflow has occurred. Here, the inner clock and interval are
 * set to 1 so that the CPU can count how many cycles ago the underflow occurred.
 */
void Timer::pulse() {
    if (m_atari->read8(INTIM) == 0x00) {
        // Underflow
        m_atari->write8(INSTAT, m_atari->read8(INSTAT) | 0xC0);
        m_atari->write8(INTIM, 0xFF);
        m_interval = 1;
        m_count = 1;
    } else {
        m_count = m_interval;
        m_atari->write8(INTIM, m_atari->read8(INTIM) - 1);
    }
}

/**
 * @brief Whenever the CPU changes its timer interval, update the timer's inner timer interval and
 * update the CPU's INTIM value to reflect this.
 */
void Timer::setInterval(uint8_t value, uint16_t clockInterval) {
    m_atari->write8(INTIM, value);
    m_atari->write8(INSTAT, m_atari->read8(INSTAT) & 0x3F);
    m_count = clockInterval;
    m_interval = clockInterval;
    pulse();
}

