#include "gb_emulator.hpp"

gb_emulator::gb_emulator() 
{
    sAppName = "OG Gameboy Emulator";
}

gb_emulator::~gb_emulator()
{

}

void gb_emulator::cpu_task()
{
    gb.run();
}

bool gb_emulator::OnUserCreate() 
{
    // Called once at the start, so create things here
    //gb.load_cartridge("../ROMs/Tetris.gb");
    //gb.load_cartridge("../ROMs/Pokemon-Red.gb");
    //gb.load_cartridge("../ROMs/super_mario_land.gb");
    //gb.load_cartridge("../ROMs/donkey_kong.gb");
    //gb.load_cartridge("../ROMs/dr_mario.gb");
    //gb.load_cartridge("../ROMs/v-rally.gb");

    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/01-special.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/02-interrupts.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/03-op-sp,hl.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/04-op-r,imm.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/05-op-rp.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/06-ld-r,r.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/08-misc-instrs.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/09-op-r,r.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/10-bit-ops.gb");
    //gb.load_cartridge("../ROMs/test/cpu_instrs/individual/11-op-a,(hl).gb");

    //gb.load_cartridge("../ROMs/test/cpu_instrs/cpu_instrs.gb");
    //gb.load_cartridge("../ROMs/test/instr_timing/instr_timing.gb");

    //gb.load_cartridge("../ROMs/test/interrupt_time/interrupt_time.gb");

    //gb.load_cartridge("../ROMs/test/mem_timing/mem_timing.gb");

    //gb.load_cartridge("../ROMs/test/mem_timing-2/mem_timing.gb");
    //gb.load_cartridge("../ROMs/test/mem_timing-2/rom_singles/01-read_timing.gb");
    //gb.load_cartridge("../ROMs/test/mem_timing-2/rom_singles/02-write_timing.gb");
    //gb.load_cartridge("../ROMs/test/mem_timing-2/rom_singles/03-modify_timing.gb");

    //gb.load_cartridge("../ROMs/test/double-halt-cancel-gbconly.gb");

    //gb.load_cartridge("../ROMs/test/halt_bug.gb");
    //gb.load_cartridge("../ROMs/hello-world.gb");
    //gb.load_cartridge("../ROMs/opus5.gb");


    gb.get_cpu()->reset();
    
    return true;
}

bool gb_emulator::OnUserUpdate(float fElapsedTime)
{
    int cycles_this_frame = 0;

    const double FRAME_TIME_MS = 1000.0 / (59.73); // ~16.74 ms

    static auto lastFrameTime = std::chrono::high_resolution_clock::now();

    while (cycles_this_frame < CYCLES_PER_FRAME) 
    {
        int cycles = gb.get_cpu()->tick();

        if(cycles == -1)
        {
            break;
        }   

        cycles_this_frame += cycles;
    }

    const frame_buffer& buffer = gb.get_video()->get_frame_buffer();

    auto gb_colot_to_pixel = [](const gb_color& c) -> olc::Pixel
    {
        // switch(c)
        // {
        //     case gb_color::Black: return olc::BLACK;
        //     case gb_color::DarkGray: return olc::DARK_GREY;
        //     case gb_color::LighGray: return olc::GREY;
        //     case gb_color::White: return olc::WHITE;
        //     default: return olc::BLACK; 
        // }
        // switch(c)
        // {
        //     case gb_color::Black: return olc::Pixel(15,56,15);
        //     case gb_color::DarkGray: return olc::Pixel(48,98,48);
        //     case gb_color::LighGray: return olc::Pixel(139,172,15);
        //     case gb_color::White: return olc::Pixel(155,188,15);
        //     default: return olc::BLACK; 
        // }
        switch(c)
        {
            case gb_color::Black: return olc::Pixel(12,24,17);
            case gb_color::DarkGray: return olc::Pixel(66,96,61);
            case gb_color::LighGray: return olc::Pixel(139,164,106);
            case gb_color::White: return olc::Pixel(202,221,149);
            default: return olc::BLACK; 
        }
    };

    for(int x=0; x<GAMEBOY_WIDTH; ++x)
    {
        for(int y=0; y<GAMEBOY_HEIGHT; ++y)
        {
            Draw(olc::vi2d(x, y), gb_colot_to_pixel(buffer.get_pixel(x, y)) );
        }
    }

    if(GetKey(olc::Key::V).bReleased)
    {
        gb.get_video()->dump_vram("../vram.bin");
    }
    if(GetKey(olc::Key::R).bReleased)
    {
        OnUserCreate();
    }
    gb.get_joypad()->set_left(GetKey(olc::Key::A).bHeld);
    gb.get_joypad()->set_right(GetKey(olc::Key::D).bHeld);
    gb.get_joypad()->set_up(GetKey(olc::Key::W).bHeld);
    gb.get_joypad()->set_down(GetKey(olc::Key::S).bHeld);

    gb.get_joypad()->set_b(GetKey(olc::Key::N).bHeld);
    gb.get_joypad()->set_a(GetKey(olc::Key::M).bHeld);
    gb.get_joypad()->set_select(GetKey(olc::Key::SPACE).bHeld);
    gb.get_joypad()->set_start(GetKey(olc::Key::ENTER).bHeld);

    gb.get_joypad()->update();

    // Frame pacing
    // auto now = std::chrono::high_resolution_clock::now();
    // auto elapsed = std::chrono::duration<double, std::milli>(now - lastFrameTime).count();

    // if (elapsed < FRAME_TIME_MS) {
    //     std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(FRAME_TIME_MS - elapsed));
    // }

    // lastFrameTime = std::chrono::high_resolution_clock::now();

    return true;
}

gameboy& gb_emulator::get_gb()
{
    return gb;
}
