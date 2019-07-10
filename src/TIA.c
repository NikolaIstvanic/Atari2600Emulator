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
 * TIA.c: C file which encompasses the actions performed by the Atari 2600's
 * Television Interface Adaptor (TIA). In the Atari, the TIA performs all
 * graphics operations. These operations include drawing backgrounds for games,
 * displays and moves sprites on the screen, and selects the colors which each
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
#include <string.h>

#include "MMU.h"
#include "TIA.h"

// TODO: remove magic numbers
static uint8_t p0x = 120;
static uint8_t p1x = 200;
static uint8_t m0x = 220;
static uint8_t m1x = 10;

uint32_t color_rom[8][16] = {
    { 0x000000, 0x004444, 0x002870, 0x001884, 0x000088, 0x5C0078, 0x780048, 0x840014,
      0x880000, 0x7C1800, 0x5C2C00, 0x2C3C00, 0x003C00, 0x003814, 0x00302C, 0x002844 },
    { 0x404040, 0x106464, 0x144484, 0x183498, 0x20209C, 0x74208C, 0x902060, 0x982030,
      0x9C201C, 0x90381C, 0x784C1C, 0x485C1C, 0x205C20, 0x1C5C34, 0x1C504C, 0x184864 },
    { 0x6C6C6C, 0x248484, 0x285C98, 0x3050AC, 0x3C3CB0, 0x883CA0, 0xA43C78, 0xAC3C4C,
      0xB04038, 0xA85438, 0x906838, 0x647C38, 0x407C40, 0x387C50, 0x347068, 0x306884 },
    { 0x909090, 0x34A0A0, 0x3C78AC, 0x4868C0, 0x5858C0, 0x9C58B0, 0xB8588C, 0xC05868,
      0xC05C50, 0xBC7050, 0xAC8450, 0x809C50, 0x5C9C5C, 0x50986C, 0x4C8C84, 0x4484A0 },
    { 0xB0B0B0, 0x40B8B8, 0x4C8CBC, 0x5C80D0, 0x7070D0, 0xB070C0, 0xCC70A0, 0xD0707C,
      0xD07468, 0xCC8868, 0xC09C68, 0x94B468, 0x74B474, 0x68B484, 0x64A89C, 0x589CB8 },
    { 0xC8C8C8, 0x50D0D0, 0x5CA0CC, 0x7094E0, 0x8888E0, 0xC084D0, 0xDC84B4, 0xE08894,
      0xE08C7C, 0xDC9C7C, 0xD4B47C, 0xACD07C, 0x8CD08C, 0x7CCC9C, 0x78C0B4, 0x6CB4D0 },
    { 0xDCDCDC, 0x5CE8E8, 0x68B4DC, 0x80A8EC, 0xA0A0EC, 0xD09CDC, 0xEC9CC4, 0xECA0A8,
      0xECA490, 0xECB490, 0xE8CC90, 0xC0E490, 0xA4E4A4, 0x90E4B4, 0x88D4CC, 0x7CCCE8 },
    { 0xF4F4F4, 0x68FCFC, 0x78C8EC, 0x94BCFC, 0xB4B4FC, 0xE0B0EC, 0xFCB0D4, 0xFCB4BC,
      0xFCB8A4, 0xFCC8A4, 0xFCE0A4, 0xD4FCA4, 0xB8FCB8, 0xA4FCC8, 0x9CECE0, 0x8CE0FC }
};

/*
 * Given a TIA structure pointer, initialize all of its fields to their
 * appropriate starting values.
 *
 * This means all of the pixels of the TIA's version of the screen should be
 * zeroed out, the beam's x and y position are 0, and the state of the TIA is
 * in its initial VSYNC area.
 */
void tia_init(TIA* tia)
{
    memset(tia->pixels, 0, sizeof(uint32_t) * WIDTH * HEIGHT);
    tia->beam_x = 0;
    tia->beam_y = 0;
    tia->tia_state = TIA_VSYNC;
}

/*
 * Given an 8-bit number, return its bits reversed.
 */
static inline uint8_t reverse(uint8_t bits)
{
    uint8_t count = sizeof(bits) * 8 - 1;
    uint8_t reverse = bits;

    bits >>= 1;
    while (bits) {
        reverse <<= 1;
        reverse |= bits & 1;
       bits >>= 1;
       count--;
    }
    return reverse << count;
}

/*
 * Use the 8-bit integer parameter as an index into the color ROM defined below.
 * This method will return the 32-bit color value of which ever value is in the
 * selected color register.
 */
static inline uint32_t color_lookup(uint8_t luminosity, uint8_t color)
{
    return color_rom[luminosity][color];
}

/*
 * Whenever the electron beam is in the draw section of the screen, draw the
 * playfield/background of the ROM. The playfield is a 20-bit value which
 * represents pixels that are either the playfield's color or the background's
 * color. This 20-bit value only covers half of the screen; the second half of
 * the screen is determined by the value in the CTRLPF register. If this value
 * is 1, then the right half of the screen is a mirror of the left half;
 * otherwise, the right half is the same as the left half.
 */
static void draw_playfield(CPU* cpu, TIA* tia)
{
    int i;
    int j;
    uint32_t line = reverse(read8(cpu, PF2)) | (read8(cpu, PF1) << 8)
        | (reverse(read8(cpu, PF0)) << 16);
    uint8_t colupf = read8(cpu, COLUPF);
    uint8_t colubk = read8(cpu, COLUBK);
    uint32_t play_clr = color_lookup((colupf & 0xF) >> 1, colupf >> 4);
    uint32_t bg_clr = color_lookup((colubk & 0xF) >> 1, colubk >> 4);
    uint16_t y = tia->beam_y - VBLANK_MAX;
    uint32_t pixel_width = (WIDTH / 2) / 20;

    for (i = 19; i >= 0; i--) {
        uint32_t clr = (line >> i) & 0x1 ? play_clr : bg_clr;

        for (j = 0; j < pixel_width; j++) {
            tia->pixels[y * WIDTH + (19 - i) * pixel_width + j] = clr;
        }
    }

    for (i = 19; i >= 0; i--) {
        if (read8(cpu, CTRLPF) & 0x1) {
            line = (read8(cpu, PF0) >> 4) | (reverse(read8(cpu, PF1)) << 4)
                | (read8(cpu, PF2) << 12);
        }
        uint32_t clr = (line >> i) & 0x1 ? play_clr : bg_clr;

        for (j = 0; j < pixel_width; j++) {
            tia->pixels[y * WIDTH + (WIDTH / 2) + (19 - i) * pixel_width + j] = clr;
        }
    }
}

/*
 * Whenever the TIA beam is in the draw state, draw the sprite for player 0
 * whose x position is determined by the beam's x position.
 */
static void draw_player0(CPU* cpu, TIA* tia)
{
    int i;
    uint8_t sprite;
    if (read8(cpu, REFP0) & 0x8) {
        sprite = read8(cpu, GRP0);
    } else {
        sprite = reverse(read8(cpu, GRP0));
    }
    uint8_t colup0 = read8(cpu, COLUP0);
    uint32_t sprite_color = color_lookup((colup0 & 0xF) >> 1, colup0 >> 4);
    uint16_t y = tia->beam_y - VBLANK_MAX;

    for (i = 0; i < 16; i++) {
        uint32_t color = (sprite >> i / 2) & 0x1 ? sprite_color
            : tia->pixels[y * WIDTH + p0x + i];
        tia->pixels[y * WIDTH + p0x + i] = color;
    }
}

/*
 * Whenever the TIA beam is in the draw state, draw the sprite for player 1
 * whose x position is determined by the beam's x position.
 */
static void draw_player1(CPU* cpu, TIA* tia)
{
    int i;
    uint8_t sprite;
    if (read8(cpu, REFP1) & 0x8) {
        sprite = read8(cpu, GRP1);
    } else {
        sprite = reverse(read8(cpu, GRP1));
    }
    uint8_t colup1 = read8(cpu, COLUP1);
    uint32_t sprite_color = color_lookup((colup1 & 0xF) >> 1, colup1 >> 4);
    uint16_t y = tia->beam_y - VBLANK_MAX;

    for (i = 0; i < 16; i++) {
        uint32_t color = (sprite >> (i / 2)) & 0x1 ? sprite_color
            : tia->pixels[y * WIDTH + p1x + i];
        tia->pixels[y * WIDTH + p1x + i] = color;
    }
}

static inline void draw_missile0(CPU* cpu, TIA* tia)
{
    if (!(read8(cpu, ENAM0) & 0x2)) {
        return;
    }

    int i;
    uint16_t y = tia->beam_y - VBLANK_MAX;
    uint8_t colup0 = read8(cpu, COLUP0);
    uint32_t sprite_color = read8(cpu, COLUP0) & 0x02
        ? color_lookup((colup0 & 0xF) >> 1, colup0 >> 4)
        : tia->pixels[y * WIDTH + m0x];
    uint8_t size = read8(cpu, NUSIZ0);
    for (i = 0; i < size; i++) {
        tia->pixels[y * WIDTH + m0x + i] = sprite_color;
    }
}

static inline void draw_missile1(CPU* cpu, TIA* tia)
{
    if (!(read8(cpu, ENAM1) & 0x2)) {
        return;
    }

    int i;
    uint16_t y = tia->beam_y - VBLANK_MAX;
    uint8_t colup1 = read8(cpu, COLUP1);
    uint32_t sprite_color = read8(cpu, COLUP1) & 0x02
        ? color_lookup((colup1 & 0xF) >> 1, colup1 >> 4)
        : tia->pixels[y * WIDTH + m1x];
    uint8_t size = read8(cpu, NUSIZ1);
    for (i = 0; i < size; i++) {
        tia->pixels[y * WIDTH + m1x + i] = sprite_color;
    }
}

/*
 * Perform one operation of the TIA. This means move the beam to its appropriate
 * position after the CPU's current instruction has executed and check to see
 * whether any draw operations need to be performed.
 */
void tia_step(CPU* cpu, TIA* tia)
{
    if ((read8(cpu, VSYNC) & 0x02) && !(0 <= tia->beam_y && tia->beam_y <= VBLANK_MIN)) {
        tia->beam_x = 0;
        tia->beam_y = 0;
        tia->tia_state = TIA_VSYNC;
    /*} else if ((read8(cpu, VBLANK) & 0x02) && !(VBLANK_MIN <= tia->beam_y && tia->beam_y < VBLANK_MAX)) {
        tia->beam_x = 0;
        tia->beam_y = VBLANK_MIN;
        tia->tia_state = TIA_VBLANK;
        write8(cpu, VBLANK, 0x00);
     */
    } else if (wsync) {
        wsync = 0;
        if (VBLANK_MAX <= tia->beam_y && tia->beam_y < DRAW_MAX) {
            draw_playfield(cpu, tia);
            draw_player0(cpu, tia);
            draw_player1(cpu, tia);
            draw_missile0(cpu, tia);
            draw_missile1(cpu, tia);
        }
        tia->beam_y++;
        tia->beam_x = 0;

        if (tia->beam_y >= MAX_Y) {
            tia->beam_y = 0;
            tia->tia_state = TIA_VSYNC;
        } else if (tia->beam_y >= DRAW_MAX) {
            tia->tia_state = TIA_OVERSCAN;
        } else if (tia->beam_y >= VBLANK_MAX) {
            tia->tia_state = TIA_HBLANK;
        } else if (tia->beam_y >= VBLANK_MIN) {
            tia->tia_state = TIA_VBLANK;
        } else {
            tia->tia_state = TIA_VSYNC;
        }
        //return; // TODO: determine if this return is needed
    } else if (resp0) {
        resp0 = 0;
        if (tia->tia_state == TIA_DRAW) {
            p0x = tia->beam_x;
            draw_player0(cpu, tia);
        } else if (tia->tia_state == TIA_HBLANK) {
            p0x = HBLANK_MAX + 3;
            draw_player0(cpu, tia);
        }
    } else if (resp1) {
        resp1 = 0;
        if (tia->tia_state == TIA_DRAW) {
            p1x = tia->beam_x;
            draw_player1(cpu, tia);
        } else if (tia->tia_state == TIA_HBLANK) {
            p1x = HBLANK_MAX + 3;
            draw_player1(cpu, tia);
        }
    } else if (resm0) {
        resm0 = 0;
        if (tia->tia_state == TIA_DRAW) {
            m0x = tia->beam_x;
            draw_missile0(cpu, tia);
        } else if (tia->tia_state == TIA_HBLANK) {
            m0x = HBLANK_MAX + 2;
            draw_missile0(cpu, tia);
        }
    } else if (resm1) {
        resm1 = 0;
        if (tia->tia_state == TIA_DRAW) {
            m1x = tia->beam_x;
            draw_missile1(cpu, tia);
        } else if (tia->tia_state == TIA_HBLANK) {
            m1x = HBLANK_MAX + 2;
            draw_missile1(cpu, tia);
        }
    } else if (resbl) {
        resbl = 0;
    }

    tia->beam_x += 3 * cpu->cycles;

    switch (tia->tia_state) {
        case TIA_VSYNC:
            if (tia->beam_x >= MAX_X) {
                tia->beam_x %= MAX_X;
                tia->beam_y++;
            
                if (tia->beam_y >= VBLANK_MIN) {
                    tia->tia_state = TIA_VBLANK;
                }
            }
            break;

        case TIA_VBLANK:
            if (tia->beam_x >= MAX_X) {
                tia->beam_x %= MAX_X;
                tia->beam_y++;

                if (tia->beam_y >= VBLANK_MAX) {
                    tia->tia_state = TIA_HBLANK;
                }
            }
            break;

        case TIA_HBLANK:
            if (tia->beam_x >= HBLANK_MAX) {
                tia->tia_state = TIA_DRAW;
            }
            break;

        case TIA_DRAW:
            if (tia->beam_x >= MAX_X) {
                tia->beam_x %= MAX_X;
                tia->beam_y++;

                if (tia->beam_y >= DRAW_MAX) {
                    tia->tia_state = TIA_OVERSCAN;
                    break;
                } else {
                    tia->tia_state = TIA_HBLANK;
                }
            }
            draw_playfield(cpu, tia);
            draw_player0(cpu, tia);
            draw_player1(cpu, tia);
            draw_missile0(cpu, tia);
            draw_missile1(cpu, tia);
            break;

        case TIA_OVERSCAN:
            if (tia->beam_x >= MAX_X) {
                tia->beam_x %= MAX_X;
                tia->beam_y++;

                if (tia->beam_y >= MAX_Y) {
                    tia->beam_y = 0;
                    tia->tia_state = TIA_VSYNC;
                }
            }
            break;
    }
}

