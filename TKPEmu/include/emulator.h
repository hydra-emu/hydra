#pragma once
#ifndef TKP_EMULATOR_H
#define TKP_EMULATOR_H
#include <SDL2/SDL.h>
#include <iosfwd>
#include <mutex>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include "TKPImage.h"
#include "disassembly_instr.h"
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
		std::atomic_bool FastMode = false;
		std::atomic_int InstructionBreak = -1;
		void Start(EmuStartOptions start_mode);
		virtual void Reset();
		virtual void HandleKeyDown(SDL_Keycode keycode);
		virtual void HandleKeyUp(SDL_Keycode keycode);
		virtual void LoadFromFile(std::string&& path) = 0;
		void StartLogging(std::string filename);
		void StopLogging();
		void Screenshot(std::string filename);
		virtual float* GetScreenData();
		std::mutex ThreadStartedMutex;
		std::mutex DrawMutex;
		std::thread UpdateThread;
		TKPImage EmulatorImage{};
	protected:
		void log_state();
		std::unique_ptr<std::ofstream> ofstream_ptr_;
	private:
		virtual void v_log_state();
		virtual void start_normal();
		virtual void start_debug();
		virtual void update();
		virtual std::string print() const;
		friend std::ostream& operator<<(std::ostream& os, const Emulator& obj);
		bool logging_ = false;
		std::string log_filename_;
	};
}
#endif
