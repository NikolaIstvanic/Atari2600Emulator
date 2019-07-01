#ifndef TIA_h
#define TIA_h

/* INCLUDE */
#include "types.h"

/* DEFINES */
#define MAX_X 228
#define MAX_Y 262

#define VSYNC_MIN 0
#define VSYNC_MAX 2

#define VBLANK_MIN  3
#define VBLANK_MAX 39

#define HBLANK_MIN  0
#define HBLANK_MAX 67

#define DRAW_X_MIN  68
#define DRAW_X_MAX 227
#define DRAW_Y_MIN  40
#define DRAW_Y_MAX 231

/* MACROS */
#define IN_VSYNC(y)  (VSYNC_MIN <= (y) && (y) <= VSYNC_MAX)
#define IN_VBLANK(y) (VBLANK_MIN <= (y) && (y) <= VBLANK_MAX)
#define IN_HBLANK(x) (HBLANK_MIN <= (x) && (x) <= HBLANK_MAX)
#define IN_DRAW_X(x) (DRAW_X_MIN <= (x) && (x) <= DRAW_X_MAX)
#define IN_DRAW_Y(y) (DRAW_Y_MIN <= (y) && (y) <= DRAW_Y_MAX)

/* ENUM */
enum { TIA_VSYNC, TIA_VBLANK, TIA_HBLANK, TIA_DRAW, TIA_OVERSCAN };

/* COLOR ROM */
extern uint32_t color_rom[0x10];

/* METHOD PROTOTYPES */
void tia_init(TIA* tia);
uint8_t tia_step(CPU* cpu, TIA* tia);

#endif

