#pragma once
#ifndef TKP_BASE_APPLICATION_H
#define TKP_BASE_APPLICATION_H
#include <memory>
#include "../../emulator.h"
#include "../../ImGui/imgui_internal.h"
#include "../../Tools/disassembly_instr.h"
namespace TKPEmu::Applications {
	class IMApplication {
	protected:
		using Emulator = TKPEmu::Emulator;
		using DisInstr = TKPEmu::Tools::DisInstr;
		bool* rom_loaded_;
	public:
		IMApplication(bool* rom_loaded) : rom_loaded_(rom_loaded) {};
		virtual ~IMApplication() = default;
		IMApplication(const IMApplication&) = delete;
		IMApplication& operator=(const IMApplication&) = delete;
		virtual void Draw(const char* title, bool* p_open = nullptr) = 0;
		virtual void Reset() {};
	};
}
#endif