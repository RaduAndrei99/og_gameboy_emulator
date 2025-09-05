#include "gameboy.hpp"

void gameboy::loadCartridge(const std::string& path)
{
    cartridge = std::make_unique<gb_cartridge>();

    cartridge->loadCartridge(path);
}
