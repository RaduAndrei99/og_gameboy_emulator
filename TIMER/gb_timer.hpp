#ifndef _GB_TIMER_
#define _GB_TIMER_

#include <cstdint>
#include <memory>

#include "../BUS/gb_bus.hpp"

struct gb_bus;

class gb_timer
{
public:
    uint16_t div = 0;  // internal 16-bit divider
    uint8_t tima = 0;
    uint8_t tma = 0;
    uint8_t tac = 0;

    int timer_counter = 0; // counts down CPU cycles until next TIMA tick
    uint clocks = 0;

    bool overflow_pending = false;

    std::shared_ptr<gb_bus> bus; // to request interrupts

public:
    gb_timer();
    ~gb_timer();

    void tick(int ticks); // call this each CPU step

    uint8_t get_DIV() const;
    void reset_DIV();

    uint8_t get_TIMA() const;
    void set_TIMA(uint8_t val);

    uint8_t get_TMA() const;
    void set_TMA(uint8_t val);

    uint8_t get_TAC() const;
    void set_TAC(uint8_t val);

    void set_bus(const std::shared_ptr<gb_bus>& b);
    
    void print_status();
};

#endif