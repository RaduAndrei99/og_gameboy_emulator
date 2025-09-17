#include "frame_buffer.hpp"

frame_buffer::frame_buffer(int width, int height) :
    width(width),
    height(height),
    buffer(width*height, gb_color::White)
{
}

void frame_buffer::set_pixel(int x, int y, gb_color color) 
{
    buffer[pixel_index(x, y)] = color;
}

gb_color frame_buffer::get_pixel(int x, int y) const{ return buffer.at(pixel_index(x, y)); }

int frame_buffer::pixel_index(int x, int y) const { return (y * width) + x; }

void frame_buffer::reset() 
{
    for (uint i = 0; i < width * height; i++) 
    {
        buffer[i] = gb_color::White;
    }
}