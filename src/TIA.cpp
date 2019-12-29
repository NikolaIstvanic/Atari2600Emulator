/**
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
 * TIA.cpp: class which encompasses the actions performed by the Atari 2600's
 * Television Interface Adaptor (TIA). In the Atari, the TIA performs all
 * graphics operations; these operations include drawing backgrounds for games,
 * displays and moving sprites on the screen, and selecting the colors which each
 * pixel will display as.
 *
 * Graphics on the Atari 2600 are intricately drawn on the screen with respect
 * to a moving electron beam present in the television. Writing a 2 to the
 * address 0x00 sends a signal from the Atari to the television indicating that
 * the beam should move back to the top of the screen. Writing the value 2 to
 * the address 0x01 sends a signal to the television that the beam should move
 * to a section right above the drawing area of the screen. Writing any value to
 * the address 0x02 sends a signal to the television for the beam to finish its
 * current scanline and move to the next (as well as position the beam at the
 * beginning of the drawing section of the screen). Pixel information for
 * backgrounds and sprites are read from their respective registers and only
 * drawn when the beam is in the draw area of the screen.
 *
 * For a diagram of how the scanline moves on the screen as well as how the
 * screen is divided, refer to page 2 of the Stella Programmer's Guide
 * (https://atarihq.com/danb/files/stella.pdf).
 */
#include "Atari.hpp"
#include "TIA.hpp"

/**
 * @brief Given an unigned byte whose four most significant bits form a value
 * from -8 to 7, return that value as a signed 8-bit value.
 */
static inline int8_t toSigned(uint8_t b) { return (b >> 4) + (b >> 7) * -0x10; }

/**
 * @brief Given an 8-bit number, return its bits reversed
 */
static inline uint8_t reverse(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0f) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

TIA::TIA() {
    colorROM = {{
		{ olc::Pixel(0, 0, 0),       olc::Pixel(68, 68, 0),     olc::Pixel(112, 40, 0),
          olc::Pixel(132, 24, 0),    olc::Pixel(136, 0, 0),     olc::Pixel(120, 0, 92),
          olc::Pixel(72, 0, 120),    olc::Pixel(20, 0, 132),    olc::Pixel(0, 0, 136),
          olc::Pixel(0, 24, 124),    olc::Pixel(0, 44, 92),     olc::Pixel(0, 60, 44),
          olc::Pixel(0, 60, 0),      olc::Pixel(20, 56, 0),     olc::Pixel(44, 48, 0),
          olc::Pixel(68, 40, 0) },
		{ olc::Pixel(64, 64, 64),    olc::Pixel(100, 100, 16),  olc::Pixel(132, 68, 20),
          olc::Pixel(152, 52, 24),   olc::Pixel(156, 32, 32),   olc::Pixel(140, 32, 116),
          olc::Pixel(96, 32, 144),   olc::Pixel(48, 32, 152),   olc::Pixel(28, 32, 156),
          olc::Pixel(28, 56, 144),   olc::Pixel(28, 76, 120),   olc::Pixel(28, 92, 72),
          olc::Pixel(32, 92, 32),    olc::Pixel(52, 92, 28),    olc::Pixel(76, 80, 28),
          olc::Pixel(100, 72, 24) },
		{ olc::Pixel(108, 108, 108), olc::Pixel(132, 132, 36),  olc::Pixel(152, 92, 40),
          olc::Pixel(172, 80, 48),   olc::Pixel(176, 60, 60),   olc::Pixel(160, 60, 136),
          olc::Pixel(120, 60, 164),  olc::Pixel(76, 60, 172),   olc::Pixel(56, 64, 176),
          olc::Pixel(56, 84, 168),   olc::Pixel(56, 104, 144),  olc::Pixel(56, 124, 100),
          olc::Pixel(64, 124, 64),   olc::Pixel(80, 124, 56),   olc::Pixel(104, 112, 52),
          olc::Pixel(132, 104, 48) } ,
		{ olc::Pixel(144, 144, 144), olc::Pixel(160, 160, 52),  olc::Pixel(172, 120, 60),
          olc::Pixel(192, 104, 72),  olc::Pixel(192, 88, 88),   olc::Pixel(176, 88, 156),
          olc::Pixel(140, 88, 184),  olc::Pixel(104, 88, 192),  olc::Pixel(80, 92, 192),
          olc::Pixel(80, 112, 188),  olc::Pixel(80, 132, 172),  olc::Pixel(80, 156, 128),
          olc::Pixel(92, 156, 92),   olc::Pixel(108, 152, 80),  olc::Pixel(132, 140, 76),
          olc::Pixel(160, 132, 68) }, 
		{ olc::Pixel(176, 176, 176), olc::Pixel(184, 184, 64),  olc::Pixel(188, 140, 76),
          olc::Pixel(208, 128, 92),  olc::Pixel(208, 112, 112), olc::Pixel(192, 112, 176),
          olc::Pixel(160, 112, 204), olc::Pixel(124, 112, 208), olc::Pixel(104, 116, 208),
          olc::Pixel(104, 136, 204), olc::Pixel(104, 156, 192), olc::Pixel(104, 180, 148),
          olc::Pixel(116, 180, 116), olc::Pixel(132, 180, 104), olc::Pixel(156, 168, 100),
          olc::Pixel(184, 156, 88) }, 
		{ olc::Pixel(200, 200, 200), olc::Pixel(208, 208, 80),  olc::Pixel(204, 160, 92),
          olc::Pixel(224, 148, 112), olc::Pixel(224, 136, 136), olc::Pixel(208, 132, 192),
          olc::Pixel(180, 132, 220), olc::Pixel(148, 136, 224), olc::Pixel(124, 140, 224),
          olc::Pixel(124, 156, 220), olc::Pixel(124, 180, 212), olc::Pixel(124, 208, 172),
          olc::Pixel(140, 208, 140), olc::Pixel(156, 204, 124), olc::Pixel(180, 192, 120),
          olc::Pixel(208, 180, 108) }, 
		{ olc::Pixel(220, 220, 220), olc::Pixel(232, 232, 92),  olc::Pixel(220, 180, 104),
          olc::Pixel(236, 168, 128), olc::Pixel(236, 160, 160), olc::Pixel(220, 156, 208),
          olc::Pixel(196, 156, 236), olc::Pixel(168, 160, 236), olc::Pixel(144, 164, 236),
          olc::Pixel(144, 180, 236), olc::Pixel(144, 204, 232), olc::Pixel(144, 228, 192),
          olc::Pixel(164, 228, 164), olc::Pixel(180, 228, 144), olc::Pixel(204, 212, 136),
          olc::Pixel(232, 204, 124) }, 
		{ olc::Pixel(244, 244, 244), olc::Pixel(252, 252, 104), olc::Pixel(236, 200, 120),
          olc::Pixel(252, 188, 148), olc::Pixel(252, 180, 180), olc::Pixel(236, 176, 224),
          olc::Pixel(212, 176, 252), olc::Pixel(188, 180, 252), olc::Pixel(164, 184, 252),
          olc::Pixel(164, 200, 252), olc::Pixel(164, 224, 252), olc::Pixel(164, 252, 212),
          olc::Pixel(184, 252, 184), olc::Pixel(200, 252, 164), olc::Pixel(224, 236, 156),
          olc::Pixel(252, 224, 140) }
	}};
}

/**
 * @brief Reset beam position to its initial location; reset state to VSYNC.
 */
void TIA::reset() {
    beamX = 0;
    beamY = 0;
    state = TIA_VSYNC;
}

/**
 * @brief Getter for the sprites that make up the screen.
 */
olc::Sprite& TIA::getScreen() { return sprScreen; }

/**
 * @brief Given a color variable, break it into luminosity and color to index
 * into the color ROM.
 * @return The color from the Atari 2600's palette for the given luminosity
 * and color
 */
inline olc::Pixel& TIA::getColor(uint8_t color) {
    return colorROM[(color & 0x0F) >> 1][color >> 4];
}

/**
 * @brief Whenever the electron beam is in the draw section of the screen, draw
 * the playfield/background of the ROM. The playfield is a 20-bit value which
 * represents pixels that are either the playfield's color or the background's
 * color. This 20-bit value only covers half of the screen; the second half of
 * the screen is determined by the value in the CTRLPF register. If this value
 * is 1, then the right half of the screen is a mirror of the left half;
 * otherwise, the right half is the same as the left half.
 */
void TIA::drawPlayfield() {
    uint8_t colupf;
    uint32_t line = reverse(atari->read8(PF2)) | (atari->read8(PF1) << 8)
        | (reverse(atari->read8(PF0)) << 16);
    uint8_t colubk = atari->read8(COLUBK);
    uint16_t y = beamY - 40;

    if (atari->read8(CTRLPF) & 0x02) {
        // Scoreboard mode
        colupf = atari->read8(COLUP0);
    } else {
        colupf = atari->read8(COLUPF);
    }

    olc::Pixel pfColor = getColor(colupf);
    olc::Pixel bgColor = getColor(colubk);

    // Draw the left half of the screen
    for (int i = 19; i >= 0; i--) {
        olc::Pixel color = (line >> i) & 0x01 ? pfColor : bgColor;
        sprScreen.SetPixel((19 - i) * 4, y, color);
        sprScreen.SetPixel((19 - i) * 4 + 1, y, color);
        sprScreen.SetPixel((19 - i) * 4 + 2, y, color);
        sprScreen.SetPixel((19 - i) * 4 + 3, y, color);
    }

    if (atari->read8(CTRLPF) & 0x01) {
        // Mirror right side of the screen
        line = (atari->read8(PF0) >> 4) | (reverse(atari->read8(PF1)) << 4)
            | (atari->read8(PF2) << 12);
    }
    if (atari->read8(CTRLPF) & 0x02) {
        // Scoreboard mode
        colupf = atari->read8(COLUP1);
        pfColor = getColor(colupf);
    }

    // Draw other the other half of the screen
    for (int i = 19; i >= 0; i--) {
        olc::Pixel color = (line >> i) & 0x01 ? pfColor : bgColor;
        sprScreen.SetPixel(WIDTH / 2 + (19 - i) * 4, y, color);
        sprScreen.SetPixel(WIDTH / 2 + (19 - i) * 4 + 1, y, color);
        sprScreen.SetPixel(WIDTH / 2 + (19 - i) * 4 + 2, y, color);
        sprScreen.SetPixel(WIDTH / 2 + (19 - i) * 4 + 3, y, color);
    }
}

/**
 * @brief Whenever the TIA beam is in the draw state, draw the sprite for
 * player 0 whose x position is determined by the beam's x position.
 */
void TIA::drawPlayer0() {
    uint8_t sprite;
    uint8_t colup0 = atari->read8(COLUP0);
    olc::Pixel spriteColor = getColor(colup0);
    uint16_t y = beamY - 40;

    if (atari->read8(REFP0) & 0x10) {
        sprite = atari->read8(GRP0);
    } else {
        sprite = reverse(atari->read8(GRP0));
    }

    for (int i = 0; i < 8; i++) {
        olc::Pixel color = (sprite >> i ) & 0x01 ? spriteColor
            : sprScreen.GetPixel((p0x + i) % WIDTH, y);
        sprScreen.SetPixel((p0x + i) % WIDTH, y, color);
    }
}

/**
 * @brief Whenever the TIA beam is in the draw state, draw the sprite for
 * player 1 whose x position is determined by the beam's x position.
 */
void TIA::drawPlayer1() {
    uint8_t sprite;
    uint8_t colup1 = atari->read8(COLUP1);
    olc::Pixel spriteColor = getColor(colup1);
    uint16_t y = beamY - 40;

    if (atari->read8(REFP1) & 0x10) {
        sprite = atari->read8(GRP1);
    } else {
        sprite = reverse(atari->read8(GRP1));
    }

    for (int i = 0; i < 8; i++) {
        olc::Pixel color = (sprite >> i) & 0x01 ? spriteColor
            : sprScreen.GetPixel((p1x + i) % WIDTH, y);
        sprScreen.SetPixel((p1x + i) % WIDTH, y, color);
    }
}

/**
 * @brief Whenever the TIA beam is in the draw state, draw the sprite for
 * missile 0 whose x position is determined by the beam's x position.
 */
void TIA::drawMissile0() {
    if (!(atari->read8(ENAM0) & 0x02)) {
        return;
    }

    uint16_t y = beamY - 40;
    uint8_t colup0 = atari->read8(COLUP0);
    olc::Pixel color = getColor(colup0);
    uint8_t size = 1 << ((atari->read8(NUSIZ0) >> 4) & 0x03);

    for (int i = 0; i < size; i++) {
        sprScreen.SetPixel((m0x + i) % WIDTH, y, color);
    }
}

/**
 * @brief Whenever the TIA beam is in the draw state, draw the sprite for
 * missile 1 whose x position is determined by the beam's x position.
 */
void TIA::drawMissile1() {
    if (!(atari->read8(ENAM1) & 0x02)) {
        return;
    }

    uint16_t y = beamY - 40;
    uint8_t colup1 = atari->read8(COLUP1);
    olc::Pixel color = getColor(colup1);
    uint8_t size = 1 << ((atari->read8(NUSIZ1) >> 4) & 0x03);

    for (int i = 0; i < size; i++) {
        sprScreen.SetPixel((m1x + i) % WIDTH, y, color);
    }
}

/**
 * @brief Whenever the TIA beam is in the draw state, draw the sprite for
 * the ball whose x position is determined by the beam's x position.
 */
void TIA::drawBall() {
    if (!(atari->read8(ENABL) & 0x02)) {
        return;
    }

    uint16_t y = beamY - 40;
    uint8_t colupf = atari->read8(COLUPF);
    olc::Pixel color = getColor(colupf);
    uint8_t size = 1 << ((atari->read8(CTRLPF) >> 4) & 0x03);

    for (int i = 0; i < size; i++) {
        sprScreen.SetPixel((blx + i) % WIDTH, y, color);
    }
}

/**
 * @brief Perform one step of the TIA's operations. Simulate moving the electron
 * beam, drawing sprites, and altering TIA states according to beam positioning
 * as well as data from RAM.
 */
void TIA::step() {
    frameCounter++;
    if (frameCounter >= 208 * 262) {
        frameDone = true;
        frameCounter = 0;
    }

    if ((atari->read8(VSYNC) & 0x02) && !(0 <= beamY && beamY <= 3)) {
        beamX = 0;
        beamY = 0;
        state = TIA_VSYNC;
    } else if (atari->wsync) {
        atari->wsync = 0;
        if (40 <= beamY && beamY <= 231) {
            drawPlayfield();
            drawPlayer0();
            drawPlayer1();
            drawMissile0();
            drawMissile1();
            drawBall();
        }
        beamY++;
        beamX = 0;

        if (beamY >= 262) {
            beamY = 0;
            state = TIA_VSYNC;
        } else if (beamY >= 232) {
            state = TIA_OVERSCAN;
        } else if (beamY >= 40) {
            state = TIA_HBLANK;
        } else if (beamY >= 3) {
            state = TIA_VBLANK;
        } else {
            state = TIA_VSYNC;
        }
    } else if (atari->resp0) {
        atari->resp0 = 0;
        if (beamX <= 68) {
            p0x = 3;
        } else {
            p0x = beamX - 68;
        }
    } else if (atari->resp1) {
        atari->resp1 = 0;
        if (beamX <= 68) {
            p1x = 3;
        } else {
            p1x = beamX - 68;
        }
    } else if (atari->resm0) {
        atari->resm0 = 0;
        if (beamX <= 68) {
            m0x = 2;
        } else {
            m0x = beamX - 68;
        }
    } else if (atari->resm1) {
        atari->resm1 = 0;
        if (beamX <= 68) {
            m1x = 2;
        } else {
            m1x = beamX - 68;
        }
    } else if (atari->resbl) {
        atari->resbl = 0;
        if (beamX <= 68) {
            blx = 2;
        } else {
            blx = beamX - 68;
        }
    } else if (atari->hmove) {
        atari->hmove = 0;
        p0x = (((int16_t) p0x - toSigned(atari->read8(HMP0))) + WIDTH) % WIDTH;
        p1x = (((int16_t) p1x - toSigned(atari->read8(HMP1))) + WIDTH) % WIDTH;
        m0x = (((int16_t) m0x - toSigned(atari->read8(HMM0))) + WIDTH) % WIDTH;
        m1x = (((int16_t) m1x - toSigned(atari->read8(HMM1))) + WIDTH) % WIDTH;
        blx = (((int16_t) blx - toSigned(atari->read8(HMBL))) + WIDTH) % WIDTH;
    } else if (atari->hmclr) {
        atari->hmclr = 0;
        atari->write8(HMP0, 0x00);
        atari->write8(HMP1, 0x00);
        atari->write8(HMM0, 0x00);
        atari->write8(HMM1, 0x00);
        atari->write8(HMBL, 0x00);
    }

    beamX++;
    switch (state) {
        case TIA_VSYNC:
            if (beamX >= 228) {
                beamX %= 228;
                beamY++;

                if (beamY >= 3) {
                    state = TIA_VBLANK;
                }
            }
            break;

        case TIA_VBLANK:
            if (beamX >= 228) {
                beamX %= 228;
                beamY++;

                if (beamY >= 40) {
                    state = TIA_HBLANK;
                }
            }
            break;

        case TIA_HBLANK:
            if (beamX >= 68) {
                state = TIA_DRAW;
            }
            break;

        case TIA_DRAW:
            if (beamX >= 228) {
                beamX %= 228;
                beamY++;

                if (beamY >= 232) {
                    state = TIA_OVERSCAN;
                    break;
                } else {
                    state = TIA_HBLANK;
                }
            }
            drawPlayfield();
            drawPlayer0();
            drawPlayer1();
            drawMissile0();
            drawMissile1();
            drawBall();
            break;

        case TIA_OVERSCAN:
            if (beamX >= 228) {
                beamX %= 228;
                beamY++;

                if (beamY >= 262) {
                    beamY = 0;
                    state = TIA_VSYNC;
                    frameDone = true;
                }
            }
            break;
    }
#ifdef DEBUG
    std::cout << "beamX: " << (int) beamX << " beamY: " << (int) beamY
        << std::endl;
#endif
}

