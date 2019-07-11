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

Display* dpy;
Window win;
GLXContext glc;
struct timespec frameStart;

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

static void update_pressed_keys()
{
    XEvent xev;

    if (XPending(dpy)) {
        XNextEvent(dpy, &xev);
        if (xev.xkey.type == KeyPress) {
            switch (xev.xkey.keycode) {
                /*
                // z = B
                case 52:
                    buttons = CLEARBIT(buttons, BUTTON_B);
                    break;

                // Left = left
                case 113:
                    dpad = CLEARBIT(dpad, BUTTON_LEFT);
                    break;

                // Right = right
                case 114:
                    dpad = CLEARBIT(dpad, BUTTON_RIGHT);
                    break;

                // Up = up
                case 111:
                    dpad = CLEARBIT(dpad, BUTTON_UP);
                    break;

                // Down = down
                case 116:
                    dpad = CLEARBIT(dpad, BUTTON_DOWN);
                    break;
                */

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
}

static void refresh_screen(TIA* tia)
{
    struct timespec frameEnd;
    long seconds, useconds;
    uint32_t mtime;

    glClear(GL_COLOR_BUFFER_BIT);
    GLint iViewport[4];
    glGetIntegerv(GL_VIEWPORT, iViewport);
    glRasterPos2f(-1, 1);
    glPixelZoom(iViewport[2] / WIDTH, -iViewport[3] / HEIGHT);
    glDrawPixels(WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, tia->pixels);
    glXSwapBuffers(dpy, win);

    clock_gettime(CLOCK_MONOTONIC, &frameEnd);
    seconds = frameEnd.tv_sec - frameStart.tv_sec;
    useconds = frameEnd.tv_nsec - frameStart.tv_nsec;
    mtime = seconds * 1000000 + useconds;

    if (0 < mtime && mtime < 16666) {
        usleep(16666 - mtime);
    }
    clock_gettime(CLOCK_MONOTONIC, &frameStart);
}

/*
 * Once the machine source code has been loaded into the emulator's CPU's RAM,
 * let the CPU execute the instructions it contains until the user wants the
 * program to end.
 */
static void run(CPU* cpu, TIA* tia)
{
    static unsigned int x = 0;

    clock_gettime(CLOCK_MONOTONIC, &frameStart);
    
    while (1) {
        /* Execute one instruction */
        cpu_step(cpu);

        /* Perform graphics operation */
        tia_step(cpu, tia);

        if (tia->beam_y == DRAW_MAX) {
            update_pressed_keys(cpu);
            refresh_screen(tia);
        }
        
        if (++x == 1160) {
            //exit(1);
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
    Window root;
    GLint att[] = { GLX_RGBA, GLX_DOUBLEBUFFER };
    XVisualInfo* vi;
    Colormap cmap;
    XSetWindowAttributes swa;
    
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

    cpu_init(&cpu);
    tia_init(&tia);

    load_source(&cpu, argv[1]);
    run(&cpu, &tia);
    return EXIT_SUCCESS;
}

