#pragma once
#ifndef TKP_BASE_APPLICATION_H
#define TKP_BASE_APPLICATION_H
#include <memory>
#include "emulator.h"
#include "../imgui/imgui_internal.h"
#include "disassembly_instr.h"
namespace TKPEmu::Applications {
    enum class TKPShortcut {
        NONE,
        CTRL_R,
        CTRL_P,
        CTRL_F,
		CTRL_O,
		CTRL_S,
		CTRL_L,
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
		void SetEmulator(std::shared_ptr<Emulator> emulator);
		virtual void HandleShortcut(TKPShortcut&);
		static void SetupWindow(const ImVec2& min_size, const ImVec2& max_size);
		static void DrawMenuEmulation(Emulator* emulator);
        static void ResetEmulatorState(Emulator* emulator);
	protected:
		std::shared_ptr<Emulator> emulator_;
		unsigned window_flags_ = 0;
		ImVec2 min_size = { 400, 400 };
		ImVec2 max_size = { 750, 750 };
	private:
		bool drawing_ = false;
		const std::string menu_title_;
		const std::string window_title_;
		virtual void v_draw() = 0;
		virtual void v_reset() {};
	};
}
#endif
