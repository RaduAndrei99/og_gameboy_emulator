add_library(GB_JOYPAD gb_joypad.cpp)

target_include_directories(GB_JOYPAD PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GB_JOYPAD PUBLIC GB_BUS)
