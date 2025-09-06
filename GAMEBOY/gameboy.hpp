#ifndef _GAMEBOY_
#define _GAMEBOY_

#include<memory>

#include "../CPU/cpu_sharpsm83.hpp"
#include "../CARTRIDGE/gb_cartridge.hpp"
#include "../BUS/gb_bus.hpp"

class gameboy{
private:
    std::shared_ptr<gb_bus> bus;
    std::shared_ptr<gb_cartridge> cartridge;

    sharpsm83 cpu;

    bool is_running = false;

public:
    gameboy();
    void load_cartridge(const std::string& path);

    const std::shared_ptr<gb_cartridge>& get_cartridge();

    const sharpsm83& get_cpu();

    void run();
};
#endif