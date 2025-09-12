#include "gb_bus.hpp"
#include <iostream>

uint8_t gb_bus::bus_read(const uint16_t& address)
{
    // cartridge read
    if(address < 0x8000)
    {
        return cartridge? cartridge->read(address) : 0xFF;
    }

    // VRAM read
    // TODO
    if(0x8000 <= address && address <= 0x9FFF) return 0xFF;

    // external RAM read
    if(0xA000 <= address && address <= 0xBFFF)  return cartridge->read(address);

    // internal RAM read
    if(0xC000 <= address && address <= 0xDFFF) return mem.read_main(address - 0xC000);

    // mirrored RAM
    // Accessing this range may cause a lawsuit from Nintendo.
    if(0xE000 <= address && address <= 0xFDFF) 
    {
        return mem.read_main(address - 0x2000);
    }

    // OAM read
    if(0xFE00 <= address && address <= 0xFE9F) 
    {
        // TODO
        return 0xFF;
    }

    // Not usable memory area
    if(0xFEA0 <= address && address <= 0xFEFF) 
    {
        std::cout<<"[READ] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';

        return 0xFF;
    }

    // I/O registers
    if(0xFF00 <= address && address <= 0xFF7F)
    {
        //std::cout<<"[READ] IO: "<<std::hex<<address<<'\n';
        if (address == 0xFF01) return serial_data;
        if (address == 0xFF02) return serial_control;
        if (address == 0xFF0F) return cpu->get_IF(); // Interrupt flags

        // TODO
        return 0xFF;
    };

    // HRAM
    if(0xFF80 <= address && address <= 0xFFFE)  return mem.read_hram(address - 0xFF80);

    // Interrupt enable register
    if(address == 0xFFFF) return cpu->get_IE();

    std::cout<<"[READ end] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';

    exit(-1);
}
void gb_bus::bus_write(const uint16_t& address, const uint8_t& data)
{
    // cartridge write
    if(address < 0x8000)
    {
        if(cartridge)
            cartridge->write(address, data);
        return;
    }

    // VRAM write
    if(0x8000 <= address && address <= 0x9FFF) 
    {
        // TODO
        return;
    }

    // external RAM write
    if(0xA000 <= address && address <= 0xBFFF)
    {
        cartridge->write(address, data);
        return;
    }

    // internal RAM write
    if(0xC000 <= address && address <= 0xDFFF) 
    {
        mem.write_main(address - 0xC000, data);
        return;
    }

    // Accessing this range may cause a lawsuit from Nintendo.
    // Mirrored RAM
    if(0xE000 <= address && address <= 0xFDFF)
    {
        mem.write_main(address - 0x2000, data);
        return;
    }

    // OAM write
    if(0xFE00 <= address && address <= 0xFE9F)
    {
        // TODO
        return;
    }

    // Not usable memory area)
    if(0xFEA0 <= address && address <= 0xFEFF) 
    {
        std::cout<<"[WRITE] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';
        //exit(-1);
    }

    // I/O registers
    if(0xFF00 <= address && address <= 0xFF7F)
    {
        //std::cout<<"[WRITE] IO: "<<std::hex<<address<<'\n';

        if (address == 0xFF01) // SB - serial data
        { 
            serial_data = data;
        }
        else if (address == 0xFF02) // SC - serial control
        {
            if (data == 0x81) 
            {
                // Blargg’s test: print immediately
                std::cout << static_cast<char>(serial_data);
            }
            serial_control = data;
        }
        else if (address == 0xFF0F) // IF - interrupt flags
        {
            cpu->set_IF(data);
        }

        // TODO
        return;
    }

    // HRAM
    if(0xFF80 <= address && address <= 0xFFFE)
    {
        mem.write_hram(address - 0xFF80, data);

        return;
    }

    // Interrupt enable register
    if(address == 0xFFFF)
    {
        cpu->set_IE(data);
        return;
    }
    
    std::cout<<"[WRITE] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';

    exit(-1);
}
void gb_bus::set_cartridge(const std::shared_ptr<gb_cartridge>& c)
{
   cartridge = c; 
}
void gb_bus::set_cpu(const std::shared_ptr<sharpsm83>& c)
{
    cpu = c;
}
