#pragma once
#ifndef TKP_GB_PPU_H
#define TKP_GB_PPU_H
#include "../Bus/bus.h"
#include "../../Tools/TKPImage.h"
#include <mutex>
#include <queue>
#include <array>
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
		struct Pixel {
			uint8_t color : 2;
			uint8_t palette : 1;
			// TODO: CGB Sprite priority
			uint8_t bg_prio : 1;
		};
		Bus* bus_;
		std::array<uint8_t, 0x9F> mem_OAM_;
		std::array<uint8_t, 4 * 160 * 144> screen_{};
		std::queue<Pixel> bg_fifo_{};
		std::queue<Pixel> sprite_fifo_{};

		// PPU register pointers
		uint8_t& LCDC, &STAT, &LYC, &LY, &IF, &SCY, &SCX, &WY, &WX;

		int clock_ = 0;
		int clock_target_ = 0;
		int next_stat_mode = 0;
		std::mutex* draw_mutex_ = nullptr;
		inline void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
		int set_mode(int mode);
		int get_mode();
		int update_lyc();
		inline void draw_scanline();
		inline void draw_tile(int addr, int xx, int yy);
	};
}
#endif