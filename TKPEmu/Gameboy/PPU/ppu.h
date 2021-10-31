#pragma once
#ifndef TKP_GB_PPU_H
#define TKP_GB_PPU_H
#include "../Bus/bus.h"
#include "../../Tools/TKPImage.h"
namespace TKPEmu::Gameboy::Devices {
	constexpr int FRAME_CYCLES = 70224;
	class PPU {
	private:
		using TKPImage = TKPEmu::Tools::TKPImage;
	public:
		PPU(Bus* bus);
		bool NeedsDraw();
		void Update(uint8_t tTemp);
		void Reset();
		// Lock the mutex from the emulator before calling this function
		void Draw(TKPImage* screen);
	private:
		Bus* bus_;
		std::array<uint8_t, 0x9F> mem_OAM_;

		// PPU register pointers
		uint8_t* LCDC_ = nullptr;
		uint8_t* STAT_ = nullptr;
		uint8_t* IF_   = nullptr;
		uint8_t* LYC_  = nullptr;
		uint8_t* LY_   = nullptr;

		enum LCDCFlag {
			BG_ENABLE   = 1 << 0,
			OBJ_ENABLE  = 1 << 1,
			OBJ_SIZE    = 1 << 2,
			BG_TILEMAP  = 1 << 3,
			BG_TILES    = 1 << 4,
			WND_ENABLE  = 1 << 5,
			WND_TILEMAP = 1 << 6,
			LCD_ENABLE  = 1 << 7
		};

		enum STATFlag {
			MODE = 0b11,
			COINCIDENCE = 1 << 2,
			MODE0_INTER = 1 << 3,
			MODE1_INTER = 1 << 4,
			MODE2_INTER = 1 << 5,
			COINC_INTER = 1 << 6
		};

		enum IFInterrupt {
			VBLANK  = 1 << 0,
			LCDSTAT = 1 << 1,
			TIMER   = 1 << 2,
			SERIAL  = 1 << 3,
			JOYPAD  = 1 << 4
		};

		int clock_ = 0;
		int clock_target_ = 0;
		int next_stat_mode = 0;
		bool needs_draw = false;
		// TODO: remove test_draw function
		void test_draw(TKPImage*);
		int set_mode(int mode);
		int get_mode();
		int update_lyc();
	};
}
#endif