#pragma once

#include <array>
#include <cstdint>

#include "olcPixelGameEngine.h"

#define WIDTH 160
#define HEIGHT 192

class Atari;

class TIA {
    public:
        TIA();
        ~TIA() = default;

        void connectAtari(Atari* a) { atari = a; }
        void reset();
        olc::Sprite& getScreen();
        void step();

        bool frameDone = false;

    private:
        inline olc::Pixel& getColor(uint8_t color);
        void drawPlayfield();
        void drawPlayer0();
        void drawPlayer1();
        void drawMissile0();
        void drawMissile1();
        void drawBall();

        enum {
            TIA_VSYNC,
            TIA_VBLANK,
            TIA_HBLANK,
            TIA_DRAW,
            TIA_OVERSCAN
        } state;

        uint8_t beamX = 0;
        uint16_t beamY = 0;
        uint32_t frameCounter = 0;

        uint8_t p0x = 0;
        uint8_t p1x = 0;
        uint8_t m0x = 0;
        uint8_t m1x = 0;
        uint8_t blx = 0;

        Atari* atari;

        olc::Sprite sprScreen = olc::Sprite(WIDTH, HEIGHT);
        std::array<std::array<olc::Pixel, 16>, 8> colorROM;
};

