#include<fstream>
#include "file_io.hpp"

std::vector<uint8_t> read_file_to_vector(const std::string& file_name) 
{
    // Open the file in binary mode at the end (to get size)
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    
    if (!file) {
        std::cout<<"failed to open file: " + file_name<<'\n';

        return std::vector<uint8_t>();
    }

    // Get file size
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Allocate vector of correct size
    std::vector<uint8_t> buffer(size);

    // Read file data into buffer
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cout<<"failed to read file: " + file_name<<'\n';
    }

    return buffer;
}