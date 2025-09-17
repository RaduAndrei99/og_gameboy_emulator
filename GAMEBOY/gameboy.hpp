#ifndef _GAMEBOY_
#define _GAMEBOY_

#include<memory>

#include "../CPU/cpu_sharpsm83.hpp"
#include "../CARTRIDGE/gb_cartridge.hpp"
#include "../BUS/gb_bus.hpp"
#include "../TIMER/gb_timer.hpp"

class gameboy{
private:
    std::shared_ptr<gb_bus> bus;
    std::shared_ptr<gb_cartridge> cartridge;
    std::shared_ptr<sharpsm83> cpu;
    std::shared_ptr<gb_timer> timer;
    std::shared_ptr<gb_ppu> video;

    bool is_running = false;
    
public:
    gameboy();
    void load_cartridge(const std::string& path);

    const std::shared_ptr<gb_cartridge>& get_cartridge();

    const std::shared_ptr<sharpsm83>& get_cpu() const;
    std::shared_ptr<sharpsm83>& get_cpu();
    std::shared_ptr<gb_bus>& get_bus();
    std::shared_ptr<gb_ppu>& get_video();
    std::shared_ptr<gb_timer>& get_timer();

    void run();

    void reset();

    void emulate_cycles(const long int& cycles);

    
};
#endif