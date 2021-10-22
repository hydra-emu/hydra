#ifndef TKP_GB_GAMEBOY_H
#define TKP_GB_GAMEBOY_H
#include "../emulator.h"
#include "../Tools/disassembly_instr.h"
#include "CPU/cpu.h"
namespace TKPEmu::Gameboy {
	class Gameboy : public Emulator {
	private:
		using CPU = TKPEmu::Gameboy::Devices::CPU;
		using PPU = TKPEmu::Gameboy::Devices::PPU;
		using Bus = TKPEmu::Gameboy::Devices::Bus;
		using Cartridge = TKPEmu::Gameboy::Devices::Cartridge;
		using DisInstr = TKPEmu::Tools::DisInstr;
	public:
		Gameboy();
		~Gameboy() = default;
		void Start() override;
		void StartDebug() override;
		void Reset() override;
		void Update() override;
		void UpdateDebug() override;
		void LoadFromFile(const std::string& path) override;
		void LoadInstrToVec(std::vector<DisInstr>& vec, bool& finished) override;

	private:
		Bus bus_;
		CPU cpu_;
		PPU ppu_;
		Cartridge cartridge_;
	};
}
#endif