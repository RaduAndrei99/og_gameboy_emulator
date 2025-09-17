#include <cstdlib>
#include <iostream>

#include "gb_color.hpp"

gb_color get_color(uint8_t pixel)
{
    switch(pixel)
    {
        case 0: return gb_color::White;
        case 1: return gb_color::LighGray;
        case 2: return gb_color::DarkGray;
        case 3: return gb_color::Black;

        default: 
        {
            std::cout<<"Wrong pixel value: "<<pixel<<'\n';
            exit(-1);
        }
    }
}