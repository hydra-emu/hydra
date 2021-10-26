#ifndef TKP_GB_GAMEBOY_H
#define TKP_GB_GAMEBOY_H
#include "../emulator.h"
#include "../Tools/disassembly_instr.h"
#include "CPU/cpu.h"
#include <unordered_map>
namespace TKPEmu::Gameboy {
	// If a value in this struct is -1, that specific register won't matter for the breakpoint 
	struct GameboyBreakpoint {
		uint8_t A_Value = -1;
		uint8_t B_Value = -1;
		uint8_t C_Value = -1;
		uint8_t D_Value = -1;
		uint8_t E_Value = -1;
		uint8_t F_Value = -1;
		uint8_t H_Value = -1;
		uint8_t L_Value = -1;
		uint16_t PC_Value = -1;
		uint16_t SP_Value = -1;
	};
	class Gameboy : public Emulator {
	private:
		using CPU = TKPEmu::Gameboy::Devices::CPU;
		using PPU = TKPEmu::Gameboy::Devices::PPU;
		using Bus = TKPEmu::Gameboy::Devices::Bus;
		using Cartridge = TKPEmu::Gameboy::Devices::Cartridge;
		using DisInstr = TKPEmu::Tools::DisInstr;
	public:
		Gameboy();
		~Gameboy();
		int GetPCHexCharSize() override { return 4; };
		void Start() override;
		void StartDebug() override;
		void Reset() override;
		void Update() override;
		void LoadFromFile(const std::string& path) override;
		void LoadInstrToVec(std::vector<DisInstr>& vec, bool& finished) override;
		void AddBreakpoint(GameboyBreakpoint bp);
		void RemoveBreakpoint(int index);
	private:
		Bus bus_;
		CPU cpu_;
		PPU ppu_;
		Cartridge cartridge_;
	};
}
#endif