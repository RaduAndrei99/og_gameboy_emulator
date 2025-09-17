add_library(GAMEBOY gameboy.cpp)

target_include_directories(CARTRIDGE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(CPU PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GAMEBOY PUBLIC CARTRIDGE CPU GB_BUS VIDEO)
