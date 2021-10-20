#ifndef TKP_GB_GAMEBOY_H
#define TKP_GB_GAMEBOY_H
#include "../emulator.h"
#include "CPU/cpu.h"
namespace TKPEmu::Gameboy {
	class Gameboy : public Emulator {
	private:
		using CPU = TKPEmu::Gameboy::Devices::CPU;
		using PPU = TKPEmu::Gameboy::Devices::PPU;
		using Bus = TKPEmu::Gameboy::Devices::Bus;
		using Cartridge = TKPEmu::Gameboy::Devices::Cartridge;
	public:
		Gameboy(TKPImage* ptr);
		~Gameboy() = default;
		void Reset() override;
		void Update() override;
		void LoadFromFile(const std::string& path) override;
	private:
		Bus bus_;
		CPU cpu_;
		PPU ppu_;
		Cartridge cartridge_;
	};
}
#endif