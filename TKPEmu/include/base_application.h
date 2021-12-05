#pragma once
#ifndef TKP_BASE_APPLICATION_H
#define TKP_BASE_APPLICATION_H
#include <memory>
#include "emulator.h"
#include "../lib/imgui_internal.h"
#include "disassembly_instr.h"
namespace TKPEmu::Applications {
    enum class TKPShortcut {
        NONE,
        CTRL_R,
        CTRL_P,
        CTRL_F,
		CTRL_O,
        F7,
    };
	class IMApplication {
	private:
		using Emulator = TKPEmu::Emulator;
	public:
		IMApplication(std::string menu_title, std::string window_title);
		virtual ~IMApplication() = default;
		IMApplication(const IMApplication&) = delete;
		IMApplication& operator=(const IMApplication&) = delete;
		void Draw();
		void DrawMenuItem();
		void Reset();
		bool* IsOpen();
		void SetEmulator(Emulator* emulator);
		virtual void HandleShortcut(TKPShortcut& shortcut);
		static void SetupWindow();
		static void DrawMenuEmulation(Emulator* emulator);
        static void ResetEmulatorState(Emulator* emulator);
	protected:
		Emulator* emulator_ = nullptr;
		unsigned window_flags_ = 0;
	private:
		bool drawing_ = false;
		const std::string menu_title_;
		const std::string window_title_;
		virtual void v_draw() = 0;
		virtual void v_reset() {};
	};
}
#endif
