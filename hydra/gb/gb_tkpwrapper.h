#pragma once
#ifndef TKP_GB_GAMEBOY_H
#define TKP_GB_GAMEBOY_H
#include <array>
#include <include/emulator.h>
#include <gb/gb_breakpoint.h>
#include <gb/gb_addresses.h>
#include <gb/gb_cpu.h>
#include <gb/gb_ppu.h>
#include <gb/gb_bus.h>
#include <gb/gb_timer.h>
#include <gb/gb_apu.h>
#include <gb/gb_apu_ch.h>

namespace hydra {
	namespace Applications {
		class GameboyRomData;
	}
	namespace Gameboy::QA {
		struct TestGameboy;
	}
}
namespace hydra::Gameboy {
	class Gameboy_TKPWrapper : public Emulator {
		TKP_EMULATOR(Gameboy_TKPWrapper);
	private:
		using GameboyPalettes = std::array<std::array<float, 3>,4>;
		using GameboyKeys = std::array<uint32_t, 4>;
		using CPU = hydra::Gameboy::CPU;
		using PPU = hydra::Gameboy::PPU;
		using APU = hydra::Gameboy::APU;
		using ChannelArrayPtr = hydra::Gameboy::ChannelArrayPtr;
		using ChannelArray = hydra::Gameboy::ChannelArray;
		using Bus = hydra::Gameboy::Bus;
		using Timer = hydra::Gameboy::Timer;
		using Cartridge = hydra::Gameboy::Cartridge;
		using GameboyBreakpoint = hydra::Gameboy::Utils::GameboyBreakpoint;
	public:
		// Used by automated tests
		void Update() { update(); }
	private:
		ChannelArrayPtr channel_array_ptr_;
		Bus bus_;
		APU apu_;
		PPU ppu_;
		Timer timer_;
		CPU cpu_;
		GameboyKeys direction_keys_;
		GameboyKeys action_keys_;
		uint8_t& joypad_, &interrupt_flag_;
		__always_inline void update_audio_sync();
		friend class hydra::Gameboy::QA::TestGameboy;
	};
}
#endif
