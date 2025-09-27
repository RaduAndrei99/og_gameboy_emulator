#ifndef _GB_BUS_
#define _GB_BUS_

#include "../MEMORY/gb_memory.hpp"
#include "../CARTRIDGE/gb_cartridge.hpp"
#include "../CPU/cpu_sharpsm83.hpp"
#include "../TIMER/gb_timer.hpp"
#include "../VIDEO/gb_ppu.hpp"
#include "../JOYPAD/gb_joypad.hpp"


class sharpsm83;
class gb_timer;
class gb_ppu;
class gb_joypad;

struct gb_bus
{
    std::shared_ptr<gb_cartridge> cartridge;
    std::shared_ptr<sharpsm83> cpu;
    std::shared_ptr<gb_ppu>  video;
    std::shared_ptr<gb_timer> timer;
    std::shared_ptr<gb_joypad> joypad;

    bool boot_rom_active = true;

    gb_memory mem;

    uint8_t serial_data = 0;     // SB (0xFF01)
    uint8_t serial_control = 0;  // SC (0xFF02)

    uint8_t bus_read(const uint16_t& address);
    void bus_write(const uint16_t& address, const uint8_t& data);

    void set_cartridge(const std::shared_ptr<gb_cartridge>& c);
    void set_cpu(const std::shared_ptr<sharpsm83>& c);
    void set_timer(const std::shared_ptr<gb_timer>& t);
    void set_video(const std::shared_ptr<gb_ppu>& v);
    void set_joypad(const std::shared_ptr<gb_joypad>& j);

    void tick(int cycles);

    void dma_transfer(uint8_t byte);
};

#endif