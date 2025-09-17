#include "gb_memory.hpp"
#include<iostream>

uint8_t gb_memory::read_main(uint16_t address)
{
    return address <= MAIN_MAX_MEMORY_SIZE ? main_ram[address] : 0xFF;
}
void gb_memory::write_main(uint16_t address, uint8_t dataIn)
{
    if(address <= MAIN_MAX_MEMORY_SIZE)
        main_ram[address] = dataIn;
}
void gb_memory::init_memory()
{
    for(int i=0; i<MAIN_MAX_MEMORY_SIZE; ++i)
    {
        main_ram[i] = 0;
    }
}
uint8_t gb_memory::read_hram(uint16_t address)
{
    return hram[address];
}
void gb_memory::write_hram(uint16_t address, uint8_t dataIn)
{
    if(address <= HRAM_MAX_MEMORY_SIZE)
        hram[address] = dataIn;
}
void gb_memory::print_memory_layout()
{

    std::cout<<"\t\t";

    for(int i=0;i<0x10;++i)
    {
        std::cout<<"0x"<<std::hex<<static_cast<int>(i)<<'\t';
    }
    std::cout<<'\n';
    std::cout<<"===============================================================================================================================================\n";

    for(int i=0; i<MAIN_MAX_MEMORY_SIZE; i+=0x10)
    {
        std::cout<<"0x"<<std::hex<<static_cast<int>(i)<<"\t || \t";
        for(int j=0; j<0x10; ++j)
        {
            std::cout<<"0x"<<std::hex<<static_cast<int>(main_ram[i+j])<<'\t';
        }
        std::cout<<'\n';
    }
}

uint8_t gb_memory::read_oam(uint16_t address)
{
    return oam[address];
}
void gb_memory::write_oam(uint16_t address, uint8_t dataIn)
{
    oam[address] = dataIn;
}