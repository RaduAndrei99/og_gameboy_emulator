add_library(VIDEO gb_ppu.cpp)
add_library(COLOR gb_color.cpp)
add_library(FRAME_BUFFER frame_buffer.cpp)

target_include_directories(VIDEO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(COLOR PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(FRAME_BUFFER PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(VIDEO PUBLIC GB_BUS COLOR FRAME_BUFFER)
