#ifndef _GB_BUS_
#define _GB_BUS_

#include "../MEMORY/gb_wram.hpp"
#include "../CARTRIDGE/gb_cartridge.hpp"

struct gb_bus
{
    std::shared_ptr<gb_cartridge> cartridge;

    uint8_t serial_data = 0;     // SB (0xFF01)
    uint8_t serial_control = 0;  // SC (0xFF02)

    gb_wram mem;

    uint8_t bus_read(const uint16_t& address);
    void bus_write(const uint16_t& address, const uint8_t& data);

    void set_cartridge(const std::shared_ptr<gb_cartridge>& c);
};

#endif