#include <iostream>
#include "gb_ppu.hpp" 

#include <fstream>
#include <iomanip>

gb_ppu::gb_ppu() : buffer(GAMEBOY_WIDTH, GAMEBOY_HEIGHT)
{
    LCDC = 0x91;
    STAT = 0x85;

    SCY = 0x00;
    SCX = 0x00;

    LY = 0x00; 
    LYC = 0x00;

    BGP = 0xFC;
}
gb_ppu::~gb_ppu() = default;

void gb_ppu::tick(int cycles)
{
    cycle_count += cycles;
    
    switch(mode)
    {
        case ppu_mode::OAM_SEARCH:
        {
            select_objects_for_line();

            if(cycle_count >= CLOCKS_PER_OAM_SEARCH)
            {
                cycle_count -= CLOCKS_PER_OAM_SEARCH;

                mode = ppu_mode::PIXEL_TRANSFER;

                update_STAT();
            }

            break;
        }
        case ppu_mode::PIXEL_TRANSFER:
        {
            // Transfer the pixel data to the LCD driver
            if(cycle_count >= CLOCKS_PER_PIXEL_TRANSFER) // 172 cycles for pixel transfer
            {
                cycle_count -= CLOCKS_PER_PIXEL_TRANSFER;

                mode = ppu_mode::HBLANK;

                update_STAT();
                
                render_scanline();
            }
            break;
        }
        case ppu_mode::HBLANK:
        {
            if(cycle_count >= CLOCKS_PER_HBLANK)
            {
                cycle_count -= CLOCKS_PER_HBLANK;

                LY++;

                if(LY == 144)
                {
                    mode = ppu_mode::VBLANK;
                    
                    // fire VBLANK interrupt
                    bus->cpu->set_IF(bus->cpu->get_IF() | 0x01);

                    update_STAT();
                }
                else
                {
                    mode = ppu_mode::OAM_SEARCH;

                    update_STAT();
                }
            }
            
            break;
        }
        case ppu_mode::VBLANK:
        {
            if(cycle_count >= CLOCKS_PER_VBLANK)
            {
                cycle_count -= CLOCKS_PER_VBLANK;

                LY++;

                if(LY == 154)
                {
                    LY = 0;
                    mode = ppu_mode::OAM_SEARCH;
                    update_STAT();
                }

            }

            break;
        }
    }
}

uint8_t gb_ppu::read(const uint16_t& address)
{
    return video_ram[address];
}
void gb_ppu::write(const uint16_t& address, const uint8_t& data)
{
    video_ram[address] = data;
}
void gb_ppu::select_objects_for_line() 
{
    visible_objects.clear();

    int spriteHeight = (LCDC & 0x04) ? 16 : 8;

    for (int i = 0; i < 40; ++i) 
    {
        object_attribute obj;
        obj.y_position = bus->bus_read(0xFE00 + i*4 + 0);
        obj.x_position = bus->bus_read(0xFE00 + i*4 + 1);
        obj.tile_index = bus->bus_read(0xFE00 + i*4 + 2);
        obj.attributes = bus->bus_read(0xFE00 + i*4 + 3);

        int top = obj.y_position - 16;

        if (LY >= top && LY < top + spriteHeight) 
        {
            visible_objects.push_back(obj);

            if (visible_objects.size() == 10) break;
        }
    }
}

void gb_ppu::render_scanline()
{
    // display disabled
    if(!(LCDC & 0x80))
    {
        return;
    }

    //std::cout<<"Current LY: "<<(int)LY<<'\n';

    // 1. Backgound
    if(LCDC & 0x01)
    {
        render_background_line();
    }

    // 2. Sprites
    if(LCDC & 0x02)
    {
        render_sprite_line();
    }
    
}

void gb_ppu::render_background_line()
{
    // Background tile map base address (0x9800 or 0x9C00)
    uint16_t bg_base_pointer = (LCDC & 0x08) ? 0x9C00 : 0x9800;
    // Tile data base address (0x8000 unsigned, or 0x8800 signed)
    uint16_t tile_data_base = (LCDC & 0x10) ? 0x8000 : 0x8800;
    uint8_t palette = bus->bus_read(0xFF47);

    uint screen_y = LY;

    for (uint screen_x = 0; screen_x < 160; ++screen_x)
    {
        // Screen position adjusted by scroll
        uint scrolled_x = screen_x + SCX;
        uint scrolled_y = screen_y + SCY;

        // Tile coordinates in the map
        // uint tile_row = scrolled_y / 8;
        // uint tile_col = scrolled_x / 8;

        uint tile_row = (scrolled_y % 256) / 8;
        uint tile_col = (scrolled_x % 256) / 8;

        // Fetch tile index from the tile map
        uint16_t tile_map_address = bg_base_pointer + (tile_row * 32) + tile_col;
        uint8_t tile_index = bus->bus_read(tile_map_address);

        // Handle signed indexing in 0x8800 mode
        uint real_index = (LCDC & 0x10) ? tile_index : ((int8_t)tile_index + 128);

        // Each tile = 16 bytes (2 bytes per row, 8 rows)
        uint16_t tile_addr = tile_data_base + (real_index * 16);

        // Select row within tile
        uint8_t tile_line = scrolled_y % 8;

        uint8_t low  = bus->bus_read(tile_addr + tile_line * 2);
        uint8_t high = bus->bus_read(tile_addr + tile_line * 2 + 1);

        // Bit index (MSB = leftmost pixel)
        int bit = 7 - (scrolled_x % 8);
        uint8_t pixel_value = ((high >> bit) & 1) << 1 | ((low >> bit) & 1);

        // Store raw color index (0–3) for sprite blending
        scanline_color_index[screen_x] = pixel_value;

        // Map pixel value through background palette (FF47)
        uint8_t shade = (palette >> (pixel_value * 2)) & 0x03;

        gb_color color = get_color(shade);

        buffer.set_pixel(screen_x, screen_y, color);
    }
}

void gb_ppu::render_window_line()
{

}
void gb_ppu::render_sprite_line()
{
    int height = (LCDC & 0x04) ? 16 : 8;

    int screen_y = LY;

    for(int i=0; i<visible_objects.size(); ++i)
    {
        const object_attribute& sprite = visible_objects[i];

        // TODO: more complicated
        if (sprite.y_position == 0 || sprite.y_position >= 160) { continue; }
        if (sprite.x_position == 0 || sprite.x_position >= 168) { continue; }

        // Y position in screen space: OAM.y - 16
        int y_pos = sprite.y_position - 16;
        int y_in_sprite = LY - y_pos;

        // Apply vertical flip
        bool y_flip = sprite.attributes & 0x10;
        if (y_flip)
        {
            y_in_sprite = height - 1 - y_in_sprite;
        }
        
        // Handle 8x16 mode: force even tile index, select top/bottom tile
        uint8_t tile_index = sprite.tile_index;
        if (height == 16)
        {
            tile_index &= 0xFE; // hardware ignores LSB
            if (y_in_sprite >= 8) 
            {
                tile_index |= 0x01; // bottom tile
                y_in_sprite -= 8;
            }
        }

        // Fetch the 2 bytes for this row of tile data

        uint16_t tile_addr = 0x8000 + (tile_index * 16);
        uint8_t low  = bus->bus_read(tile_addr + y_in_sprite * 2);
        uint8_t high = bus->bus_read(tile_addr + y_in_sprite * 2 + 1);

        for (int x = 0; x < 8; x++) 
        {
            // OAM.x - 8 is the true position
            int screen_x = sprite.x_position - 8 + x;

            if (screen_x < 0 || screen_x >= 160) continue;

            bool x_flip = sprite.attributes & 0x20;
            
            int bit = x_flip ? x : (7 - x);
            uint8_t pixel_value = ((high >> bit) & 1) << 1 | ((low >> bit) & 1);

            // transparent pixel
            if (pixel_value == 0) continue;

            bool priority = sprite.attributes & 0x80;
            bool bg_non_zero = (scanline_color_index[screen_x] != 0);

            if (priority && bg_non_zero) continue; // behind BG
            
            bool pallete_bit = sprite.attributes & 0x10;
            uint8_t palette = pallete_bit ? bus->bus_read(0xFF49) : bus->bus_read(0xFF48);

            uint8_t shade = (palette >> (pixel_value * 2)) & 0x03;

            gb_color color = get_color(shade);
            buffer.set_pixel(screen_x, screen_y, color);
        }
    }
}

void gb_ppu::set_bus(const std::shared_ptr<gb_bus>& b) 
{
    bus = b; 
}
void gb_ppu::update_STAT() 
{
    static ppu_mode prev_mode = ppu_mode::HBLANK;
    static bool prev_lyc_match = false;

    // Update mode bits
    STAT = (STAT & ~0x03) | ((uint8_t)mode & 0x03);

    // Coincidence flag
    bool lyc_match = (LY == LYC);
    if (lyc_match) 
        STAT |= 0x04;
    else 
        STAT &= ~0x04;

    bool request = false;

    // Edge-triggered mode interrupts
    if (mode != prev_mode) 
    {
        if (mode == ppu_mode::HBLANK && (STAT & 0x08)) request = true;
        if (mode == ppu_mode::VBLANK && (STAT & 0x10)) request = true;
        if (mode == ppu_mode::OAM_SEARCH && (STAT & 0x20)) request = true;
    }

    // Edge-triggered LYC=LY interrupt
    if (lyc_match && !prev_lyc_match && (STAT & 0x40)) 
    {
        request = true;
    }

    if (request) 
    {
        bus->cpu->set_IF(bus->cpu->get_IF() | 0x02);
    }

    prev_mode = mode;
    prev_lyc_match = lyc_match;
}
const frame_buffer& gb_ppu::get_frame_buffer()
{
    return buffer;
}
uint8_t gb_ppu::read_LCDC()
{
    return LCDC;
}
void gb_ppu::write_LCDC(uint8_t data)
{
    LCDC = data;
}
uint8_t gb_ppu::read_STAT()
{
    return STAT;
}
void gb_ppu::write_STAT(uint8_t data)
{
    STAT = data;
}
uint8_t gb_ppu::read_SCY()
{
    return SCY;
}
void gb_ppu::write_SCY(uint8_t data)
{
    SCY = data;
}
uint8_t gb_ppu::read_SCX()
{
    return SCX;
}
void gb_ppu::write_SCX(uint8_t data)
{
    SCX = data;
}
uint8_t gb_ppu::read_LY()
{
    return LY;
}
void gb_ppu::write_LY(uint8_t data)
{
    LY = data;
}
uint8_t gb_ppu::read_LYC()
{
    return LYC;
}
void gb_ppu::write_LYC(uint8_t data)
{
    LYC = data;
}

uint8_t gb_ppu::read_BGP()
{
    return BGP;
}
void gb_ppu::write_BGP(uint8_t data)
{
    BGP = data;
}

uint8_t gb_ppu::read_OPB0()
{
    return OBP0;
}
void gb_ppu::write_OBP0(uint8_t data)
{
    OBP0 = data;
}
uint8_t gb_ppu::read_OPB1()
{
    return OBP1;
}
void gb_ppu::write_OBP1(uint8_t data)
{
    OBP1 = data;
}
void gb_ppu::dump_vram(const std::string &filename) 
{
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open VRAM dump file: " << filename << "\n";
        return;
    }
    int size = 0;

    // VRAM is 0x2000 bytes (8 KB) from 0x8000–0x9FFF
    for (uint16_t addr = 0x8000; addr <= 0x9FFF; addr++) {
        uint8_t value = bus->bus_read(addr);
        out.put(static_cast<char>(value));
        size++;
    }

    out.close();
    std::cout << "VRAM dumped to " << filename << " (" << size << " bytes)\n";
}