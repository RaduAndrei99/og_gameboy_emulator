#include "gb_joypad.hpp"

uint8_t gb_joypad::read()
{
    uint8_t result = 0xFF; // default all released

    if (!(JOYP & 0x10)) 
    { 
        if (right) result &= ~(1 << 0);
        if (left)  result &= ~(1 << 1);
        if (up)    result &= ~(1 << 2);
        if (down)  result &= ~(1 << 3);
    }
    else if (!(JOYP & 0x20)) 
    { 
        if (a)      result &= ~(1 << 0);
        if (b)      result &= ~(1 << 1);
        if (select) result &= ~(1 << 2);
        if (start)  result &= ~(1 << 3);
    }

    return (result & 0x0F) | (JOYP & 0xF0);
}

void gb_joypad::write(uint8_t value)
{
    // Only bits 4 (P14) and 5 (P15) are writable
    JOYP = (JOYP & 0xCF) | (value & 0x30);
}

void gb_joypad::update() 
{
    uint8_t new_state = read();

    // Detect falling edges (0 = pressed)
    uint8_t changed = prev_state & ~new_state;

    if (changed) 
    {
        // Trigger Joypad interrupt (bit 4 of IF)
        bus->cpu->set_IF(bus->cpu->get_IF() | 0x10);
    }

    prev_state = new_state;
}

void gb_joypad::set_bus(const std::shared_ptr<gb_bus>& b)
{
    bus = b;
}

void gb_joypad::set_right(bool val)
{
    right = val;
}
void gb_joypad::set_left(bool val)
{
    left = val;
}
void gb_joypad::set_up(bool val)
{
    up = val;
}
void gb_joypad::set_down(bool val)
{
    down = val;
}
void gb_joypad::set_a(bool val)
{
    a = val;
}
void gb_joypad::set_b(bool val)
{
    b = val;
}
void gb_joypad::set_select(bool val)
{
    select = val;
}
void gb_joypad::set_start(bool val)
{
    start = val;
}