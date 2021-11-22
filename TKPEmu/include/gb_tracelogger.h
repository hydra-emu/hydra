#pragma once
#ifndef TKP_GB_TRACELOGGER_H
#define TKP_GB_TRACELOGGER_H
#include "base_tracelogger.h"
#include "gameboy.h"
namespace TKPEmu::Applications {
	using Gameboy = TKPEmu::Gameboy::Gameboy;
	class GameboyTracelogger : public BaseTracelogger {
	public:
		GameboyTracelogger() {};
	private:
		void v_draw() override;
		void set_logtypes() override;
		Gameboy* emulator_ = nullptr;
	};
}
#endif
