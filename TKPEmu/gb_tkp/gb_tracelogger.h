#pragma once
#ifndef TKP_GB_TRACELOGGER_H
#define TKP_GB_TRACELOGGER_H
#include <array>
#include "../include/base_application.h"
#include "gameboy.h"
namespace TKPEmu::Applications {
	using Gameboy = TKPEmu::Gameboy::Gameboy;
	class GameboyTracelogger : public IMApplication {
	public:
		GameboyTracelogger(std::string menu_title, std::string window_title);
	private:
		std::array<bool, LogTypeSize> available_types_{};
		std::string log_path_;
        bool ready_to_log_ = false;
        bool is_logging_ = false;
		void v_draw() override;
		void set_logtypes();
	};
}
#endif
