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
 * MMU.c: file which handles all reads and writes to the Atari 2600's RAM.
 *
 * Atari 2600 Memory Map:
 *
 * 0xFFFF -----------> +------------------------------+   --+
 *                     |                              |     |
 *                     |                              |     |
 *                     |       Cartridge Memory       |4 Kilobytes
 *                     |                              |     |
 *                     |                              |     |
 * 0xF000 ---------->  +------------------------------+   --+
 *                     |                              |
 *                     :              .               :
 *                     :              .               :
 *                     |                              |
 * 0x0297 ---------->  +------------------------------+
 *                     |     PIA Ports and Timer      |
 * 0x0280 ---------->  +------------------------------+
 *                     |                              |
 *                     :              .               :
 *                     :              .               :
 *                     |                              |
 * 0x00FF ---------->  +------------------------------+   --+
 *                     |                              |     |
 *                     |                              |     |
 *                     |           PIA RAM            | 128 Bytes
 *                     |                              |     |
 *                     |                              |     |
 * 0x0080 ---------->  +------------------------------+   --+
 *                     |                              |
 *                     :              .               :
 *                     :              .               :
 *                     |                              |
 * 0x003D ---------->  +------------------------------+
 *                     |           TIA Read           |
 * 0x002C ---------->  +------------------------------+
 *                     |           TIA Write          |
 * 0x0000 ---------->  +------------------------------+
 *
 * TIA - WRITE ADDRESS SUMMARY (Write only)
 * 00      VSYNC   ......1.  vertical sync set-clear
 * 01      VBLANK  11....1.  vertical blank set-clear
 * 02      WSYNC   <strobe>  wait for leading edge of horizontal blank
 * 03      RSYNC   <strobe>  reset horizontal sync counter
 * 04      NUSIZ0  ..111111  number-size player-missile 0
 * 05      NUSIZ1  ..111111  number-size player-missile 1
 * 06      COLUP0  1111111.  color-lum player 0 and missile 0
 * 07      COLUP1  1111111.  color-lum player 1 and missile 1
 * 08      COLUPF  1111111.  color-lum playfield and ball
 * 09      COLUBK  1111111.  color-lum background
 * 0A      CTRLPF  ..11.111  control playfield ball size & collisions
 * 0B      REFP0   ....1...  reflect player 0
 * 0C      REFP1   ....1...  reflect player 1
 * 0D      PF0     1111....  playfield register byte 0
 * 0E      PF1     11111111  playfield register byte 1
 * 0F      PF2     11111111  playfield register byte 2
 * 10      RESP0   <strobe>  reset player 0
 * 11      RESP1   <strobe>  reset player 1
 * 12      RESM0   <strobe>  reset missile 0
 * 13      RESM1   <strobe>  reset missile 1
 * 14      RESBL   <strobe>  reset ball
 * 15      AUDC0   ....1111  audio control 0
 * 16      AUDC1   ....1111  audio control 1
 * 17      AUDF0   ...11111  audio frequency 0
 * 18      AUDF1   ...11111  audio frequency 1
 * 19      AUDV0   ....1111  audio volume 0
 * 1A      AUDV1   ....1111  audio volume 1
 * 1B      GRP0    11111111  graphics player 0
 * 1C      GRP1    11111111  graphics player 1
 * 1D      ENAM0   ......1.  graphics (enable) missile 0
 * 1E      ENAM1   ......1.  graphics (enable) missile 1
 * 1F      ENABL   ......1.  graphics (enable) ball
 * 20      HMP0    1111....  horizontal motion player 0
 * 21      HMP1    1111....  horizontal motion player 1
 * 22      HMM0    1111....  horizontal motion missile 0
 * 23      HMM1    1111....  horizontal motion missile 1
 * 24      HMBL    1111....  horizontal motion ball
 * 25      VDELP0  .......1  vertical delay player 0
 * 26      VDELP1  .......1  vertical delay player 1
 * 27      VDELBL  .......1  vertical delay ball
 * 28      RESMP0  ......1.  reset missile 0 to player 0
 * 29      RESMP1  ......1.  reset missile 1 to player 1
 * 2A      HMOVE   <strobe>  apply horizontal motion
 * 2B      HMCLR   <strobe>  clear horizontal motion registers
 * 2C      CXCLR   <strobe>  clear collision latches
 *
 * TIA - READ ADDRESS SUMMARY (Read only)
 * 30      CXM0P   11......  read collision M0-P1, M0-P0 (Bit 7,6)
 * 31      CXM1P   11......  read collision M1-P0, M1-P1
 * 32      CXP0FB  11......  read collision P0-PF, P0-BL
 * 33      CXP1FB  11......  read collision P1-PF, P1-BL
 * 34      CXM0FB  11......  read collision M0-PF, M0-BL
 * 35      CXM1FB  11......  read collision M1-PF, M1-BL
 * 36      CXBLPF  1.......  read collision BL-PF, unused
 * 37      CXPPMM  11......  read collision P0-P1, M0-M1
 * 38      INPT0   1.......  read pot port
 * 39      INPT1   1.......  read pot port
 * 3A      INPT2   1.......  read pot port
 * 3B      INPT3   1.......  read pot port
 * 3C      INPT4   1.......  read input
 * 3D      INPT5   1.......  read input
 *
 * PIA 6532 - RAM, Switches, and Timer (Read/Write)
 * 80..FF  RAM     11111111  128 bytes RAM (in PIA chip) for variables and stack
 * 0280    SWCHA   11111111  Port A; input or output  (read or write)
 * 0281    SWACNT  11111111  Port A DDR, 0= input, 1=output
 * 0282    SWCHB   11111111  Port B; console switches (read only)
 * 0283    SWBCNT  11111111  Port B DDR (hardwired as input)
 * 0284    INTIM   11111111  Timer output (read only)
 * 0285    INSTAT  11......  Timer Status (read only, undocumented)
 * 0294    TIM1T   11111111  set 1 clock interval (838 nsec/interval)
 * 0295    TIM8T   11111111  set 8 clock interval (6.7 usec/interval)
 * 0296    TIM64T  11111111  set 64 clock interval (53.6 usec/interval)
 * 0297    T1024T  11111111  set 1024 clock interval (858.2 usec/interval)
 */
#include <string.h>

#include "MMU.h"

const uint16_t NMI_VECTOR = 0xFFFA;
const uint16_t IRQ_VECTOR = 0xFFFE;
const uint16_t RESET_VECTOR = 0xFFFC;

uint8_t wsync = 0;
uint8_t resp0 = 0;
uint8_t resp1 = 0;
uint8_t resm0 = 0;
uint8_t resm1 = 0;
uint8_t resbl = 0;
uint8_t hmove = 0;

/*
 * Given a CPU pointer, initialize is memory by first zeroing it out, then
 * setting up the interrupt vectors, and setting up the interrupt handlers.
 */
void mem_init(CPU* cpu)
{
    /* Initialize RAM */
     memset(cpu->RAM, 0, SIZE_MEM);
}

/*******************************************************************************
 *                              MEMORY METHODS                                 *
 ******************************************************************************/
/*
 * Read the byte located at the address parameter in the CPU parameter's RAM.
 */
uint8_t read8(CPU* cpu, uint16_t address)
{
    return cpu->RAM[address];
}

/*
 * Read the two bytes located at the address supplied in the CPU's RAM, noting
 * that data in RAM is stored in little-endian format.
 */
uint16_t read16(CPU* cpu, uint16_t address)
{
    return (cpu->RAM[address + 1] << 8) | cpu->RAM[address];
}

/*
 * Write the byte given into the CPU's RAM at the given address.
 */
void write8(CPU* cpu, uint16_t address, uint8_t data)
{
    cpu->RAM[address] = data;
    if (address == WSYNC) {
        wsync = 1;
    } else if (address == RESP0) {
        resp0 = 1;
    } else if (address == RESP1) {
        resp1 = 1;
    } else if (address == RESM0) {
        resm0 = 1;
    } else if (address == RESM1) {
        resm1 = 1;
    } else if (address == RESBL) {
        resbl = 1;
    } else if (address == HMOVE) {
        hmove = 1;
    }
}

/*
 * Write the 2 bytes given into the CPU's RAM at the given address in little-
 * endian.
 */
void write16(CPU* cpu, uint16_t address, uint16_t data)
{
    cpu->RAM[address] = data & 0x00FF;
    cpu->RAM[address + 1] = data >> 8;
}

/*******************************************************************************
 *                               STACK METHODS                                 *
 ******************************************************************************/
/*
 * Push onto the stack an unsigned 8-bit integer.
 */
void push8(CPU* cpu, uint8_t data)
{
    write8(cpu, cpu->S-- | 0x100, data);
}

/*
 * Push onto the stack an unsigned 16-bit integer in little endian format.
 */
void push16(CPU* cpu, uint16_t data)
{
    write16(cpu, --cpu->S | 0x100, data);
    cpu->S--;
}

/*
 * Pop off from the stack the entry below the first free location. This entry
 * will be of type unsigned 8-bit integer.
 */
uint8_t pop8(CPU* cpu)
{
    return read8(cpu, ++cpu->S | 0x100);
}

/*
 * Pop off from the stack the entry below the first free location keeping the
 * little endianness of memory in mind. This entry will be of type unsigned
 * 16-bit integer.
 */
uint16_t pop16(CPU* cpu)
{
    uint16_t value = read16(cpu, ++cpu->S | 0x100);
    cpu->S++;
    return value;
}
