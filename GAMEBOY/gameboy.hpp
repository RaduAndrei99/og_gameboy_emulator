#ifndef _GAMEBOY_
#define _GAMEBOY_

#include<memory>

#include "../CPU/cpu_sharpsm83.hpp"
#include "../CARTRIDGE/gb_cartridge.hpp"

class gameboy{
private:
    sharpsm83 cpu;
    std::unique_ptr<gb_cartridge> cartridge;

public:
    void loadCartridge(const std::string& path);
};
#endif