#ifndef TKP_GB_GAMEBOY_H
#define TKP_GB_GAMEBOY_H
#include "../emulator.h"
#include "../Tools/disassembly_instr.h"
#include "CPU/cpu.h"
#include <unordered_map>
namespace TKPEmu::Gameboy {
	class Gameboy : public Emulator {
	private:
		using CPU = TKPEmu::Gameboy::Devices::CPU;
		using PPU = TKPEmu::Gameboy::Devices::PPU;
		using Bus = TKPEmu::Gameboy::Devices::Bus;
		using Cartridge = TKPEmu::Gameboy::Devices::Cartridge;
		using DisInstr = TKPEmu::Tools::DisInstr;
		enum class BreakReq {
			RegA,
			RegB,
			RegC,
			RegD,
			RegE,
			RegF,
			RegH,
			RegL,
			PC,
			SP,
			LY
		};
		// This is to be implemented differently for each platform
		// Every member of this class is one that the user can add a Breakpoint to
		// Multiple members can be set to a value, which means a breakpoint is reached when all of the members match the state of the device
		struct Breakpoint {
			std::unordered_map<BreakReq, uint16_t> BreakReqs;
		};
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
		std::vector<Breakpoint> Breakpoints;
	private:
		Bus bus_;
		CPU cpu_;
		PPU ppu_;
		Cartridge cartridge_;
		uint16_t& load_break_req(const BreakReq req);
	};
}
#endif