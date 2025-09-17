add_library(GB_EMULATOR gb_emulator.cpp)

target_include_directories(GB_EMULATOR PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GB_EMULATOR PUBLIC GAMEBOY)
