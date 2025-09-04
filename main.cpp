#include "CPU/cpu_sharpsm83.hpp"

int main()
{
    sharpsm83 sm83;

    std::vector<uint8_t> instructions = {
        0x00, 0x01, 0x02, 0x03, 
        0x04, 0x05, 0x06, 0x07, 
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F};

    sm83.reset();

    // normal instructions
    for(int i=0;i<0xC0 + 1; ++i)
    {
        sm83.execute(i);
    }

    // OxCB instructions
    for(int i=0;i<0xF + 1; ++i)
    {
        sm83.execute_0xCB_instruction(i);
    }

    sm83.printRegisters();
    
    return 0;
}