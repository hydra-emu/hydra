#pragma once
#ifndef TKP_GB_PPU_H
#define TKP_GB_PPU_H
#include "../Bus/bus.h"
#include "../../Tools/TKPImage.h"
namespace TKPEmu::Gameboy::Devices {
	constexpr int FRAME_CYCLES = 70224;
	class PPU {
	private:
		using IFInterrupt = Bus::IFInterrupt;
		using LCDCFlag = Bus::LCDCFlag;
		using STATFlag = Bus::STATFlag;
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
		uint8_t& LCDC;
		uint8_t& STAT;
		uint8_t& LYC;
		uint8_t& LY;

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