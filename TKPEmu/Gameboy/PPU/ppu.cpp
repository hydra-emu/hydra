#include "ppu.h"

namespace TKPEmu::Gameboy::Devices {
	PPU::PPU(Bus* bus) : bus_(bus) {
	}
	bool PPU::NeedsDraw(){
		return needs_draw;
	}
	void PPU::Update()
	{
	}
	void PPU::Reset()
	{
	}
	void PPU::Draw(TKPImage* screen)
	{
	}
}