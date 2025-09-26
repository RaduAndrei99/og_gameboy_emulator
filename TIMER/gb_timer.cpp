#include "gb_timer.hpp"
#include <iostream>

const uint CLOCKS_PER_CYCLE = 4;

gb_timer::gb_timer() : div(0xAC00), tima(0), tma(0), tac(0xF8), timer_counter(0), bus(nullptr) 
{

}
gb_timer::~gb_timer() = default;

void gb_timer::tick(int ticks) 
{
    if(ticks < 0) 
    {
        std::cout<<"Can't emulate negative cycles!"<<'\n';
        return;
    }

    while (ticks--)  // handle one CPU cycle at a time
    {
        uint16_t old_div = div;

        div++;
        
        if (!(tac & 0x04)) continue; // timer disabled

        int bit;
        switch (tac & 0x03) 
        {
            case 0: bit = 9; break;   // 4096 Hz
            case 1: bit = 3; break;   // 262144 Hz
            case 2: bit = 5; break;   // 65536 Hz
            case 3: bit = 7; break;   // 16384 Hz
        }

        // detect falling edge of DIV[bit]
        bool old_bit = (old_div >> bit ) & 1;
        bool new_bit = (div >> bit) & 1;

        // 1 -> 0 falling edge
        if (old_bit && !new_bit) 
        {
            // overflow
            if (tima == 0xFF) 
            { 
                tima = 0x00; // reload
                bus->cpu->set_IF(bus->cpu->get_IF() | 0x4); //s set IF.b2
                overflow_pending = true;
            }
            else 
            {
                tima++; 
            }
        }

        // if an overflow is pending, reload TIMA with TMA after one machine cycle
        if (overflow_pending) 
        {
            tima = tma;
            overflow_pending = false;
        }
    }
}

uint8_t gb_timer::get_DIV() const 
{ 
    return div >> 8; 
}
void gb_timer::reset_DIV() 
{ 
    div = 0;
}
uint8_t gb_timer::get_TIMA() const 
{ 
    return tima; 
}
void gb_timer::set_TIMA(uint8_t v) 
{ 
    tima = v; 
}
uint8_t gb_timer::get_TMA() const 
{ 
    return tma; 
}
void gb_timer::set_TMA(uint8_t v) 
{ 
    tma = v; 
}

uint8_t gb_timer::get_TAC() const 
{ 
    return tac; 
}
void gb_timer::set_TAC(uint8_t v) 
{ 
    tac = v & 0x07; 
}
void gb_timer::set_bus(const std::shared_ptr<gb_bus>& b) 
{
    bus = b; 
}
void gb_timer::print_status()
{
    std::cout<<std::hex<<"DIV: "<<div<<'\n';
    std::cout<<std::hex<<"TIMA: "<<tima<<'\n';
    std::cout<<std::hex<<"TMA: "<<tma<<'\n';
    std::cout<<std::hex<<"TAC: "<<tac<<'\n';
    std::cout<<'\n';
}
