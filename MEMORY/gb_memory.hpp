#ifndef _GB_RAM__
#define _GB_RAM__

#include <cstdint>

#define MAIN_MAX_MEMORY_SIZE 8 * 1024
#define HRAM_MAX_MEMORY_SIZE 0x80

struct gb_memory
{
    // 8 kb of main RAM
    uint8_t main_ram[MAIN_MAX_MEMORY_SIZE];

    uint8_t hram[HRAM_MAX_MEMORY_SIZE];

    // first chip enable, active low
    bool _ce;

    // second chip enable, active high
    bool ce2;

    // write eanble, active low
    bool _we;

    uint8_t read_main(uint16_t address);
    void write_main(uint16_t address, uint8_t dataIn);

    uint8_t read_hram(uint16_t address);
    void write_hram(uint16_t address, uint8_t dataIn);

    void print_memory_layout();
    void init_memory();
};

#endif