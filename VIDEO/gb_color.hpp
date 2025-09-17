#ifndef _GB_COLOR_
#define _GB_COLOR_

#include <cstdint>

enum class gb_color {
    White, // White
    LighGray, // Light gray
    DarkGray, // Dark gray
    Black, // Black
};


gb_color get_color(uint8_t pixel);


#endif