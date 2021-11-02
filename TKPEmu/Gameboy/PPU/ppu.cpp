#include "ppu.h"

namespace TKPEmu::Gameboy::Devices {
	PPU::PPU(Bus* bus) : bus_(bus), mem_OAM_(),
		LCDC(bus->GetReference(0xFF40)), 
		STAT(bus->GetReference(0xFF41)), 
		LY(bus->GetReference(0xFF44)), 
		LYC(bus->GetReference(0xFF45)) {}
	bool PPU::NeedsDraw(){
		return needs_draw;
	}
	void PPU::Update(uint8_t tTemp) {
		//*IF_ = 0xE0;
		clock_ += tTemp;
		if (LCDC & LCDCFlag::LCD_ENABLE) {
			if (clock_ >= clock_target_) {
				if (LY == 153) {
					next_stat_mode = 2;
					LY = -1;
					clock_ %= FRAME_CYCLES;
					clock_target_ = FRAME_CYCLES;
				}
				//*IF_ |= set_mode(next_stat_mode);
				
				if (int mode = get_mode(); mode == 2) {
					clock_target_ += 80;
					next_stat_mode = 3;
					LY += 1;
					//*IF_ |= update_lyc();
				}
				else if (mode == 3) {
					clock_target_ += 170;
					next_stat_mode = 0;
				}
				else if (mode == 0) {
					clock_target_ += 206;
					if (LY <= 143) {
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
					if (LY == 144) {
						//*IF_ |= IFInterrupt::VBLANK;
						// TODO: frame is done here, needs draw = true
					}
					LY += 1;
					//*IF_ |= update_lyc();
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
	void PPU::Draw(TKPImage* screen) {
		test_draw(screen);
	}
	void PPU::test_draw(TKPImage* screen) {

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
}