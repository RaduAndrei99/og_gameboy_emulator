#ifndef _GB_JOYPAD_
#define _GB_JOYPAD_

#include<cstdint>
#include<memory>

#include "../BUS/gb_bus.hpp"

struct gb_buss;

class gb_joypad
{
private:

    std::shared_ptr<gb_bus> bus;

    bool right = false, left = false, up = false, down = false;
    bool a = false, b = false, select = false, start = false;

    uint8_t JOYP = 0x3F;

    uint8_t prev_state = 0x3F;
public:
    uint8_t read();
    void write(uint8_t value);

    void update();

    void set_bus(const std::shared_ptr<gb_bus>& b);
    
    void set_right(bool val);
    void set_left(bool val);
    void set_up(bool val);
    void set_down(bool val);

    void set_a(bool val);
    void set_b(bool val);
    void set_select(bool val);
    void set_start(bool val);

};

#endif