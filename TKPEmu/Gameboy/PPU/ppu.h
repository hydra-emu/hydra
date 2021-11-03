#pragma once
#ifndef TKP_GB_PPU_H
#define TKP_GB_PPU_H
#include "../Bus/bus.h"
#include "../../Tools/TKPImage.h"
#include <mutex>
namespace TKPEmu::Gameboy::Devices {
	constexpr int FRAME_CYCLES = 70224;
	class PPU {
	private:
		using IFInterrupt = Bus::IFInterrupt;
		using LCDCFlag = Bus::LCDCFlag;
		using STATFlag = Bus::STATFlag;
		using TKPImage = TKPEmu::Tools::TKPImage;
	public:
		PPU(Bus* bus, std::mutex* draw_mutex);
		void Update(uint8_t tTemp);
		void Reset();
		uint8_t* GetScreenData();
	private:
		Bus* bus_;
		std::array<uint8_t, 0x9F> mem_OAM_;
		std::array<uint8_t, 4 * 160 * 144> screen_{};

		// PPU register pointers
		uint8_t& LCDC;
		uint8_t& STAT;
		uint8_t& LYC;
		uint8_t& LY;
		uint8_t& IF;

		int clock_ = 0;
		int clock_target_ = 0;
		int next_stat_mode = 0;
		std::mutex* draw_mutex_ = nullptr;
		inline void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
		int set_mode(int mode);
		int get_mode();
		int update_lyc();
		inline void update_cache();
		inline void draw_tile(int addr, int xx, int yy);
	};
}
#endif