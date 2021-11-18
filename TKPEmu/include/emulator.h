#pragma once
#ifndef TKP_EMULATOR_H
#define TKP_EMULATOR_H
#include "TKPImage.h"
#include "disassembly_instr.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <mutex>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
namespace TKPEmu {
	using TKPImage = TKPEmu::Tools::TKPImage;
	using DisInstr = TKPEmu::Tools::DisInstr;
	enum class EmuStartOptions {
		Normal,
		Debug
	};
	class Emulator {
	public:
		Emulator() = default;
		virtual ~Emulator() = default;
		Emulator(const Emulator&) = delete;
		Emulator& operator=(const Emulator&) = delete;
		std::atomic_bool Stopped = false;
		std::atomic_bool Paused = false;
		std::atomic_bool Step = false;
		std::atomic_bool LogReady = false;
		std::atomic_bool FastMode = false;
		std::atomic_int InstructionBreak = -1;
		constexpr virtual int GetPCHexCharSize();
		virtual void Start(EmuStartOptions start_mode);
		virtual void Reset();
		virtual void HandleKeyDown(SDL_Keycode keycode);
		virtual void HandleKeyUp(SDL_Keycode keycode);
		virtual void LoadFromFile(std::string&& path) = 0;
		virtual void Screenshot(std::string filename);
		virtual float* GetScreenData();
		std::mutex ThreadStartedMutex;
		std::mutex DrawMutex;
		std::thread UpdateThread;
		TKPImage EmulatorImage{};
		protected:
		virtual void start_normal();
		virtual void start_debug();
		virtual void update();
		virtual std::string print() const;
		private:
		friend std::ostream& operator<<(std::ostream& os, const Emulator& obj);
	};
}
#endif
