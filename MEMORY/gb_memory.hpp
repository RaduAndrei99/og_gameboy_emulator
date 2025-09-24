#ifndef _GB_RAM__
#define _GB_RAM__

#include <cstdint>

#define MAIN_MAX_MEMORY_SIZE 8 * 1024
#define HRAM_MAX_MEMORY_SIZE 0x80

struct gb_memory
{
    // 8 kb of main RAM
    uint8_t main_ram[MAIN_MAX_MEMORY_SIZE];

    // 127 bytes of high RAM
    uint8_t hram[HRAM_MAX_MEMORY_SIZE];

    // 160 bytes of OAM memory
    uint8_t oam[0xA0];

    uint8_t KEY1 = 0x00;
    uint8_t JOYP = 0x3F;

    uint8_t read_main(uint16_t address);
    void write_main(uint16_t address, uint8_t dataIn);

    uint8_t read_hram(uint16_t address);
    void write_hram(uint16_t address, uint8_t dataIn);

    uint8_t read_oam(uint16_t address);
    void write_oam(uint16_t address, uint8_t dataIn);

    void print_memory_layout();
    void init_memory();
};

#endif