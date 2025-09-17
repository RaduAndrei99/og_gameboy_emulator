#include "gb_bus.hpp"
#include <iostream>

const std::array<uint8_t, 256> bootDMG = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x00, 0x00, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x00, 0x00, 0x3E, 0x01, 0xE0, 0x50
};

uint8_t gb_bus::bus_read(const uint16_t& address)
{
    // cartridge read
    if(address < 0x8000)
    {
        if(boot_rom_active && 0x00 <= address && address <= 0xFF)
        {
            //std::cout<<"Getting from boot rom at: 0x"<<std::hex<<(int)address<<'\n';
            return bootDMG[address];
        }

        return cartridge? cartridge->read(address) : 0xFF;
    }

    // VRAM read
    if(0x8000 <= address && address <= 0x9FFF) return video->read(address - 0x8000);

    // external RAM read
    if(0xA000 <= address && address <= 0xBFFF)  return cartridge->read(address);

    // internal RAM read
    if(0xC000 <= address && address <= 0xDFFF) return mem.read_main(address - 0xC000);

    // mirrored RAM
    // Accessing this range may cause a lawsuit from Nintendo.
    if(0xE000 <= address && address <= 0xFDFF) 
    {
        return mem.read_main(address - 0x2000);
    }

    // OAM read
    if(0xFE00 <= address && address <= 0xFE9F) 
    {
        return mem.read_oam(address - 0xFE00);
    }

    // Not usable memory area
    if(0xFEA0 <= address && address <= 0xFEFF) 
    {
        std::cout<<"[READ] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';

        return 0xFF;
    }

    // I/O registers
    if(0xFF00 <= address && address <= 0xFF7F)
    {
        //std::cout<<"[READ] IO: "<<std::hex<<address<<'\n';
        if (address == 0xFF01) return serial_data;
        if (address == 0xFF02) return serial_control;
        if (address == 0xFF04) return timer->get_DIV();
        if (address == 0xFF05) return timer->get_TIMA();
        if (address == 0xFF06) return timer->get_TMA();
        if (address == 0xFF07) return timer->get_TAC();
        if (address == 0xFF0F) return cpu->get_IF(); // Interrupt flags
        if (address == 0xFF40) return video->read_LCDC();
        if (address == 0xFF41) return video->read_STAT();
        if (address == 0xFF42) return video->read_SCY();
        if (address == 0xFF43) return video->read_SCX();
        if (address == 0xFF44) return video->read_LY();
        if (address == 0xFF45) return video->read_LYC();
        if (address == 0xFF47) return video->read_BGP();

        //std::cout<<"[READ] IO not implemented: 0x"<<std::hex<<static_cast<int>(address)<<'\n';
        
        // TODO
        return 0xFF;
    };

    // HRAM
    if(0xFF80 <= address && address <= 0xFFFE)  return mem.read_hram(address - 0xFF80);

    // Interrupt enable register
    if(address == 0xFFFF) return cpu->get_IE();

    std::cout<<"[READ end] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';

    exit(-1);
}
void gb_bus::bus_write(const uint16_t& address, const uint8_t& data)
{
    // cartridge write
    if(address < 0x8000)
    {
        if(cartridge)
            cartridge->write(address, data);
        return;
    }

    // VRAM write
    if(0x8000 <= address && address <= 0x9FFF) 
    {
        video->write(address - 0x8000, data);
        return;
    }

    // external RAM write
    if(0xA000 <= address && address <= 0xBFFF)
    {
        cartridge->write(address, data);
        return;
    }

    // internal RAM write
    if(0xC000 <= address && address <= 0xDFFF) 
    {
        mem.write_main(address - 0xC000, data);
        return;
    }

    // Accessing this range may cause a lawsuit from Nintendo.
    // Mirrored RAM
    if(0xE000 <= address && address <= 0xFDFF)
    {
        mem.write_main(address - 0x2000, data);
        return;
    }

    // OAM write
    if(0xFE00 <= address && address <= 0xFE9F)
    {
        mem.write_oam(address - 0xFE00, data);
        return;
    }

    // Not usable memory area)
    if(0xFEA0 <= address && address <= 0xFEFF) 
    {
        std::cout<<"[WRITE] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';
        return;
        //exit(-1);
    }

    // I/O registers
    if(0xFF00 <= address && address <= 0xFF7F)
    {
        //std::cout<<"[WRITE] IO: "<<std::hex<<address<<'\n';

        if (address == 0xFF01) // SB - serial data
        { 
            serial_data = data;
            return;
        }
        if (address == 0xFF02) // SC - serial control
        {
            if (data == 0x81) 
            {
                // Blargg’s test: print immediately
                std::cout << static_cast<char>(serial_data);
            }
            serial_control = data;
            return;
        }
        if (address == 0xFF04) 
        { 
            timer->reset_DIV(); return; 
        }
        if (address == 0xFF05) 
        { 
            timer->set_TIMA(data); 
            return; 
        }
        if (address == 0xFF06) 
        { 
            timer->set_TMA(data); 
            return; 
        }
        if (address == 0xFF07) 
        { 
            timer->set_TAC(data); 
            return; 
        }
        if (address == 0xFF0F) // IF - interrupt flags
        {
            cpu->set_IF(data);
            return;
        }
        if (address == 0xFF40)
        {
            video->write_LCDC(data);
            return;
        }
        if (address == 0xFF41)
        {
            video->write_STAT(data);
            return;
        }
        if (address == 0xFF42)
        {
            video->write_SCY(data);
            return;
        }
        if (address == 0xFF43)
        {
            video->write_SCX(data);
            return;
        }
        if (address == 0xFF44)
        {
            video->write_LY(data);
            return;
        }
        if (address == 0xFF45)
        {
            video->write_LYC(data);
            return;
        }
        if (address == 0xFF47)
        {
            video->write_BGP(data);
            return;
        }

        if (address == 0xFF50) 
        {
            std::cout<<"BOOT ROM deactivated!"<<"\n";
            boot_rom_active = false;  // unmap boot ROM
            return;
        }

        //std::cout<<"[WRITE] IO not implemented: 0x"<<std::hex<<static_cast<int>(address)<<" data: 0x"<<std::hex<<static_cast<int>(data)<<'\n';
        // TODO
        return;
    }

    // HRAM
    if(0xFF80 <= address && address <= 0xFFFE)
    {
        mem.write_hram(address - 0xFF80, data);

        return;
    }

    // Interrupt enable register
    if(address == 0xFFFF)
    {
        cpu->set_IE(data);
        return;
    }
    
    std::cout<<"[WRITE] No valid address: 0x"<<std::hex<<static_cast<int>(address)<<'\n';

    exit(-1);
}
void gb_bus::set_cartridge(const std::shared_ptr<gb_cartridge>& c)
{
   cartridge = c; 
}
void gb_bus::set_cpu(const std::shared_ptr<sharpsm83>& c)
{
    cpu = c;
}
void gb_bus::tick(int cycles) 
{ 
    video->tick(4*cycles);
    timer->tick(cycles); 
}
void gb_bus::set_timer(const std::shared_ptr<gb_timer>& t)
{
    timer = t;
}
void gb_bus::set_video(const std::shared_ptr<gb_ppu>& v)
{
    video = v;
}
