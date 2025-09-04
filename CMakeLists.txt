cmake_minimum_required(VERSION 3.15)
project(GameBoyEmu)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(CPU)

# Main executable
add_executable(gbemu main.cpp)

# Link CPU (which itself links MEMORY and CARTRIDGE)
target_link_libraries(gbemu PRIVATE CPU)
