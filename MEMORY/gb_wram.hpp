#ifndef _GB_RAM__
#define _GB_RAM__

#include <cstdint>

#define MAX_MEMORY_SIZE 8 * 1024

struct gb_wram
{
    // 8 kb of main RAM
    uint8_t data[MAX_MEMORY_SIZE];

    // first chip enable, active low
    bool _ce;

    // second chip enable, active high
    bool ce2;

    // write eanble, active low
    bool _we;

    uint8_t read(uint8_t address);
    void write(uint16_t address, uint8_t dataIn);

    void print_memory_layout();
    void init_memory();
};

#endif