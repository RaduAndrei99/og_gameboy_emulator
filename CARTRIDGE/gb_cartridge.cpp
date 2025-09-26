#include<cstdlib>

#include "gb_cartridge.hpp"
#include "../UTILS/file_io.hpp"

std::shared_ptr<gb_cartridge> load_and_construct_cartridge(const std::string& path)
{
    std::vector<uint8_t> raw_data = read_file_to_vector(path);

    if(raw_data.size() == 0) 
    {
        std::cout<<"failed to read file: " + path<<'\n';
        exit(-1);
    }

    std::cout<<"Loaded "<< path<<" with "<<raw_data.size()<<" bytes."<<'\n';

    uint8_t cart_type = raw_data[cartridge_header::CARTRIDGE_TYPE];
    
    switch(cart_type)
    {
        case 0x0: return std::make_shared<no_mbc>(raw_data);
        
        case 0x1: 
        case 0x2: 
        case 0x3: 
            return std::make_shared<mbc1>(raw_data);

        default:
        {
            std::cout<<"[CARTRIDGE] Unknown cartridge type: "<<(int)cart_type<<'\n';
            exit(-1);
        };
    }
}

gb_cartridge::gb_cartridge(std::vector<uint8_t>& rom)
{
    raw_data = rom;

    if(raw_data.size() == 0)
        return;

    load_info();

    ram.resize(info.ram_size);

    print_info();
}

void gb_cartridge::load_info()
{
    info.title = std::string(raw_data.begin() + cartridge_header::TITLE, 
                             raw_data.begin() + cartridge_header::TITLE_END);

    info.manufacturer_code = std::string(raw_data.begin() + cartridge_header::MANUFACTURER_CODE, 
                             raw_data.begin() + cartridge_header::MANUFACTURER_CODE_END);
    
    info.cgb_flag = raw_data[cartridge_header::CGB_FLAG];

    info.license_code = std::string(raw_data.begin() + cartridge_header::NEW_LICENSE_CODE, 
                             raw_data.begin() + cartridge_header::NEW_LICENSE_CODE_END);

    info.sgb_flag = raw_data[cartridge_header::SGB_FLAG];

    info.cartridge_type = raw_data[cartridge_header::CARTRIDGE_TYPE];

    info.rom_size = raw_data[cartridge_header::ROM_SIZE];
    info.ram_size = raw_data[cartridge_header::RAM_SIZE];
    info.destination_code = raw_data[cartridge_header::DESTINATION_CODE];

    info.old_license_code = raw_data[cartridge_header::OLD_LICENSE_CODE];
    info.mask_rom_version_number = raw_data[cartridge_header::MASK_ROM_VERSION_NUMBER];

    auto validate_header_checksum = [this](std::vector<uint8_t>& raw_data, uint8_t value)
    {
        uint8_t checksum = 0;
        for (uint16_t address = 0x0134; address <= 0x014C; address++) {
            checksum = checksum - raw_data[address] - 1;
        }

        if(value == checksum)
        {
            std::cout<<"Header checksum is correct!"<<'\n';
        }
        else
        {
            std::cout<<"Header checksum failed!"<<'\n';
        }
    };

    info.header_checksum = raw_data[cartridge_header::HEADER_CHECKSUM];
    validate_header_checksum(raw_data, info.header_checksum);

    auto validate_global_checksum = [this](std::vector<uint8_t>& raw_data, uint16_t value)
    {
        uint16_t checksum = 0;
        for (int address = 0; address<raw_data.size(); address++) {
            if(address == 0x014E || address == 0x014F ) continue;
            checksum = checksum + raw_data[address];
        }

        if(value == checksum)
        {
            std::cout<<"Global checksum is correct!"<<'\n';
        }
        else
        {
            std::cout<<"Global checksum failed!"<<'\n';
        }
    };

    uint8_t high = raw_data[cartridge_header::GLOBAL_CHECKSUM];
    uint8_t low = raw_data[cartridge_header::GLOBAL_CHECKSUM_END];
    info.global_checksum = (high << 8) | low;
    validate_global_checksum(raw_data, info.global_checksum);    
}

std::string namecode_to_publisher_value(const std::string& code)
{
    auto it = code_to_publisher.find(code); 
    if (it != code_to_publisher.end())
        return it->second;
    return "Unknown";
}
std::string code_to_cartridge_type(uint8_t code)
{
    auto it = gb_cartridge_types.find(code); 
    if (it != gb_cartridge_types.end())
        return it->second;
    return "Unknown";
}
std::string code_to_rom_sizes_type(uint8_t code)
{
    auto it = gb_rom_sizes.find(code); 
    if (it != gb_rom_sizes.end())
        return it->second;
    return "Unknown";
}

std::string code_to_ram_sizes_type(uint8_t code)
{
    auto it = gb_ram_sizes.find(code); 
    if (it != gb_ram_sizes.end())
        return it->second;
    return "Unknown";
}

std::string code_to_destination_code(uint8_t code)
{
    auto it = gb_destination_codes.find(code); 
    if (it != gb_destination_codes.end())
        return it->second;
    return "Unknown";
}

std::string namecode_to_old_licensees(uint8_t code)
{
    auto it = gb_old_licensees.find(code); 
    if (it != gb_old_licensees.end())
        return it->second;
    return "Unknown";
}

void gb_cartridge::print_info()
{
    std::cout<<"Entry point: "
    <<"0x"<<std::hex<<static_cast<int>(raw_data[0x0100]) << ' '
    <<"0x"<<std::hex<<static_cast<int>(raw_data[0x0101]) << ' '
    <<"0x"<<std::hex<<static_cast<int>(raw_data[0x0102]) << ' '
    <<"0x"<<std::hex<<static_cast<int>(raw_data[0x0103]) <<'\n';

    std::cout<<"Nintendo logo: "<<'\n';
    for(int i=0x104; i<0x133; i+=0x10)
    {
        for(int j=0; j<0x10; ++j)
        {
            std::cout<<"0x"<<std::hex<<static_cast<int>(raw_data[i+j])<<'\t';
        }
        std::cout<<'\n';
    }

    std::cout<<"Title: "<<info.title<<'\n';
    std::cout<<"Manufacturer code: "<<info.manufacturer_code<<" (may be empty)"<<'\n';
    std::cout<<"CGB flag: 0x"<<std::hex<<static_cast<int>(info.cgb_flag)<<" (may be empty)"<<'\n';
    std::cout<<"New license code: "<<info.license_code<<" -> " << namecode_to_publisher_value(info.license_code) 
    <<" (meaningfull only if old license is 0x33)"<<'\n';
    std::cout<<"SGB flag: 0x"<<std::hex<<static_cast<int>(info.sgb_flag)<<" (may be empty)"<<'\n';
    std::cout<<"Cartridge type: "<<std::hex<<static_cast<int>(info.cartridge_type)<<" -> " << code_to_cartridge_type(info.cartridge_type) 
    <<'\n';

    std::cout<<"ROM size: 0x"<<std::hex<<static_cast<int>(info.rom_size)<<" -> " << code_to_rom_sizes_type(info.rom_size)<<'\n';

    std::cout<<"RAM size: 0x"<<std::hex<<static_cast<int>(info.ram_size)<<" -> " << code_to_ram_sizes_type(info.ram_size)<<'\n';

    std::cout<<"Destination code: 0x"<<std::hex<<static_cast<int>(info.destination_code)<<" -> " << code_to_destination_code(info.destination_code)<<'\n';

    std::cout<<"Old license code: 0x"<<std::hex<<static_cast<int>(info.old_license_code)<<" -> " << namecode_to_old_licensees(info.old_license_code) 
    <<'\n';

    std::cout<<"Mask ROM version number: 0x"<<std::hex<<static_cast<int>(info.mask_rom_version_number)<<'\n';
    std::cout<<"Header checksum: 0x"<<std::hex<<static_cast<int>(info.header_checksum)<<'\n';
    std::cout<<"Global checksum: 0x"<<std::hex<<static_cast<int>(info.global_checksum)<<'\n';
}

no_mbc::no_mbc(std::vector<uint8_t>& rom) : gb_cartridge(rom)
{
    std::cout<<"No mbc cartridge!"<<'\n';
}

uint8_t no_mbc::read(const uint16_t& address)
{
    return raw_data[address];
}

void no_mbc::write(const uint16_t& address, const uint8_t& data)
{
    //std::cout<<"[CARTRIDGE] No MBC write!"<<'\n';
}

mbc1::mbc1(std::vector<uint8_t>& rom) : gb_cartridge(rom)
{
    std::cout<<"MBC1 cartridge!"<<'\n';
}
void mbc1::write(const uint16_t& address, const uint8_t& data)
{
    if(address < 0x2000)
    {
        ram_enabled = ((data & 0xF) == 0xA);
    } 
    else if(address < 0x4000)
    {
        rom_bank_low = data & 0x1F;

        if(rom_bank_low == 0x0) rom_bank_low = 0x1;
    }
    else if(address < 0x6000)
    {
        // either the high bits of rom or the ram depending on the mode
        rom_bank_high = data & 0x03;
        ram_bank = data & 0x03;
    }
    else if(address < 0x8000)
    {
        mode = data & 0x01;
    }
    else if (0xA000 <= address && address < 0xC000) 
    {
        if (!ram_enabled || ram.size() == 0) return;
        
        uint32_t bank = (mode == 0) ? 0 : ram_bank;
        uint32_t offset = bank * 0x2000 + (address - 0xA000);
        ram[offset] = data;
    }
}

uint8_t mbc1::read(const uint16_t& address)
{
    if (address < 0x4000) 
    {
        return raw_data[address];
    }
    else if (address < 0x8000) 
    {
        // Switchable bank
        if (rom_bank_low == 0) rom_bank_low = 0x1; // hardware quirk: bank 0 -> 1
        uint32_t bank = (rom_bank_high << 5) | rom_bank_low;
        
        uint32_t offset = bank * 0x4000 + (address - 0x4000);

        return raw_data[offset];
    }
    else if (0xA000 <= address && address < 0xC000) 
    {
        if (!ram_enabled) return 0xFF;
        
        uint32_t bank = (mode == 0) ? 0 : ram_bank;
        uint32_t offset = bank * 0x2000 + (address - 0xA000);
        return ram[offset];
    }
    
    return 0xFF;
}

