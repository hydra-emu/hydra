#include "ppu.h"
#include <mutex>
#include <glad/glad.h>

namespace TKPEmu::Gameboy::Devices {
	PPU::PPU(Bus* bus, std::mutex* draw_mutex) : bus_(bus), mem_OAM_(), draw_mutex_(draw_mutex),
		LCDC(bus->GetReference(0xFF40)),
		STAT(bus->GetReference(0xFF41)),
		LY(bus->GetReference(0xFF44)),
		LYC(bus->GetReference(0xFF45)),
		IF(bus->GetReference(0xFF0F))
	{
		screen_.fill(0xFF);
	}

	void PPU::Update(uint8_t tTemp) {
		IF &= 0b11111110;
		clock_ += tTemp;
		if (LCDC & LCDCFlag::LCD_ENABLE) {
			if (clock_ >= clock_target_) {
				if (LY == 153) {
					next_stat_mode = 2;
					LY = -1;
					clock_ %= FRAME_CYCLES;
					clock_target_ = FRAME_CYCLES;
				}
				IF |= set_mode(next_stat_mode);
				
				if (int mode = get_mode(); mode == 2) {
					clock_target_ += 80;
					next_stat_mode = 3;
					LY += 1;
					IF |= update_lyc();
				}
				else if (mode == 3) {
					clock_target_ += 170;
					next_stat_mode = 0;
				}
				else if (mode == 0) {
					clock_target_ += 206;
					if (LY <= 143) {
						next_stat_mode = 2;
					}
					else {
						next_stat_mode = 1;
					}
				}
				else if (mode == 1) {
					clock_target_ += 456;
					next_stat_mode = 1;
					if (LY == 144) {
						IF |= IFInterrupt::VBLANK;
						// TODO: frame is done here, needs draw = true
						std::lock_guard<std::mutex> lg(*draw_mutex_);
						update_cache();
					}
					LY += 1;
					IF |= update_lyc();
				}
			}
		}
		else {
			if (clock_ >= FRAME_CYCLES) {
				clock_ %= FRAME_CYCLES;
			}
		}
	}
	void PPU::Reset() {
		LY = 0x91;
		STAT = 0b1000'0000;
		next_stat_mode = 2;
		clock_ = 0;
		clock_target_ = 0;
	}
	
	uint8_t* PPU::GetScreenData() {
		return screen_.data();
	}

	int PPU::set_mode(int mode) {
		if (get_mode() == mode) {
			return 0;
		}
		STAT &= 0b1111'1100;
		STAT |= mode;
		if (mode != 3 && STAT & (1 << (mode + 3))) {
			return IFInterrupt::LCDSTAT;
		}
		return 0;
	}
	int PPU::get_mode() {
		return STAT & STATFlag::MODE;
	}

	inline void PPU::set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
		static const int gbwidth = 160;
		static const int gbheight = 144;
		int calc = x * 4 + (y * gbwidth * 4);
		screen_[calc++] = r;
		screen_[calc++] = g;
		screen_[calc++] = b;
		screen_[calc] = 255;
	}

	int PPU::update_lyc() {
		if (LYC == LY) {
			STAT |= STATFlag::COINCIDENCE;
			if (STAT & STATFlag::COINC_INTER)
				return IFInterrupt::LCDSTAT;
		}
		else {
			STAT &= 0b1111'1011;
		}
		return 0;
	}

	void PPU::update_cache() {
		int x = 0;
		int y = 0;
		for (int i = 0x8000; i < 0x8FFF; i += 0x10) {
			draw_tile(i, x, y);
			x += 8;
			if (x == 128) {
				x = 0;
				y += 8;
			}
		}
	}
	inline void PPU::draw_tile(int addr, int xx, int yy) {
		// TODO: cache the changes from getting the writes in bus
		// TODO: get colors from display
		static const int color1[3] = { 160, 202, 72 };
		static const int color2[3] = { 70, 255, 0 };
		static const int color3[3] = { 50, 100, 200 };
		static const int color4[3] = { 255, 30, 110 };
		int x = 0;
		int y = 0;
		for (int i = 0; i < 16; i += 2) {
			auto byte1 = bus_->Read(addr + i);
			auto byte2 = bus_->Read(addr + i + 1);
			for (int b = 0; b < 8; b++) {
				int code = ((byte1 >> (7 - b)) & 0x1) | (((byte2 >> (7 - b)) & 0x1) << 1);
				switch (code) {
				case 0: set_pixel(xx + x, yy + y, color1[0], color1[1], color1[2]); break;
				case 1: set_pixel(xx + x, yy + y, color2[0], color2[1], color2[2]); break;
				case 2: set_pixel(xx + x, yy + y, color3[0], color3[1], color3[2]); break;
				case 3: set_pixel(xx + x, yy + y, color4[0], color4[1], color4[2]); break;
				}
				x++;
			}
			y++;
			x = 0;
		}
	}
}