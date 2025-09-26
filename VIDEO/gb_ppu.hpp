#ifndef _gb_ppu_
#define _gb_ppu_

#include <cstdint>
#include <vector>
#include <memory>

#include "../BUS/gb_bus.hpp"
#include "gb_color.hpp"
#include "frame_buffer.hpp"

#define VIDEO_RAM_MAX_MEMORY_SIZE 0x2000

#define CLOCKS_PER_OAM_SEARCH 80
#define CLOCKS_PER_PIXEL_TRANSFER 172
#define CLOCKS_PER_HBLANK 204
#define CLOCKS_PER_VBLANK 456

const uint GAMEBOY_WIDTH = 160;
const uint GAMEBOY_HEIGHT = 144;

enum class ppu_mode
{
    HBLANK = 0,
    VBLANK = 1,
    OAM_SEARCH = 2,
    PIXEL_TRANSFER = 3
};

struct object_attribute
{
    uint8_t y_position;
    uint8_t x_position;
    uint8_t tile_index;
    uint8_t attributes;
};

struct gb_bus;

class gb_ppu
{
private:
    // 8 kb of video RAM
    uint8_t video_ram[VIDEO_RAM_MAX_MEMORY_SIZE];

    ppu_mode mode = ppu_mode::OAM_SEARCH;

    long int cycle_count = 0;

    std::shared_ptr<gb_bus> bus;

    std::vector<object_attribute> visible_objects;
    uint8_t scanline_color_index[160];

    frame_buffer buffer;

    // PPU registers
    uint8_t LCDC; // LCD Control (0xFF40)
    uint8_t STAT; // LCDC Status (0xFF41)

    uint8_t SCY; // Scroll Y (0xFF42)
    uint8_t SCX; // Scroll X (0xFF43)

    uint8_t LY; // LCD Y-Coordinate (0xFF44), current line
    uint8_t LYC; // LY Compare (0xFF45)

    uint8_t BGP; // BG palette data (0xFF47)
    uint8_t OBP0; // obj palette register 0
    uint8_t OBP1; // obj palette register 1

    uint8_t WX; // Window X position (0xFF4B)
    uint8_t WY; // Window Y position (0xFF4A)

    void select_objects_for_line();
    void render_scanline();
    void render_background_line();
    void render_window_line();
    void render_sprite_line();

    void update_STAT();
public:
    gb_ppu();
    ~gb_ppu();

    void tick(int cycles);

    uint8_t read(const uint16_t& address);
    void write(const uint16_t& address, const uint8_t& data);

    void set_bus(const std::shared_ptr<gb_bus>& b);

    const frame_buffer& get_frame_buffer();

    uint8_t read_LCDC();
    void write_LCDC(uint8_t data);

    uint8_t read_STAT();
    void write_STAT(uint8_t data);

    uint8_t read_SCY();
    void write_SCY(uint8_t data);

    uint8_t read_SCX();
    void write_SCX(uint8_t data);

    uint8_t read_LY();
    void write_LY(uint8_t data);

    uint8_t read_LYC();
    void write_LYC(uint8_t data);

    uint8_t read_BGP();
    void write_BGP(uint8_t data);

    uint8_t read_OPB0();
    void write_OBP0(uint8_t data);

    uint8_t read_OPB1();
    void write_OBP1(uint8_t data);

    void dump_vram(const std::string &filename);
};

#endif