#define OLC_PGE_APPLICATION
#include "EMULATOR/gb_emulator.hpp"
//#include "GAMEBOY/gameboy.hpp"


int main(int argc, char *argv[])
{

    // gameboy gb;
    // gb.load_cartridge("ROMs/test/halt_bug.gb");
    // gb.run();

    gb_emulator emu;

    if (emu.Construct(160, 144, 4, 4))
        emu.Start();

    return 0;
}