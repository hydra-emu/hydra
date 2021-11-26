#pragma once
#ifndef TKP_BASE_APPLICATION_H
#define TKP_BASE_APPLICATION_H
#include <memory>
#include "emulator.h"
#include "../lib/imgui_internal.h"
#include "disassembly_instr.h"
namespace TKPEmu::Applications {
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
		void SetEmulator(Emulator* emulator);

		static void SetupWindow();
		static void DrawMenuEmulation(Emulator* emulator, bool* rom_loaded);
        static void ResetEmulatorState(Emulator* emulator);
	protected:
		Emulator* emulator_ = nullptr;
	private:
		bool drawing_ = false;
		const std::string menu_title_;
		const std::string window_title_;
		virtual void v_draw();
		virtual void v_reset();
	};
}
#endif
