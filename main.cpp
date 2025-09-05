#include "GAMEBOY/gameboy.hpp"
#include <string>

int main(int argc, char *argv[])
{
    gameboy gb;

    gb.loadCartridge("/Users/radu-andrei.budeanu/Projects/GBEmu/ROMs/test/cpu_instrs/individual/01-special.gb");
    //gb.loadCartridge("/Users/radu-andrei.budeanu/Projects/GBEmu/ROMs/tetris.gb");

    return 0;
}