#ifndef _FILE_IO__
#define _FILE_IO__

#include <iostream>
#include <vector>
#include <cstdint>

std::vector<uint8_t> read_file_to_vector(const std::string& file_name);

#endif
