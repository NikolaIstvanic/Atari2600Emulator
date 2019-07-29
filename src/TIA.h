#ifndef TIA_h
#define TIA_h

/* INCLUDE */
#include "types.h"

/* DEFINES */
#define MAX_X 228
#define MAX_Y 262
#define VBLANK_MIN 3
#define VBLANK_MAX 40
#define HBLANK_MAX 68
#define DRAW_MAX 232

// TODO: refactor

/* ENUM */
enum { TIA_VSYNC, TIA_VBLANK, TIA_HBLANK, TIA_DRAW, TIA_OVERSCAN };

/* COLOR ROM */
extern uint32_t color_rom[8][16];

/* METHOD PROTOTYPES */
void tia_init(TIA* tia);
void tia_step(CPU* cpu, TIA* tia);

#endif

