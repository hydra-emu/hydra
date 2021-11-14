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
		static void SetupWindow() {
			ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(450, 450));
		};
	};
}
#endif
