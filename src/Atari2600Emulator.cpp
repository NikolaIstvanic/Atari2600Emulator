#include <fstream>
#include <iostream>

#include "Atari.hpp"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define SIZE_CART 4096
#define CART_OFFSET 0xF000

class Atari2600Emulator : public olc::PixelGameEngine {
    public:
        Atari2600Emulator(std::string romPath) {
            sAppName = "Atari 2600 Emulator";
            this->romPath = romPath;
        }

    private:
        Atari atari;
        float residualTime = 0.0f;
        std::string romPath;

        bool OnUserCreate() override {
            std::ifstream ifs;
            ifs.open(romPath, std::ifstream::binary);

            if (ifs.is_open()) {
                ifs.read((char*) atari.RAM.data() + CART_OFFSET, SIZE_CART);
                ifs.close();
            } else {
                ifs.close();
                return false;
            }
            atari.reset();
            return true;
        }

        bool OnUserUpdate(float elapsedTime) override {
            uint8_t controller[2];

            controller[0] = 0x00;
            //controller[0] |= GetKey(olc::Key::J).bHeld ? 0x80 : 0x00;
            controller[0] |= GetKey(olc::Key::W).bHeld ? 0x00 : 0x10;
            controller[0] |= GetKey(olc::Key::S).bHeld ? 0x00 : 0x20;
            controller[0] |= GetKey(olc::Key::A).bHeld ? 0x00 : 0x40;
            controller[0] |= GetKey(olc::Key::D).bHeld ? 0x00 : 0x80;

            controller[1] = 0x00;
            controller[1] |= GetKey(olc::Key::F1).bHeld ? 0x00 : 0x01;
            controller[1] |= GetKey(olc::Key::F2).bHeld ? 0x00 : 0x02;
            controller[1] |= GetKey(olc::Key::F3).bHeld ? 0x00 : 0x08;
            controller[1] |= GetKey(olc::Key::F4).bHeld ? 0x00 : 0x40;
            controller[1] |= GetKey(olc::Key::F5).bHeld ? 0x00 : 0x80;

            atari.write8(SWCHA, controller[0]);
            atari.write8(SWCHB, controller[1]);

            if (GetKey(olc::Key::R).bPressed) {
                atari.reset();
            }
#if 0
            if (GetKey(olc::Key::SPACE).bPressed) {
                do { atari.step(); } while (atari.cpu.cycles != 0);
                do { atari.step(); } while (atari.cpu.cycles == 0);
            }
            if (GetKey(olc::Key::F).bPressed) {
                do { atari.step(); } while (!atari.tia.frameDone);
                atari.tia.frameDone = false;
            }
#endif
            if (GetKey(olc::Key::ESCAPE).bPressed) {
                return false;
            }

#if 1
            if (residualTime > 0.0) {
                residualTime -= elapsedTime;
            } else {
                residualTime += (1.0f / 60.0f) - elapsedTime;
                do { atari.step(); } while (!atari.tia.frameDone);
                atari.tia.frameDone = false;
            }
#endif

            DrawSprite(0, 0, &atari.tia.getScreen(), 1);

            return true;
        }
};

int main(int argc, char* argv[]) {
    if (argc > 1) {
        Atari2600Emulator emu(argv[1]);
        if (emu.Construct(WIDTH, HEIGHT, 4, 2)) {
            emu.Start();
        }
    } else {
        std::cout << "Provide path to ROM" << std::endl;
    }

    return 0;
}

