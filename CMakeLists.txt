cmake_minimum_required(VERSION 3.15)
project(GameBoyEmu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(UTILS)
add_subdirectory(MEMORY)
add_subdirectory(CARTRIDGE)
add_subdirectory(TIMER)
add_subdirectory(VIDEO)
add_subdirectory(JOYPAD)
add_subdirectory(BUS)
add_subdirectory(CPU)

add_subdirectory(GAMEBOY)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})
link_directories(${PNG_LIBRARY_DIRS})
add_definitions(${PNG_DEFINITIONS})
if(NOT PNG_FOUND)
    message(ERROR " PNG not found!")
endif(NOT PNG_FOUND)

find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

add_subdirectory(EMULATOR)

# Main executable
add_executable(gbemu main.cpp)

# Link CPU (which itself links MEMORY and CARTRIDGE)
target_link_libraries(gbemu PRIVATE GB_EMULATOR ${PNG_LIBRARIES} ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})
