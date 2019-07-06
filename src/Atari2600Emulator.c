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
 *           _________________________________________________________
 *          |\                                                       /|
 *          | \_____________________________________________________/ |
 *          |  ;        .-.   .-.   .--------.   .-.   .-.         ;  |
 *          |  ;        |o|   |o|   |  ATARI |:  |o|   |o|         ;  |
 *          | ;         `-'   `-'   `--------:;  `-'   `-'          ; |
 *          |;                       `````````                       ;|
 *          |`````````````````````````````````````````````````````````|
 *          |=========================================================|
 *          |=========================================================|
 *          |=========================================================|
 *          |=========================================================|
 *          |=========================================================|
 *          |=========================================================|
 *          |=========================================================|
 *          |=========================================================|
 *          |=========================================================|
 *          |#########################################################|
 *           \_______________________________________________________/
 *
 *                              ATARI 2600 EMULATOR
 *                                Nikola Istvanic
 *
 * This program is meant to emulate the hardware and software of an Atari 2600,
 * a popular home video game console during the late 1970s and early 1980s (see
 * https://en.wikipedia.org/wiki/Atari_2600 for more).
 *
 * By creating an environment that programmatically simulates how the hardware
 * of the 2600 behaves, an emulator can simply operate by reading in machine
 * code of a ROM and execute normally, recreating how a real 2600 would perform
 * these instructions. This is the aim of this program: seamlessly recreate the
 * environment of the 2600 and have it execute any ROM of machine code
 * instructions.
 *
 * This emulator adheres to the following structure: the Atari2600Emulator file
 * initializes an emulated CPU for the processor used in the Atari 2600: the
 * MOS 6507 microprocessor (https://en.wikipedia.org/wiki/MOS_Technology_6507).
 * After the CPU is initialized with the appropriate starting values for its
 * fields, this file takes in as input the name of the ROM which is saved
 * locally as input. The contents of that file (if present) are then read into
 * the emulated CPU's RAM. Finally, the CPU starts running the instructions
 * found in that file until either the program it's running ends or the user
 * wishes to end the emulator (hits the ESC key).
 */
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "CPU.h"
#include "MMU.h"
#include "TIA.h"

/*
 * Input keys for the Atari 2600 emulator. The standard Atari 2600 controller is
 * emulated here with an array with 5 entries. One entry for UP, DOWN, LEFT,
 * RIGHT, and the red button.
 */
uint8_t keys[5];

/*
 * Take in the name of the ROM file which contains the game which will be ran on
 * this emulator. If this file is present, this method will read its contents
 * into the RAM of the CPU pointer which is given as a parameter.
 */
static void load_source(CPU* cpu, const char* rom_path)
{
    FILE* rom = fopen(rom_path, "rb");

    if (!rom) {
        printf("ERROR: ROM %s not found\n", rom_path);
        fflush(stdin);
        exit(EXIT_FAILURE);
    }
    /* Read ROM into CPU RAM */
    if (!fread(cpu->RAM + ROM_OFFSET, 1, SIZE_CART, rom)) {
        printf("ERROR: reading ROM %s failed\n", rom_path);
        exit(EXIT_FAILURE);
    }
}

static void update_pressed_keys(CPU* cpu)
{
    uint8_t* pressed = SDL_GetKeyState(NULL);

    if (pressed[SDLK_ESCAPE]) {
        exit(EXIT_SUCCESS);
    }

    /* Handle hitting the reset button */
    if (pressed[SDLK_r]) {
        cpu->PC = read16(cpu, RESET_VECTOR);
    }

    /*
    for (int i = 0; i < 5; i++) {
        // TOOD: handle user input
    }
    */
}

/*
 * Method which updates the SDL screen pixels based on the contents of the TIA
 * pixel array only whenever pixels of the screen have been changed by the CPU.
 */
static void refresh_screen(TIA* tia)
{
    SDL_Surface* emulator_screen = SDL_GetVideoSurface();

    /* Secure surface to access pixels */
    SDL_LockSurface(emulator_screen);
    memcpy(emulator_screen->pixels, tia->pixels, sizeof(uint32_t) * EMU_WIDTH * EMU_HEIGHT);

    /* Release surface */
    SDL_UnlockSurface(emulator_screen);
    SDL_Flip(emulator_screen);
}

/*
 * Once the machine source code has been loaded into the emulator's CPU's RAM,
 * let the CPU execute the instructions it contains until the user wants the
 * program to end.
 */
static void run(CPU* cpu, TIA* tia)
{
    SDL_Event e;

    /* Initialize emulator screen */
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetVideoMode(EMU_WIDTH, EMU_HEIGHT, BPP, SDL_HWSURFACE | SDL_DOUBLEBUF);
    static unsigned int x = 0;

    while (1) {
        if (SDL_PollEvent(&e)) {
            continue;
        }

        update_pressed_keys(cpu);

        /* Execute one instruction */
        cpu_step(cpu);

        /* Perform graphics operation */
        tia_step(cpu, tia);

        /* TODO: replace this with 60 FPS and refresh rate logic */
        if (++x % 100 == 0) {
            refresh_screen(tia);
        }
    }
}

/*
 * Main method of the Atari 2600 Emulator. Here, take in the user input from the
 * terminal to be used as the name of the file which he/she would like to read
 * and be emulated.
 *
 * Assuming that the file is present and is an Atari 2600 source ROM, this
 * program will read the contents of the file into its CPU and then begin to
 * execute the source code. To end this, the ESCape key can be pressed which
 * will end the program and return control to the operating system.
 */
int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("ERROR: must have ROM path\n");
        return EXIT_FAILURE;
    }

    CPU cpu;
    TIA tia;

    cpu_init(&cpu);
    tia_init(&tia);
    load_source(&cpu, argv[1]);
    //load_source(&cpu, "missiles.rom");
    run(&cpu, &tia);
    return EXIT_SUCCESS;
}

