#include "../include/gb_tracelogger.h"
#include "../include/gb_addresses.h"
#include "../lib/imgui.h"
namespace TKPEmu::Applications {
	void GameboyTracelogger::v_draw() {
		ImGui::NewLine();
		ImGui::TextUnformatted("Memory to log:");
		ImGui::Separator();
		for (int i = 0; i < LogTypeSize; i++) {
			ImGui::Checkbox(LogTypeNames[i].c_str(), &available_types_[i]);
			if ((i + 1) % 5 != 0) {
				ImGui::SameLine();
			}
		}
	}
	void GameboyTracelogger::set_logtypes() {
		std::unique_ptr<std::vector<LogType>> ptr = std::make_unique<std::vector<LogType>>();
		for (int i = 0; i < LogTypeSize; i++) {
			if (available_types_[i]) {
				ptr->push_back(LogTypeMap[i]);
			}
		}
		(dynamic_cast<Gameboy*>(emulator_))->SetLogTypes(std::move(ptr));
	}
}
