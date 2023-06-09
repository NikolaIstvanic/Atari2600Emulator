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
    ~TIA();

    void connectAtari(Atari* atari);
    void reset();
    olc::Sprite& getScreen();
    void step();

    bool m_frameDone = false;

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
    } m_state;

    uint8_t m_beamX = 0;
    uint16_t m_beamY = 0;
    uint32_t m_frameCounter = 0;

    uint8_t m_p0x = 0;
    uint8_t m_p1x = 0;
    uint8_t m_m0x = 0;
    uint8_t m_m1x = 0;
    uint8_t m_blx = 0;

    Atari* m_atari;

    olc::Sprite m_sprScreen = olc::Sprite(WIDTH, HEIGHT);
    std::array<std::array<olc::Pixel, 16>, 8> m_colorRom;
};

