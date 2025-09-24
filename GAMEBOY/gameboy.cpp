#include "gameboy.hpp"
#include <iostream>

gameboy::gameboy()
{
    bus = std::make_shared<gb_bus>();
    cpu = std::make_shared<sharpsm83>();
    video = std::make_shared<gb_ppu>();

    cpu->set_bus(bus);
    bus->set_cpu(cpu);

    timer = std::make_shared<gb_timer>();

    bus->set_timer(timer);
    timer->set_bus(bus);

    bus->set_video(video);
    video->set_bus(bus);   
}

void gameboy::load_cartridge(const std::string& path)
{
    cartridge = load_and_construct_cartridge(path);
    bus->set_cartridge(cartridge);
}

const std::shared_ptr<gb_cartridge>& gameboy::get_cartridge()
{
    return cartridge;
}

std::shared_ptr<gb_ppu>& gameboy::get_video()\
{
    return video;
}

const std::shared_ptr<sharpsm83>& gameboy::get_cpu() const
{
    return cpu;
}

std::shared_ptr<sharpsm83>& gameboy::get_cpu()
{
    return cpu;
}

std::shared_ptr<gb_bus>& gameboy::get_bus()
{
    return bus;
}

std::shared_ptr<gb_timer>& gameboy::get_timer()
{
    return timer;
}

void gameboy::run()
{
    is_running = true;
    
    cpu->reset();
    
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

        int cycles = cpu->tick();
        if(cycles == -1)
            break;
    }
}

void gameboy::reset()
{
    cpu->reset();
}

void gameboy::emulate_cycles(const long int& cycles)
{

}


