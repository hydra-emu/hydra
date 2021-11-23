#pragma once
#ifndef TKP_BASE_APPLICATION_H
#define TKP_BASE_APPLICATION_H
#include <memory>
#include "emulator.h"
#include "../lib/imgui_internal.h"
#include "disassembly_instr.h"
namespace TKPEmu::Applications {
	class IMApplication {
	protected:
		using Emulator = TKPEmu::Emulator;
	public:
		IMApplication() = default;
		virtual ~IMApplication() = default;
		IMApplication(const IMApplication&) = delete;
		IMApplication& operator=(const IMApplication&) = delete;
		virtual void Draw(const char* title, bool* p_open = nullptr) = 0;
		virtual void Reset() {};
		static void SetupWindow();
		static void DrawMenuEmulation(Emulator* emulator, bool* rom_loaded);
        static void ResetEmulatorState(Emulator* emulator);
	};
}
#endif
