#ifndef _GB_BUS_
#define _GB_BUS_

#include "../MEMORY/gb_memory.hpp"
#include "../CARTRIDGE/gb_cartridge.hpp"
#include "../CPU/cpu_sharpsm83.hpp"

class sharpsm83;

struct gb_bus
{
    std::shared_ptr<gb_cartridge> cartridge;
    std::shared_ptr<sharpsm83> cpu;

    uint8_t serial_data = 0;     // SB (0xFF01)
    uint8_t serial_control = 0;  // SC (0xFF02)

    gb_memory mem;

    uint8_t bus_read(const uint16_t& address);
    void bus_write(const uint16_t& address, const uint8_t& data);

    void set_cartridge(const std::shared_ptr<gb_cartridge>& c);
    void set_cpu(const std::shared_ptr<sharpsm83>& c);

};

#endif