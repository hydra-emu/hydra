#include "ppu.h"
#include <mutex>
#include <glad/glad.h>

namespace TKPEmu::Gameboy::Devices {
	PPU::PPU(Bus* bus, std::mutex* draw_mutex) : bus_(bus), mem_OAM_(), draw_mutex_(draw_mutex),
		LCDC(bus->GetReference(0xFF40)),
		STAT(bus->GetReference(0xFF41)),
		SCY(bus->GetReference(0xFF42)),
		SCX(bus->GetReference(0xFF43)),
		LY(bus->GetReference(0xFF44)),
		LYC(bus->GetReference(0xFF45)),
		WY(bus->GetReference(0xFF4A)),
		WX(bus->GetReference(0xFF4B)),
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
						std::lock_guard<std::mutex> lg(*draw_mutex_);
						draw_scanline();
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

	inline void PPU::draw_scanline() {
		if (LCDC & 0b1) {

		}
		if (LCDC & 0b10) {

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
				if (yy + y < 144)
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

	//inline void PPU::draw_tiles() {
	//	static const int color1[3] = { 160, 202, 72 };
	//	static const int color2[3] = { 70, 255, 0 };
	//	static const int color3[3] = { 50, 100, 200 };
	//	static const int color4[3] = { 255, 30, 110 };
	//	uint8_t wx_c = WX - 7;
	//	int base = (LCDC & 0b0001'0000) ? 0x8000 : 0x8800;
	//	bool unsig = base == 0x8800;
	//	bool win_enable = ((LCDC & 0b0010'0000) && (WY <= LY));
	//	uint16_t id_loc = 0;
	//	uint8_t pos_y = 0;
	//	id_loc = (LCDC & 0b0100'0000) ? 0x9C00 : 0x9800;
	//	if (win_enable) {
	//		pos_y = LY - WY;
	//	}
	//	else {
	//		pos_y = LY + SCY;
	//	}
	//	uint16_t tile_row = (((uint8_t)(pos_y / 8)) * 32);
	//	for (int i = 0; i < 160; i += 8) {
	//		uint8_t posx = i + SCX;
	//		if (win_enable && i >= WX) {
	//			posx = i - WX;
	//		}
	//		uint16_t tile_col = (posx / 8);
	//		uint16_t tileno = 0;
	//		uint16_t tile_addy = id_loc + tile_row + tile_col;
	//		if (unsig) {
	//			tileno = bus_->Read(tile_addy);
	//		}
	//		else {
	//			tileno = static_cast<int8_t>(bus_->Read(tile_addy));
	//		}
	//		uint16_t tile_loc = base;
	//		if (unsig) {
	//			base += tileno * 16;
	//		}
	//		else {
	//			base += (tileno + 128) * 16;
	//		}
	//		uint8_t line = (pos_y % 8);
	//		line *= 2;
	//		//uint8_t byte1 = bus_->Read(tile_loc + line);
	//		//uint8_t byte2 = bus_->Read(tile_loc + line + 1);
	//		//int colorbit = posx % 8;
	//		//colorbit -= 7;
	//		//colorbit *= -1;
	//		//int colornum = colorbit & byte2;
	//		//colornum <<= 1;
	//		//colornum |= colorbit & byte1;
	//		//switch (colornum) {
	//		//case 0: set_pixel(i, LY % 144, color1[0], color1[1], color1[2]); break;
	//		//case 1: set_pixel(i, LY % 144, color2[0], color2[1], color2[2]); break;
	//		//case 2: set_pixel(i, LY % 144, color3[0], color3[1], color3[2]); break;
	//		//case 3: set_pixel(i, LY % 144, color4[0], color4[1], color4[2]); break;
	//		//}
	//		draw_tile(tile_loc + line, i, LY % 144);
	//	}
	//}
}