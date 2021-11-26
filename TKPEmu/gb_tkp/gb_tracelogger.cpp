#include <filesystem>
#include <iostream>
#include "gb_tracelogger.h"
#include "gb_addresses.h"
#include "../lib/imgui.h"
#include "../lib/imgui_stdlib.h"
namespace TKPEmu::Applications {
	void GameboyTracelogger::GameboyTracelogger(std::string menu_title, std::string window_title)
		: IMApplication(menu_title, window_title)
	{}
	void GameboyTracelogger::v_draw() {
        static std::string path_buf = std::filesystem::current_path();
        static bool path_changed = false;
        static bool file_exists = false;
        static bool overwrite = false;
        if (ImGui::InputText("##path", &path_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
            ready_to_log_ = false;
            path_changed = true;
            overwrite = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Set", ImVec2(ImGui::GetContentRegionAvail().x * 0.35f, 0.0f))) {
            ready_to_log_ = false;
            path_changed = true;
            overwrite = false;
        }
        if (path_changed) {
            path_changed = false;
            if (std::filesystem::is_directory(path_buf)) {
                std::cout << "Error: Path is directory" << std::endl;
            } else if (!overwrite && std::filesystem::exists(path_buf)) {
                file_exists = true;
                ImGui::OpenPopup("Overwrite?");
            } else {
                ready_to_log_ = true;
                log_path_ = path_buf;
            }
        }
        if (!ready_to_log_) {
            // This push makes the button seem disabled
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        }
        if (!is_logging_) {
            if (ImGui::Button("Start logging")) {
                if (emulator_ != nullptr) {
                    set_logtypes();
                    emulator_->StartLogging(log_path_);
                    is_logging_ = true;
                } else {
                    std::cerr << "Error: Emulator is nullptr" << std::endl;
                }
            }
        } else {
            if (ImGui::Button("Stop logging")) {
                if (emulator_ != nullptr) {
                    emulator_->StopLogging();
                    is_logging_ = false;
                } else {
                    std::cerr << "Error: Emulator is nullptr" << std::endl;
                }
            }
        }
        if (!ready_to_log_) {
            // Pop disabled
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
		ImGui::NewLine();
		ImGui::TextUnformatted("Memory to log:");
		ImGui::Separator();
		for (int i = 0; i < LogTypeSize; i++) {
			ImGui::Checkbox(LogTypeNames[i].c_str(), &available_types_[i]);
			if ((i + 1) % 5 != 0) {
				ImGui::SameLine();
			}
		}
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(350, 75));
        if (ImGui::BeginPopupModal("Overwrite?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("This file already exists. Overwrite?");
            if (ImGui::Button("Yes")) {
                overwrite = true;
                path_changed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
	}
	void GameboyTracelogger::set_logtypes() {
		std::unique_ptr<std::vector<LogType>> ptr = std::make_unique<std::vector<LogType>>();
		for (int i = 0; i < LogTypeSize; i++) {
			if (available_types_[i]) {
				ptr->push_back(LogTypeMap[i]);
			}
		}
		static_cast<Gameboy*>(emulator_)->SetLogTypes(std::move(ptr));
	}
}
