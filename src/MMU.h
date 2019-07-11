#ifndef MMU_h
#define MMU_h

/* INCLUDE */
#include "types.h"

/* DEFINES */
#define VSYNC  0x00
#define VBLANK 0x01
#define WSYNC  0x02
#define NUSIZ0 0x04
#define NUSIZ1 0x05
#define COLUP0 0x06
#define COLUP1 0x07
#define COLUPF 0x08
#define COLUBK 0x09
#define CTRLPF 0x0A
#define REFP0  0x0B
#define REFP1  0x0C
#define PF0    0x0D
#define PF1    0x0E
#define PF2    0x0F
#define RESP0  0x10
#define RESP1  0x11
#define RESM0  0x12
#define RESM1  0x13
#define RESBL  0x14
#define GRP0   0x1B
#define GRP1   0x1C
#define ENAM0  0x1D
#define ENAM1  0x1E
#define ENABL  0x1F
#define HMP0   0x20
#define HMP1   0x21
#define HMM0   0x22
#define HMM1   0x23
#define HMBL   0x24
#define HMOVE  0x2A
#define HMCLR  0x2B

/* STROBE BOOLEANS */
extern uint8_t wsync;
extern uint8_t resp0;
extern uint8_t resp1;
extern uint8_t resm0;
extern uint8_t resm1;
extern uint8_t resbl;
extern uint8_t hmove;
extern uint8_t hmclr;

/* INTERRUPT VECTORS */
extern const uint16_t NMI_VECTOR;
extern const uint16_t IRQ_VECTOR;
extern const uint16_t RESET_VECTOR;

/* MEMORY METHOD PROTOTYPES */
void mem_init(CPU* cpu);
uint8_t read8(CPU* cpu, uint16_t address);
uint16_t read16(CPU* cpu, uint16_t address);
void write8(CPU* cpu, uint16_t address, uint8_t value);
void write16(CPU* cpu, uint16_t address, uint16_t value);

/* STACK METHOD PROTOTYPES */
void push8(CPU* cpu, uint8_t data);
void push16(CPU* cpu, uint16_t data);
uint8_t pop8(CPU* cpu);
uint16_t pop16(CPU* cpu);

#endif

