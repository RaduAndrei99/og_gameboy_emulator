#include "gameboy.hpp"
#include <iostream>

gameboy::gameboy()
{
    bus = std::make_shared<gb_bus>();
    cpu = std::make_shared<sharpsm83>();

    cpu->set_bus(bus);
    bus->set_cpu(cpu);
}


void gameboy::load_cartridge(const std::string& path)
{
    cartridge = std::make_unique<gb_cartridge>();
    bus->set_cartridge(cartridge);

    cartridge->load_cartridge(path);
}

const std::shared_ptr<gb_cartridge>& gameboy::get_cartridge()
{
    return cartridge;
}

const std::shared_ptr<sharpsm83>& gameboy::get_cpu() const
{
    return cpu;
}
std::shared_ptr<sharpsm83>& gameboy::get_cpu()
{
    return cpu;
}
void gameboy::run()
{
    is_running = true;
    
    cpu->reset();

    int count_nop = 0;
    bool valid = false;

    while(is_running)
    {
        // if(cpu.get_current_adrress() == 0xc46b )
        // {         
        //     valid = true;

        //     int n;
        //     std::cin >> n;
        //     if(n == 0)
        //         cpu.print_registers();
            
        //     std::cin >> n;

        // }

        cpu->tick();
    }
}

void gameboy::reset()
{
    cpu.reset();
}


