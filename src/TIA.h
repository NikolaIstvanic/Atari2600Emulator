#ifndef TIA_h
#define TIA_h

/* INCLUDE */
#include "types.h"

/* DEFINES */
#define MAX_X 228
#define MAX_Y 262
#define VSYNC_MAX 2
#define VBLANK_MIN 3
#define VBLANK_MAX 39
#define HBLANK_MAX 67
#define DRAW_X_MIN 68
#define DRAW_Y_MIN 40
#define DRAW_Y_MAX 231

/* ENUM */
enum { TIA_VSYNC, TIA_VBLANK, TIA_HBLANK, TIA_DRAW, TIA_OVERSCAN };

/* COLOR ROM */
extern uint32_t color_rom[8][16];

/* METHOD PROTOTYPES */
void tia_init(TIA* tia);
void tia_step(CPU* cpu, TIA* tia);

#endif

