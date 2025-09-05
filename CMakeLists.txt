cmake_minimum_required(VERSION 3.15)
project(GameBoyEmu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(googletest)

enable_testing()
add_executable(runTests tests/cpu_sharpsm83_test.cpp)
target_link_libraries(runTests gtest_main CPU)
add_test(NAME CPU_Tests COMMAND runTests)

# Add subdirectories
add_subdirectory(UTILS)
add_subdirectory(MEMORY)
add_subdirectory(CARTRIDGE)
add_subdirectory(BUS)
add_subdirectory(CPU)

add_subdirectory(GAMEBOY)

# Main executable
add_executable(gbemu main.cpp)

# Link CPU (which itself links MEMORY and CARTRIDGE)
target_link_libraries(gbemu PRIVATE GAMEBOY)
