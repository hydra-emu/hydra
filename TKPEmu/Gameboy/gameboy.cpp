#include "gameboy.h"
namespace TKPEmu::Gameboy {
	Gameboy::Gameboy(TKPImage* ptr) : Emulator(ptr), cpu_(&bus_), ppu_(&bus_), cartridge_() {

	}
	void Gameboy::Reset() {
		cpu_.Reset();
		ppu_.Reset();
	}
	void Gameboy::Update() {
		cpu_.Update();
		//ppu_.Update();
		//if (ppu_.NeedsDraw()) {
		//	ScreenDataMutex.lock();
		//	ppu_.Draw(EmulatorImage);
		//	ScreenDataMutex.unlock();
		//}
	}
	void Gameboy::LoadFromFile(const std::string& path) {
		cartridge_.Load(path, bus_.mem);
	}
}