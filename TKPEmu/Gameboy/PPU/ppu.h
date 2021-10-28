#ifndef TKP_GB_PPU_H
#define TKP_GB_PPU_H
#include "../Bus/bus.h"
#include "../../Tools/TKPImage.h"
namespace TKPEmu::Gameboy::Devices {
	class PPU {
	private:
		using TKPImage = TKPEmu::Tools::TKPImage;
	public:
		PPU(Bus* bus);
		bool NeedsDraw();
		void Update();
		void Reset();
		// Lock the mutex from the emulator before calling this function
		void Draw(TKPImage* screen);
	private:
		Bus* bus_;
		bool needs_draw = false;
		// TODO: remove test_draw function
		void test_draw();
	};
}
#endif