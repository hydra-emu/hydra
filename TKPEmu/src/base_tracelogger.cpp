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
            if (std::filesystem::is_directory(path_buf)) {
                std::cout << "Error: Path is directory" << std::endl;
            } else if (!overwrite && std::filesystem::exists(path_buf)) {
                file_exists = true;
                path_changed = false;
                ImGui::OpenPopup("Overwrite?");
            } else {
                ready_to_log_ = true;
            }
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