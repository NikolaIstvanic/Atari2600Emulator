#ifndef VALUES_h
#define VALUES_h

/* DEFINES */
#define SIZE_CART 4096 // maximum size of an Atari 2600 ROM in bytes
#define ROM_OFFSET 0xF000 // offset into CPU RAM where ROM is loaded
#define SIZE_MEM 0x10000 // size (in bytes) of the RAM of the Atari 2600

#define WIDTH 160 // width of TIA image
#define HEIGHT 192 // height of TIA image

#define CPU_SPEED 1193182 // CPU speed (1.19 MHz)

#define MICRO_IN_SEC 1000

#define CLEARBIT(_val, _bit) ((_val) & ~(1 << (_bit))) // Clear _bit-th bit in _val

#endif

