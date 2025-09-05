add_library(CPU cpu_sharpsm83.cpp)

target_include_directories(CPU PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(CPU PUBLIC GB_BUS CARTRIDGE)
