add_library(GB_BUS gb_bus.cpp)

target_include_directories(GB_BUS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GB_BUS PUBLIC MEMORY CARTRIDGE CPU TIMER VIDEO)
