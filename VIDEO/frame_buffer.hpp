#ifndef _FRAME_BUFFER_
#define _FRAME_BUFFER_

#include <cstdint>
#include <vector>

#include "gb_color.hpp"

class frame_buffer {
public:
    frame_buffer(int width, int height);

    void set_pixel(int x, int y, gb_color color) ;
    gb_color get_pixel(int x, int y) const;

    void reset();
private:
    int width;
    int height;

    int pixel_index(int x, int y) const;

    std::vector<gb_color> buffer;
};

#endif