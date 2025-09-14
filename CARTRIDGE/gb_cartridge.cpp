#include "gb_cartridge.hpp"
#include "../UTILS/file_io.hpp"

void gb_cartridge::load_cartridge(const std::string& path)
{
    rawData = read_file_to_vector(path);

    if(rawData.size() == 0)
        return;

    std::cout<<"Loaded "<< path<<" with "<<rawData.size()<<" bytes."<<'\n';

    load_info();

    //print_info();
}

void gb_cartridge::load_info()
{
    info.title = std::string(rawData.begin() + cartridge_header::TITLE, 
                             rawData.begin() + cartridge_header::TITLE_END);

    info.manufacturer_code = std::string(rawData.begin() + cartridge_header::MANUFACTURER_CODE, 
                             rawData.begin() + cartridge_header::MANUFACTURER_CODE_END);
    
    info.cgb_flag = rawData[cartridge_header::CGB_FLAG];

    info.license_code = std::string(rawData.begin() + cartridge_header::NEW_LICENSE_CODE, 
                             rawData.begin() + cartridge_header::NEW_LICENSE_CODE_END);

    info.sgb_flag = rawData[cartridge_header::SGB_FLAG];

    info.cartridge_type = rawData[cartridge_header::CARTRIDGE_TYPE];

    info.rom_size = rawData[cartridge_header::ROM_SIZE];
    info.ram_size = rawData[cartridge_header::RAM_SIZE];
    info.destination_code = rawData[cartridge_header::DESTINATION_CODE];

    info.old_license_code = rawData[cartridge_header::OLD_LICENSE_CODE];
    info.mask_rom_version_number = rawData[cartridge_header::MASK_ROM_VERSION_NUMBER];

    auto validate_header_checksum = [this](std::vector<uint8_t>& rawData, uint8_t value)
    {
        uint8_t checksum = 0;
        for (uint16_t address = 0x0134; address <= 0x014C; address++) {
            checksum = checksum - rawData[address] - 1;
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

    info.header_checksum = rawData[cartridge_header::HEADER_CHECKSUM];
    validate_header_checksum(rawData, info.header_checksum);

    auto validate_global_checksum = [this](std::vector<uint8_t>& rawData, uint16_t value)
    {
        uint16_t checksum = 0;
        for (int address = 0; address<rawData.size(); address++) {
            if(address == 0x014E || address == 0x014F ) continue;
            checksum = checksum + rawData[address];
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

    uint8_t high = rawData[cartridge_header::GLOBAL_CHECKSUM];
    uint8_t low = rawData[cartridge_header::GLOBAL_CHECKSUM_END];
    info.global_checksum = (high << 8) | low;
    validate_global_checksum(rawData, info.global_checksum);    
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
    <<"0x"<<std::hex<<static_cast<int>(rawData[0x0100]) << ' '
    <<"0x"<<std::hex<<static_cast<int>(rawData[0x0101]) << ' '
    <<"0x"<<std::hex<<static_cast<int>(rawData[0x0102]) << ' '
    <<"0x"<<std::hex<<static_cast<int>(rawData[0x0103]) <<'\n';

    std::cout<<"Nintendo logo: "<<'\n';
    for(int i=0x104; i<0x133; i+=0x10)
    {
        for(int j=0; j<0x10; ++j)
        {
            std::cout<<"0x"<<std::hex<<static_cast<int>(rawData[i+j])<<'\t';
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

uint8_t gb_cartridge::read(const uint16_t& address)
{
    return rawData[address];
}

void gb_cartridge::write(const uint16_t& address, const uint8_t& data)
{
    rawData[address] = data;
}
