#ifndef _GB_EMULATOR_
#define _GB_EMULATOR_

#include "../olcPixelGameEngine/olcPixelGameEngine.h"

#include "../GAMEBOY/gameboy.hpp"

const int CYCLES_PER_FRAME = 70224; // T-cycles

class gb_emulator : public olc::PixelGameEngine
{
private:
    void cpu_task();

    gameboy gb;
public:
	gb_emulator();
    ~gb_emulator();

	bool OnUserCreate() override;

	bool OnUserUpdate(float fElapsedTime) override;

    gameboy& get_gb();
};

#endif