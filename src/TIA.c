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

uint32_t color_rom[8][16] = {
    {0x000000, 0x444400, 0x702800, 0x841800, 0x880000, 0x78005C, 0x480078, 0x140084,
     0x000088, 0x00187C, 0x002C5C, 0x003C2C, 0x003C00, 0x143800, 0x2C3000, 0x442800},
    {0x404040, 0x646410, 0x844414, 0x983418, 0x9C2020, 0x8C2074, 0x602090, 0x302098,
     0x1C209C, 0x1C389C, 0x1C4C78, 0x1C5C48, 0x205C20, 0x345C1C, 0x4C501C, 0x644818},
    {0x6C6C6C, 0x848424, 0x985C28, 0xAC5030, 0xB03C3C, 0xA03C88, 0x783CA4, 0x4C3CAC,
     0x3840B0, 0x3854AB, 0x386890, 0x387C64, 0x407C40, 0x507C38, 0x687034, 0x846830},
    {0x909090, 0xA0A034, 0xAC783C, 0xC06848, 0xC05858, 0xB0589C, 0x8C58B8, 0x6858C0,
     0x505CC0, 0x5070BC, 0x5084AC, 0x509C80, 0x5C9C5C, 0x6C9850, 0x848C4C, 0xA08444},
    {0xB0B0B0, 0xB8B840, 0xBC8C4C, 0xD0805C, 0xD07070, 0xC070B0, 0xA070B0, 0x7C70D0,
     0x6874D0, 0x6888CC, 0x689CC0, 0x68B494, 0x74B474, 0x84B468, 0x9CA864, 0xB89C58},
    {0xC8C8C8, 0xD0D050, 0xCCA05C, 0xE09470, 0xE08888, 0xD084C0, 0xB484DC, 0x9488E0,
     0x7C8CE0, 0x7C9CDC, 0x7CB4D4, 0x7CD0AC, 0x8CD08C, 0x9CCC7C, 0xB4C078, 0xD0B46C},
    {0xDCDCDC, 0xE8E85C, 0xDCB468, 0xECA880, 0xECA0A0, 0xDC9CD0, 0xC49CEC, 0xA8A0EC,
     0x90A4EC, 0x90B4EC, 0x90CCE8, 0x90E4C0, 0xA4E4A4, 0xB4E490, 0xCCD488, 0xE8CC7C},
    {0xECECEC, 0xFCFC68, 0xECC878, 0xFCBC94, 0xFCBC94, 0xFCB4B4, 0xECB0E0, 0xD4B0FC,
     0xBCB4FC, 0xA4B8FC, 0xA4C8FC, 0xA4E0FC, 0xA4FCD4, 0xB8FCB8, 0xC8FCA4, 0xE0EC9C}
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
    memset(tia->pixels, 0, WIDTH * HEIGHT * sizeof(uint32_t));
    tia->beam_x = VSYNC_MIN;
    tia->beam_y = VSYNC_MIN;
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
    /* 20-bit Playfield value formed from PF0, PF1, and PF2 registers */
    uint32_t line = (read8(cpu, PF0) >> 4) | (read8(cpu, PF1) << 4)
        | (read8(cpu, PF2) << 12);
    uint8_t colupf = read8(cpu, COLUPF);
    uint8_t colubk = read8(cpu, COLUBK);
    uint32_t playfield_color = color_lookup((colupf & 0xF) >> 1, colupf >> 4);
    uint32_t background_color = color_lookup((colubk & 0xF) >> 1, colubk >> 4);
    uint16_t y = tia->beam_y - 40;
    int i;

    /* Draw left half of the screen */
    for (i = 0; i < 20; i++) {
        uint32_t color = (line >> i) & 0x1 ? playfield_color : background_color;
        tia->pixels[y * WIDTH + i * 10] = color;
        tia->pixels[y * WIDTH + i * 10 + 1] = color;
        tia->pixels[y * WIDTH + i * 10 + 2] = color;
        tia->pixels[y * WIDTH + i * 10 + 3] = color;
        tia->pixels[y * WIDTH + i * 10 + 3] = color;
        tia->pixels[y * WIDTH + i * 10 + 4] = color;
        tia->pixels[y * WIDTH + i * 10 + 5] = color;
        tia->pixels[y * WIDTH + i * 10 + 6] = color;
        tia->pixels[y * WIDTH + i * 10 + 7] = color;
        tia->pixels[y * WIDTH + i * 10 + 8] = color;
        tia->pixels[y * WIDTH + i * 10 + 9] = color;
    }

    /* Draw right half of the screen */
    if (read8(cpu, CTRLPF) & 0x1) {
        /* Mirror */
        for (i = 0; i < 20; i++) {
            line = (reverse(read8(cpu, PF0)) << 16) | (read8(cpu, PF1) << 8)
                    | reverse(read8(cpu, PF2));
            uint32_t clr = (line >> i) & 0x1 ? playfield_color
                : background_color;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40)] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 1] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 2] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 3] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 4] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 5] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 6] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 7] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 8] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 9] = clr;
        }
    } else {
        /* Normal repeat */
        for (i = 0; i < 20; i++) {
            uint32_t clr = (line >> i) & 0x1 ? playfield_color
                : background_color;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40)] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 1] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 2] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 3] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 4] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 5] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 6] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 7] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 8] = clr;
            tia->pixels[y * WIDTH + (i + WIDTH / 20) * (WIDTH / 40) + 9] = clr;
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
    if (read8(cpu, REFP0) & 0x10) {
        sprite = read8(cpu, GRP0);
    } else {
        sprite = reverse(read8(cpu, GRP0));
    }
    uint8_t colup0 = read8(cpu, COLUP0);
    uint32_t sprite_color = color_lookup((colup0 & 0xF) >> 1, colup0 >> 4);
    uint16_t y = tia->beam_y - 40;

    for (i = 0; i < 8; i++) {
        uint32_t color = (sprite >> i) & 0x1 ? sprite_color
            : tia->pixels[y * WIDTH + tia->beam_x];
        tia->pixels[y * WIDTH + tia->beam_x++] = color;
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
    if (read8(cpu, REFP1) & 0x10) {
        sprite = read8(cpu, GRP1);
    } else {
        sprite = reverse(read8(cpu, GRP1));
    }
    uint8_t colup1 = read8(cpu, COLUP1);
    uint32_t sprite_color = color_lookup((colup1 & 0xF) >> 1, colup1 >> 4);
    uint16_t y = tia->beam_y - 40;

    for (i = 0; i < 8; i++) {
        uint32_t color = (sprite >> i) & 0x1 ? sprite_color
            : tia->pixels[y * WIDTH + tia->beam_x];
        tia->pixels[y * WIDTH + tia->beam_x++] = color;
    }
}

static inline void draw_missile0(CPU* cpu, TIA* tia)
{
    int i;
    uint16_t y = tia->beam_y - 40;
    uint8_t colup0 = read8(cpu, COLUP0);
    uint32_t sprite_color = read8(cpu, COLUP0) & 0x02
        ? color_lookup((colup0 & 0xF) >> 1, colup0 >> 4)
        : tia->pixels[y * WIDTH + tia->beam_x];
    uint8_t size = read8(cpu, NUSIZ0);
    for (i = 0; i < size; i++) {
        tia->pixels[y * WIDTH + tia->beam_x + i] = sprite_color;
    }
}

static inline void draw_missile1(CPU* cpu, TIA* tia)
{
    int i;
    uint16_t y = tia->beam_y - 40;
    uint8_t colup1 = read8(cpu, COLUP1);
    uint32_t sprite_color = read8(cpu, COLUP1) & 0x02
        ? color_lookup((colup1 & 0xF) >> 1, colup1 >> 4)
        : tia->pixels[y * WIDTH + tia->beam_x];
    uint8_t size = read8(cpu, NUSIZ1);
    for (i = 0; i < size; i++) {
        tia->pixels[y * WIDTH + tia->beam_x + i] = sprite_color;
    }
}

/*
 * Perform one operation of the TIA. This means move the beam to its appropriate
 * position after the CPU's current instruction has executed and check to see
 * whether any draw operations need to be performed.
 */
uint8_t tia_step(CPU* cpu, TIA* tia)
{
    uint8_t r = 0;
    tia->beam_x += 3 * cpu->cycles;

    if (read8(cpu, VSYNC) & 0x02) {
        tia->beam_x = VSYNC_MIN;
        tia->beam_y = VSYNC_MIN;
        tia->tia_state = TIA_VSYNC;
    } else if (read8(cpu, VBLANK) & 0x02) {
        tia->beam_x = VSYNC_MIN;
        tia->beam_y = VBLANK_MIN;
        tia->tia_state = TIA_VBLANK;
    } else if (wsync) {
        wsync = 0;
        if (IN_DRAW_Y(tia->beam_y)) {
            draw_playfield(cpu, tia);
            draw_player0(cpu, tia);
            draw_player1(cpu, tia);
            if (read8(cpu, ENAM0) & 0x02) {
                draw_missile0(cpu, tia);
            }
            if (read8(cpu, ENAM1) & 0x02) {
                draw_missile1(cpu, tia);
            }
            r = 1;
        }
        if (tia->beam_y < MAX_Y) {
            tia->beam_y++;
        }
        tia->beam_x = DRAW_X_MIN;
        tia->tia_state = TIA_HBLANK;
    } else if (resp0) {
        resp0 = 0;
        if (IN_DRAW_Y(tia->beam_y)) {
            draw_player0(cpu, tia);
            r = 1;
        }
    } else if (resp1) {
        resp1 = 0;
        if (IN_DRAW_Y(tia->beam_y)) {
            draw_player1(cpu, tia);
            r = 1;
        }
    } else if (resm0) {
        resm0 = 0;
        if (IN_DRAW_Y(tia->beam_y) && (read8(cpu, ENAM0) & 0x02)) {
            draw_missile0(cpu, tia);
            r = 1;
        }
    } else if (resm1) {
        resm1 = 0;
        if (IN_DRAW_Y(tia->beam_y) && (read8(cpu, ENAM1) & 0x02)) {
            draw_missile1(cpu, tia);
            r = 1;
        }
    } else {
        if (IN_VSYNC(tia->beam_y)) {
            tia->tia_state = TIA_VSYNC;
        } else if (IN_VBLANK(tia->beam_y)) {
            tia->tia_state = TIA_VBLANK;
        } else if (IN_DRAW_Y(tia->beam_y)) {
            if (IN_HBLANK(tia->beam_x)) {
                tia->tia_state = TIA_HBLANK;
            } else if (IN_DRAW_X(tia->beam_x)) {
                tia->tia_state = TIA_DRAW;
                draw_playfield(cpu, tia);
                draw_player0(cpu, tia);
                draw_player1(cpu, tia);
                if (IN_DRAW_Y(tia->beam_y) && (read8(cpu, ENAM0) & 0x02)) {
                    draw_missile0(cpu, tia);
                }
                if (IN_DRAW_Y(tia->beam_y) && (read8(cpu, ENAM1) & 0x02)) {
                    draw_missile1(cpu, tia);
                }
                r = 1;
            }
        } else {
            tia->tia_state = TIA_OVERSCAN;
        }
        if (tia->beam_x >= MAX_X) {
            tia->beam_x %= MAX_X;
            if (tia->beam_y < MAX_Y) {
                tia->beam_y++;
            } else {
                tia->beam_y %= MAX_Y;
            }
        }
    }
    return r;
}

