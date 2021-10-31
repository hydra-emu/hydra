#include "ppu.h"

namespace TKPEmu::Gameboy::Devices {
	PPU::PPU(Bus* bus) : bus_(bus), mem_OAM_() {
		// TODO: Use references for this instead
		LCDC_ = bus_->GetPointer(0xFF40);
		STAT_ = bus_->GetPointer(0xFF41);
		LY_ = bus_->GetPointer(0xFF44);
		LYC_ = bus_->GetPointer(0xFF45);
		IF_ = bus_->GetPointer(0xFF0F);
	}
	bool PPU::NeedsDraw(){
		return needs_draw;
	}
	void PPU::Update(uint8_t tTemp) {
		*IF_ = 0;
		clock_ += tTemp;
		if (*LCDC_ & LCDCFlag::LCD_ENABLE) {
			if (clock_ >= clock_target_) {
				if (*LY_ == 153) {
					next_stat_mode = 2;
					*LY_ = -1;
					clock_ %= FRAME_CYCLES;
					clock_target_ = FRAME_CYCLES;
				}
				*IF_ |= set_mode(next_stat_mode);
				
				if (int mode = get_mode(); mode == 2) {
					clock_target_ += 80;
					next_stat_mode = 3;
					*LY_ += 1;
					*IF_ |= update_lyc();
				}
				else if (mode == 3) {
					clock_target_ += 170;
					next_stat_mode = 0;
				}
				else if (mode == 0) {
					clock_target_ += 206;
					if (*LY_ <= 143) {
						// TODO: draw here
						next_stat_mode = 2;
					}
					else {
						next_stat_mode = 1;
					}
				}
				else if (mode == 1) {
					clock_target_ += 456;
					next_stat_mode = 1;
					if (*LY_ == 144) {
						*IF_ |= IFInterrupt::VBLANK;
						// TODO: frame is done here, needs draw = true
					}
					*LY_ += 1;
					*IF_ |= update_lyc();
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
		*LY_ = 0x91;
		*STAT_ = 0b1000'0000;
		next_stat_mode = 2;
		clock_ = 0;
		clock_target_ = 0;
	}
	void PPU::Draw(TKPImage* screen) {
		test_draw(screen);
	}
	void PPU::test_draw(TKPImage* screen) {

	}
	int PPU::set_mode(int mode) {
		if (get_mode() == mode) {
			return 0;
		}
		*STAT_ &= 0b1111'1100;
		*STAT_ |= mode;
		if (mode != 3 && *STAT_ & (1 << (mode + 3))) {
			return IFInterrupt::LCDSTAT;
		}
		return 0;
	}
	int PPU::get_mode() {
		return *STAT_ & STATFlag::MODE;
	}
	int PPU::update_lyc() {
		if (*LYC_ == *LY_) {
			*STAT_ |= STATFlag::COINCIDENCE;
			if (*STAT_ & STATFlag::COINC_INTER)
				return IFInterrupt::LCDSTAT;
		}
		else {
			*STAT_ &= 0b1111'1011;
		}
		return 0;
	}
}