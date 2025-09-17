#define OLC_PGE_APPLICATION
#include "EMULATOR/gb_emulator.hpp"

int main(int argc, char *argv[])
{
    gb_emulator emu;

    if (emu.Construct(160, 144, 4, 4))
        emu.Start();

    return 0;
}