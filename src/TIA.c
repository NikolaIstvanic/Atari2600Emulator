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
    memset(tia->pixels, 0, WIDTH * HEIGHT);
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
static inline uint32_t color_lookup(uint8_t value)
{
    return color_rom[value];
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
    uint32_t playfield_color = color_lookup(read8(cpu, COLUPF) >> 4);
    uint32_t background_color = color_lookup(read8(cpu, COLUBK) >> 4);
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
    uint32_t sprite_color = color_lookup(read8(cpu, COLUP0) >> 4);
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
    uint32_t sprite_color = color_lookup(read8(cpu, COLUP1) >> 4);
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
    uint32_t sprite_color = read8(cpu, COLUP0) & 0x02
        ? color_lookup(read8(cpu, COLUP0) >> 4)
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
    uint32_t sprite_color = read8(cpu, COLUP1) & 0x02
        ? color_lookup(read8(cpu, COLUP1) >> 4)
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

uint32_t color_rom[0x10] = {
    0x7F7F7F, /* Grey */
    0xFFD700, /* Gold */
    0xFF8500, /* Orange */
    0xFF5200, /* Red-orange */
    0xFF528C, /* Pink */
    0xFF00FF, /* Purple */
    0xB500FF, /* Purple-Blue */
    0x000096, /* Dark Blue */
    0x3232FF, /* Blue */
    0x6464FF, /* Light Blue */
    0x00C8C8, /* Turquoise */
    0x00C896, /* Green-Blue */
    0x00C855, /* Green */
    0x64C800, /* Yellow-Green */
    0xAFC800, /* Orange-Green */
    0xFFB464  /* Light-Orange */
};

