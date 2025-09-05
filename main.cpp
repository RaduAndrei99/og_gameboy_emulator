#include<signal.h>

#include "GAMEBOY/gameboy.hpp"

int main(int argc, char *argv[])
{
    gameboy gb;

    gb.load_cartridge("/Users/radu-andrei.budeanu/Projects/GBEmu/ROMs/test/cpu_instrs/individual/01-special.gb");
    //gb.load_cartridge("/Users/radu-andrei.budeanu/Projects/GBEmu/ROMs/test/cpu_instrs/individual/08-misc-instrs.gb");
    //gb.load_cartridge("/Users/radu-andrei.budeanu/Projects/GBEmu/ROMs/tetris.gb");

    gb.run();
    
    return 0;
}