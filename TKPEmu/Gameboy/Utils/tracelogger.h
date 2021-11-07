#pragma once
#ifndef TKP_GB_TRACELOGGER_H
#define TKP_GB_TRACELOGGER_H
#include "../../Display/Applications/base_tracelogger.h"
#include "../gameboy.h"
namespace TKPEmu::Applications {
	using Gameboy = TKPEmu::Gameboy::Gameboy;
	using LogType = unsigned;
	const LogType LOG_PC   = 1 << 1;
	const LogType LOG_SP   = 1 << 2;
	const LogType LOG_A    = 1 << 3;
	const LogType LOG_B    = 1 << 4;
	const LogType LOG_C    = 1 << 5;
	const LogType LOG_D    = 1 << 6;
	const LogType LOG_E    = 1 << 7;
	const LogType LOG_F    = 1 << 8;
	const LogType LOG_H    = 1 << 9;
	const LogType LOG_L    = 1 << 10;
	const LogType LOG_LY   = 1 << 11;
	const LogType LOG_CY   = 1 << 12;
	const LogType LOG_DIV  = 1 << 13;
	const LogType LOG_TMA  = 1 << 14;
	const LogType LOG_TIMA = 1 << 15;
	const LogType LOG_TAC  = 1 << 16;
	const auto LogTypeSize = 17;
	class GameboyTracelogger : public BaseTracelogger {
	private:
		void v_draw() noexcept final override;
		Gameboy* emulator_ = nullptr;
		int lines_logged_ = 0;
		std::unique_ptr<std::ofstream> buffer_ptr_ = nullptr;
		void log_emu_state(LogType);
	public:
		GameboyTracelogger(bool* log_mode) : BaseTracelogger(log_mode) {};
		void SetEmulator(Emulator* emulator) noexcept;
		void SetFile(std::string file) final override;
	};
}
#endif