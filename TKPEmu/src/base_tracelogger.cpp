#include "../include/base_tracelogger.h"
namespace TKPEmu::Applications {
    void BaseTracelogger::SetEmulator(Emulator* emulator) {
        emulator_ = emulator;
    }
    void BaseTracelogger::Draw(const char* title, bool* p_open = nullptr) {
        TKPEmu::Applications::IMApplication::SetupWindow();
        if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar)) {
            ImGui::End();
            return;
        }
        static std::string path_buf = std::filesystem::current_path();
        bool path_changed = false;
        if (ImGui::InputText("##path", &path_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
            path_changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Set", ImVec2(ImGui::GetContentRegionAvail().x * 0.35f, 0.0f))) {
            path_changed = true;
        }
        if (path_changed) {
            if (std::filesystem::exists(path_buf)) {
                bool overwrite = false;
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal("Overwrite?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    const std::string text = path_buf + " already exists. Overwrite?";
                    ImGui::TextUnformatted(text.c_str());
                    if (ImGui::Button("Yes")) {
                        overwrite = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("No")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
        }
        v_draw();
        ImGui::End();
    }
}