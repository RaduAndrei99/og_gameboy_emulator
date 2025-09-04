#include "gb_wram.hpp"
#include<iostream>

uint8_t gb_wram::read(uint8_t address)
{
    return data[address];
}
void gb_wram::write(uint16_t address, uint8_t dataIn)
{
    if(address <= MAX_MEMORY_SIZE)
        data[address] = dataIn;
}
void gb_wram::init_memory()
{
    for(int i=0; i<MAX_MEMORY_SIZE; ++i)
    {
        data[i] = 0;
    }
}
void gb_wram::print_memory_layout()
{

    std::cout<<"\t\t";

    for(int i=0;i<0x10;++i)
    {
        std::cout<<"0x"<<std::hex<<static_cast<int>(i)<<'\t';
    }
    std::cout<<'\n';
    std::cout<<"===============================================================================================================================================\n";

    for(int i=0; i<MAX_MEMORY_SIZE; i+=0x10)
    {
        std::cout<<"0x"<<std::hex<<static_cast<int>(i)<<"\t || \t";
        for(int j=0; j<0x10; ++j)
        {
            std::cout<<"0x"<<std::hex<<static_cast<int>(data[i+j])<<'\t';
        }
        std::cout<<'\n';
    }

}