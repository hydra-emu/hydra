#pragma once
#ifndef TKP_GB_GAMEBOY_H
#define TKP_GB_GAMEBOY_H
#include <array>
#include "emulator.h"
#include "disassembly_instr.h"
#include "gb_breakpoint.h"
#include "gb_addresses.h"
#include "gb_cpu.h"
namespace TKPEmu::Gameboy {
	using GameboyPalettes = std::array<std::array<float, 3>,4>;
	using GameboyKeys = std::array<SDL_Keycode, 4>;
	class Gameboy : public Emulator {
	private:
		using CPU = TKPEmu::Gameboy::Devices::CPU;
		using PPU = TKPEmu::Gameboy::Devices::PPU;
		using Bus = TKPEmu::Gameboy::Devices::Bus;
		using Cartridge = TKPEmu::Gameboy::Devices::Cartridge;
		using DisInstr = TKPEmu::Tools::DisInstr;
		using GameboyBreakpoint = TKPEmu::Gameboy::Utils::GameboyBreakpoint;
	public:
		Gameboy(GameboyKeys& direction_keys, GameboyKeys& action_keys);
		~Gameboy();
		void Reset() override;
		void HandleKeyDown(SDL_Keycode key) override;
		void HandleKeyUp(SDL_Keycode key) override;
		void LoadFromFile(std::string&& path) override;
		float* GetScreenData() override;
        DisInstr GetInstruction(uint16_t address);
		bool AddBreakpoint(GBBPArguments bp);
		void RemoveBreakpoint(int index);
		const auto& GetOpcodeDescription(uint8_t opc);
		GameboyPalettes& GetPalette();
		CPU& GetCPU() { return cpu_; }
		std::vector<GameboyBreakpoint> Breakpoints{};
		std::vector<DisInstr> Instructions{};
	private:
		Bus bus_;
		CPU cpu_;
		PPU ppu_;
		Cartridge cartridge_;
		GameboyKeys& direction_keys_;
		GameboyKeys& action_keys_;
		uint8_t& joypad_, &interrupt_flag_;
		std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point b = std::chrono::system_clock::now();
		float sleep_time_ = 16.75f;
		void update() override;
		std::string print() const override;
		void start_normal() override;
		void start_debug() override;
		void limit_fps();
	};
}
#endif
