add_library(CARTRIDGE gb_cartridge.cpp)

target_include_directories(CARTRIDGE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(CARTRIDGE PUBLIC FILE_IO)
