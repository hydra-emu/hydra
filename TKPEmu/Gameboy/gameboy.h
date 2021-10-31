#pragma once
#ifndef TKP_GB_GAMEBOY_H
#define TKP_GB_GAMEBOY_H
#include "../emulator.h"
#include "../Tools/disassembly_instr.h"
#include "Utils/breakpoint.h"
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
		using GameboyBreakpoint = TKPEmu::Gameboy::Utils::GameboyBreakpoint;
		// TODO: for debugging the cpu
		std::string str = "D:/gblogs/tkpemu/logtest1.txt";
	public:
		Gameboy();
		~Gameboy();
		constexpr int GetPCHexCharSize() override { return 4; };
		void Start() override;
		void StartDebug() override;
		void Reset() override;
		void Update() override;
		void LoadFromFile(const std::string& path) override;
		void LoadInstrToVec(std::vector<DisInstr>& vec, std::atomic_bool& finished);
		void AddBreakpoint(GameboyBreakpoint bp);
		void RemoveBreakpoint(int index);
		void CopyRegToBreakpoint(GameboyBreakpoint& bp);
		const auto& GetOpcodeDescription(uint8_t opc);
		CPU& GetCPU() { return cpu_; }
		std::vector<GameboyBreakpoint> Breakpoints;
	private:
		Bus bus_;
		CPU cpu_;
		PPU ppu_;
		Cartridge cartridge_;
		std::ofstream file;
	};
}
#endif