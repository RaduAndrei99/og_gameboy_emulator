#include<signal.h>

#include "GAMEBOY/gameboy.hpp"

int main(int argc, char *argv[])
{
    gameboy gb;
    
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/01-special.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/02-interrupts.gb");
    gb.load_cartridge("../ROMs/test/cpu_instrs/individual/03-op-sp,hl.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/04-op-r,imm.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/05-op-rp.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/06-ld-r,r.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/08-misc-instrs.gb");


    //gb.load_cartridge("../ROMs/test/cpu_instrs/cpu_instrs.gb");
    //gb.load_cartridge("../ROMs/tetris.gb");

    gb.run();
    
    return 0;
}