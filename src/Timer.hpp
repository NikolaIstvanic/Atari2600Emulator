#pragma once

class Atari;

class Timer {
public:
    Timer();
    ~Timer();

    void connectAtari(Atari* atari);
    void step();

private:
    void pulse();
    void setInterval(uint8_t value, uint16_t clockInterval);

    Atari* m_atari;
    uint16_t m_count = 1024;
    uint16_t m_interval = 1024;
};

