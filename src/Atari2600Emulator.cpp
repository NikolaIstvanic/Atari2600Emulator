/**
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
 *
 * This program is a continuation of a final project from a college final
 * project. Specification and hardware details for emulation can be found at
 * http://problemkaputt.de/2k6specs.htm.
 *
 * All ROMs in the ../bin directory are from https://8bitworkshop.com/
 */
#include <fstream>
#include <iostream>
#include <string>

#include "Atari.hpp"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define SIZE_CART 4096
#define CART_OFFSET 0xF000

class Atari2600Emulator : public olc::PixelGameEngine {
public:
    Atari2600Emulator(const std::string& romPath) : m_romPath{romPath} {
        sAppName = "Atari 2600 Emulator";
    }

private:
    bool OnUserCreate() override {
        std::ifstream ifs;
        ifs.open(m_romPath, std::ifstream::binary);

        if (ifs.is_open()) {
            ifs.read((char*) m_atari.m_ram.data() + CART_OFFSET, SIZE_CART);
            ifs.close();
            m_atari.reset();
            return true;
        }

        std::cout << "Error reading file" << std::endl;
        return false;
    }

    bool OnUserUpdate(float elapsedTime) override {
        uint8_t joysticks = 0x00;
        uint8_t switches = 0x00;
        uint8_t button1 = 0x00;
        uint8_t button2 = 0x00;

        joysticks |= GetKey(olc::Key::W).bHeld ? 0x00 : 0x10;
        joysticks |= GetKey(olc::Key::S).bHeld ? 0x00 : 0x20;
        joysticks |= GetKey(olc::Key::A).bHeld ? 0x00 : 0x40;
        joysticks |= GetKey(olc::Key::D).bHeld ? 0x00 : 0x80;
        joysticks |= GetKey(olc::Key::I).bHeld ? 0x00 : 0x01;
        joysticks |= GetKey(olc::Key::K).bHeld ? 0x00 : 0x02;
        joysticks |= GetKey(olc::Key::J).bHeld ? 0x00 : 0x04;
        joysticks |= GetKey(olc::Key::L).bHeld ? 0x00 : 0x08;

        switches |= GetKey(olc::Key::F1).bHeld ? 0x00 : 0x01;
        switches |= GetKey(olc::Key::F2).bHeld ? 0x00 : 0x02;
        switches |= GetKey(olc::Key::F3).bHeld ? 0x00 : 0x08;
        switches |= GetKey(olc::Key::F4).bHeld ? 0x00 : 0x40;
        switches |= GetKey(olc::Key::F5).bHeld ? 0x00 : 0x80;

        button1 |= GetKey(olc::Key::C).bHeld ? 0x00 : 0x80;
        button2 |= GetKey(olc::Key::M).bHeld ? 0x00 : 0x80;

        m_atari.write8(SWCHA, joysticks);
        m_atari.write8(SWCHB, switches);
        m_atari.write8(INPT4, button1);
        m_atari.write8(INPT5, button2);

#if 0
        // Debugging commands
        if (GetKey(olc::Key::SPACE).bPressed) {
            for (int i = 0; i < 20; i++) {
                do { atari.step(); } while (atari.cpu.cycles != 0);
                do { atari.step(); } while (atari.cpu.cycles == 0);
            }
        }
        if (GetKey(olc::Key::S).bPressed) {
            do { atari.step(); } while (atari.cpu.cycles != 0);
            do { atari.step(); } while (atari.cpu.cycles == 0);
        }
        if (GetKey(olc::Key::F).bPressed) {
            do { atari.step(); } while (!atari.tia.frameDone);
            atari.tia.frameDone = false;
        }
#endif

        if (GetKey(olc::Key::R).bPressed) {
            m_atari.reset();
        }
        if (GetKey(olc::Key::ESCAPE).bPressed) {
            return false;
        }

        if (m_residualTime > 0.0) {
            m_residualTime -= elapsedTime;
        } else {
            m_residualTime += (1.0f / 60.0f) - elapsedTime;

            do {
                m_atari.step();
            } while (!m_atari.m_tia.m_frameDone);

            m_atari.m_tia.m_frameDone = false;
        }

        DrawSprite(0, 0, &m_atari.m_tia.getScreen(), 1);

        return true;
    }

    Atari m_atari;
    float m_residualTime = 0.0f;
    std::string m_romPath;
};

int main(int argc, char* argv[]) {
    if (argc > 1) {
        Atari2600Emulator emu{argv[1]};
        if (emu.Construct(WIDTH, HEIGHT, 4, 2)) {
            emu.Start();
        }
    } else {
        std::cout << "Provide path to ROM" << std::endl;
    }

    return 0;
}

