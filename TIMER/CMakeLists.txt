add_library(TIMER gb_timer.cpp)

target_include_directories(TIMER PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TIMER PUBLIC GB_BUS)
