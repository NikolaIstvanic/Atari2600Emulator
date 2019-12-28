#pragma once

class Atari;

class Timer {
    public:
        Timer();
        ~Timer() = default;

        void connectAtari(Atari* a) { atari = a; }
        void step();

    private:
        void pulse();
        void setInterval(uint8_t value, uint16_t inter);

        Atari* atari;
        uint16_t count = 1024;
        uint16_t currTimer = 1024;
};

