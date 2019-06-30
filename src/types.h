#ifndef types_h
#define types_h

/* INCLUDE */
#include "values.h"

/* TYPEDEFS */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;

/*
 * Struct which contains data to represent a MOS 6502 processor which is used by
 * the Atari 2600. Contains general purpose registers as well as utility
 * registers used by the emulator.
 */
typedef struct cpu {
    uint8_t A;   // accumulator register
    uint8_t X;   // general purpose X 8-bit register
    uint8_t Y;   // general purpose Y 8-bit register
    uint16_t PC; // 16-bit Program Counter
    uint8_t S;   // 8-bit Stack Pointer
    uint8_t P;   // 8-bit flag register

    uint8_t RAM[SIZE_MEM]; // 65536 bytes of RAM

    uint8_t cycles; // the number of machine cycles the current instruction took
    uint8_t opcode; // the opcode of the current instruction being executed
} CPU;

/*
 * Struct which contains data to represent the Television Interface Card (TIA)
 * used on the Atari 2600. This piece of hardware was used to display the
 * graphics of the game onto a television screen as well as output sound. This
 * struct contains a two-dimensional array which represent the pixels of the
 * screen as they are changed on each VSYNC as well as a status which determines
 * which state the TIA is in. If the state is VSYNC, VBLANK, or HBLANK, then no
 * pixels have been changed on the screen.
 */
typedef struct tia {
    uint32_t pixels[WIDTH * HEIGHT]; // pixels of the screen
    uint8_t beam_x;                  // x position of the beam
    uint16_t beam_y;                 // y position of the beam
    unsigned tia_state;              // current state of the TIA
} TIA;

#endif

