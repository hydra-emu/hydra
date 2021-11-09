#pragma once
#ifndef TKP_EMULATOR_H
#define TKP_EMULATOR_H
#include "Tools/TKPImage.h"
#include "Tools/disassembly_instr.h"
#include <mutex>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
namespace TKPEmu {
	using TKPImage = TKPEmu::Tools::TKPImage;
	using DisInstr = TKPEmu::Tools::DisInstr;
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
		std::atomic_int InstructionBreak = -1;
		constexpr virtual int GetPCHexCharSize() { return 1; }
		virtual void Start() = 0;
		virtual void StartDebug() = 0;
		virtual void StartLog() = 0;
		virtual void Reset() = 0;
		virtual void Update() = 0;
		virtual void LoadFromFile(std::string&& path) = 0;
		virtual float* GetScreenData() { return nullptr; };
		std::mutex ThreadStartedMutex;
		std::mutex DrawMutex;
		std::thread UpdateThread;
		TKPImage EmulatorImage{};
	};
}
#endif