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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "CPU.h"
#include "MMU.h"
#include "TIA.h"

#define WINDOW_WIDTH (WIDTH * 4)
#define WINDOW_HEIGHT (HEIGHT * 2)

#define BUTTON_UP0 4
#define BUTTON_DOWN0 5
#define BUTTON_LEFT0 6
#define BUTTON_RIGHT0 7
#define BUTTON_UP1 0
#define BUTTON_DOWN1 1
#define BUTTON_LEFT 2
#define BUTTON_RIGHT1 3

Display* dpy;
Window win;
GLXContext glc;

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
    XEvent xev;
    uint8_t dir = 0xFF;
    uint8_t inpt4 = 0xFF;
    uint8_t inpt5 = 0xFF;

    if (XPending(dpy)) {
        XNextEvent(dpy, &xev);
        if (xev.xkey.type == KeyPress) {
            switch (xev.xkey.keycode) {
                // z = B
                case 52:
                    break;

                // Up = up
                case 111:
                    dir = CLEARBIT(dir, BUTTON_UP0);
                    break;

                // Down = down
                case 116:
                    dir = CLEARBIT(dir, BUTTON_DOWN0);
                    break;

                // Left = left
                case 113:
                    dir = CLEARBIT(dir, BUTTON_LEFT0);
                    break;

                // Right = right
                case 114:
                    dir = CLEARBIT(dir, BUTTON_RIGHT0);
                    break;

                // ESC = quit
                case 9:
                    glXMakeCurrent(dpy, None, NULL);
                    glXDestroyContext(dpy, glc);
                    XDestroyWindow(dpy, win);
                    XCloseDisplay(dpy);
                    exit(EXIT_SUCCESS);
                    break;
            }
        }
    }
    write8(cpu, SWCHA, dir);
    write8(cpu, INPT4, inpt4);
    write8(cpu, INPT5, inpt5);
}

static void* refresh_screen(void* emu)
{
    struct timespec start, end;
    long seconds, useconds;
    uint32_t utime;
    Window root;
    XVisualInfo* vi;
    Colormap cmap;
    XSetWindowAttributes swa;
    GLint iViewport[4];
    GLint att[] = { GLX_RGBA, GLX_DOUBLEBUFFER };

    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        printf("Cannot connect to X server\n");
        exit(EXIT_FAILURE);
    }
    root = DefaultRootWindow(dpy);
    vi = glXChooseVisual(dpy, 0, att);
    if (!vi) {
        printf("No appropriate visual found\n");
        exit(EXIT_FAILURE);
    }
    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;
    win = XCreateWindow(dpy, root, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
        vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);

    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Atari 2600 Emulator");
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);

    clock_gettime(CLOCK_MONOTONIC, &start);

    while (1) {
        // TODO: refactor time conversions
        clock_gettime(CLOCK_MONOTONIC, &end);
        seconds = end.tv_sec - start.tv_sec;
        useconds = (end.tv_nsec - start.tv_nsec) / 1000;
        utime = seconds * 1000000 + useconds;

        if (0 < utime && utime < 16666) {
            usleep(16666 - utime);
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glGetIntegerv(GL_VIEWPORT, iViewport);
        glRasterPos2f(-1, 1);
        glPixelZoom(iViewport[2] / WIDTH, -iViewport[3] / HEIGHT);
        glDrawPixels(WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
            (((struct EMU*) emu)->tia)->pixels);
        glXSwapBuffers(dpy, win);

        update_pressed_keys(((struct EMU*) emu)->cpu);

        clock_gettime(CLOCK_MONOTONIC, &start);
    }
    return NULL;
}

/*
 * Once the machine source code has been loaded into the emulator's CPU's RAM,
 * let the CPU execute the instructions it contains until the user wants the
 * program to end.
 */
static void run(CPU* cpu, TIA* tia)
{
    pthread_t thread_id;
    uint32_t instrs = 0;
    uint32_t BATCH_SIZE = 1000;
    struct EMU e;
    e.cpu = cpu;
    e.tia = tia;

    pthread_create(&thread_id, NULL, refresh_screen, &e);

    while (1) {
        /* Execute one instruction */
        cpu_step(cpu);

        /* Perform graphics operation */
        tia_step(cpu, tia);

        /* Execute BATCH_SIZE instructions and sleep for the proportional amount of time */
        if (++instrs == BATCH_SIZE) {
            // TODO: figure out why this works and/or find a different way
            double sleep_time = 10000 - (((double) CPU_SPEED - BATCH_SIZE) / CPU_SPEED) * MICRO_IN_SEC;
            instrs = 0;
            if (0 <= sleep_time && sleep_time <= 10000) {
                usleep(sleep_time);
            }
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
    run(&cpu, &tia);
    return EXIT_SUCCESS;
}

