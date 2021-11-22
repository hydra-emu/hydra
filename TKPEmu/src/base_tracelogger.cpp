#include <iostream>
#include "../include/base_tracelogger.h"
namespace TKPEmu::Applications {
    void BaseTracelogger::SetEmulator(Emulator* emulator) {
        emulator_ = emulator;
    }
    void BaseTracelogger::Draw(const char* title, bool* p_open) {
        TKPEmu::Applications::IMApplication::SetupWindow();
        if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar)) {
            ImGui::End();
            return;
        }
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
            // Push disabled
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.6f);
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        }
        if (!is_logging_) {
            if (ImGui::Button("Start logging")) {
                if (emulator_ != nullptr) {
                    set_logtypes();
                    emulator_->StartLogging(log_path_);
                } else {
                    std::cerr << "Error: Emulator is nullptr" << std::endl;
                }
                is_logging_ = true;
            }
        } else {
            if (ImGui::Button("Stop logging")) {
                if (emulator_ != nullptr) {
                    emulator_->StopLogging();
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
        v_draw();
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
        ImGui::End();
    }
}