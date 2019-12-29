#pragma once

class Atari;

class Timer {
    public:
        Timer() = default;
        ~Timer() = default;

        void connectAtari(Atari* a) { atari = a; }
        void step();

    private:
        void pulse();
        void setInterval(uint8_t value, uint16_t clockInterval);

        Atari* atari;
        uint16_t count = 1024;
        uint16_t interval = 1024;
};

