#include "gb_bus.hpp"
#include <iostream>

uint8_t gb_bus::bus_read(const uint16_t& address)
{
    // ROM read
    if(address < 0x8000) return cartridge->read(address);

    if(0xC000 <= address && address <= 0xDFFF) return mem.read(address);

    // Nintendo sends a lawyer if you access this range
    if(0xE000 <= address && address <= 0xFDFF) exit(-1);
    if(0xFEA0 <= address && address <= 0xFEFF) exit(-1);


    if (address == 0xFF01) return serial_data;
    if (address == 0xFF02) return serial_control;

    std::cout<<"[READ] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';

    exit(-1);
}
void gb_bus::bus_write(const uint16_t& address, const uint8_t& data)
{
    // ROM write
    if(address < 0x8000)
    {
        cartridge->write(address, data);
    }
    else if(0xC000 <= address && address <= 0xDFFF) 
    {
        mem.write(address, data);
    }
    // Nintendo sends a lawyer if you access this range
    else if(0xE000 <= address && address <= 0xFDFF)
    {
        exit(-1);
    }
    else if(0xFEA0 <= address && address <= 0xFEFF) 
    {
        exit(-1);
    }
    else if (address == 0xFF01) // SB - serial data
    { 
        serial_data = data;
    }
    else if (address == 0xFF02) // SC - serial control
    {
        if (data == 0x81) 
        {
            // Blargg’s test: print immediately
            std::cout << static_cast<char>(serial_data);
            std::cout.flush();
        }
        serial_control = data;
    }
    else
    {
        std::cout<<"[WRITE] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';

        exit(-1);
    }
}
void gb_bus::set_cartridge(const std::shared_ptr<gb_cartridge>& c)
{
   cartridge = c; 
}