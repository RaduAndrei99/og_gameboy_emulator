#include<signal.h>

#include<cassert>

#include "GAMEBOY/gameboy.hpp"

void test_cartridge(gameboy& gb, std::string path)
{
    gb.load_cartridge(path);

    gb.run();
}

int main(int argc, char *argv[])
{
    gameboy gb;
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/01-special.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/02-interrupts.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/03-op-sp,hl.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/04-op-r,imm.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/05-op-rp.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/06-ld-r,r.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/08-misc-instrs.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/09-op-r,r.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/10-bit-ops.gb");
    test_cartridge(gb, "../ROMs/test/cpu_instrs/individual/11-op-a,(hl).gb");
    //test_cartridge(gb, "../ROMs/test/cpu_instrs/cpu_instrs.gb");

    return 0;
}