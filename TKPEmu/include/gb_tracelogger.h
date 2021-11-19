#pragma once
#ifndef TKP_GB_TRACELOGGER_H
#define TKP_GB_TRACELOGGER_H
#include "base_tracelogger.h"
#include "gameboy.h"
namespace TKPEmu::Applications {
	using Gameboy = TKPEmu::Gameboy::Gameboy;
	using LogType = unsigned;
	// TODO: make into enum
	const LogType LOG_NONE = 0;
	const LogType LOG_PC   = 1 << 0;
	const LogType LOG_SP   = 1 << 1;
	const LogType LOG_A    = 1 << 2;
	const LogType LOG_B    = 1 << 3;
	const LogType LOG_C    = 1 << 4;
	const LogType LOG_D    = 1 << 5;
	const LogType LOG_E    = 1 << 6;
	const LogType LOG_F    = 1 << 7;
	const LogType LOG_H    = 1 << 8;
	const LogType LOG_L    = 1 << 9;
	const LogType LOG_LY   = 1 << 10;
	const LogType LOG_CY   = 1 << 11;
	const LogType LOG_DIV  = 1 << 12;
	const LogType LOG_TMA  = 1 << 13;
	const LogType LOG_TIMA = 1 << 14;
	const LogType LOG_TAC  = 1 << 15;
	constexpr auto LogTypeSize = 16;
	class GameboyTracelogger : public BaseTracelogger {
	public:
		GameboyTracelogger() {};
	private:
		void v_draw() noexcept final override;
		Gameboy* emulator_ = nullptr;
		LogType log_type_;
		int lines_logged_ = 0;
		std::unique_ptr<std::ofstream> buffer_ptr_ = nullptr;
		void log_emu_state();
	};
}
#endif
