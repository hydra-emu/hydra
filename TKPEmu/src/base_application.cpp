#include "../include/base_application.h"

namespace TKPEmu::Applications {
    IMApplication::IMApplication(std::string menu_title, std::string window_title) :
        menu_title_(menu_title),
        window_title_(window_title)
    {}
    void IMApplication::SetEmulator(Emulator* emulator) {
        emulator_ = emulator;
    }
    void IMApplication::HandleShortcut(TKPShortcut& shortcut) {}
    void IMApplication::Draw() {
        if (drawing_) {
            SetupWindow();
            if (!ImGui::Begin(window_title_.c_str(), &drawing_, window_flags_)) {
                ImGui::End();
                return;
            }
            v_draw();
            ImGui::End();
        }
    }
    void IMApplication::DrawMenuItem() {
        if (ImGui::MenuItem(menu_title_.c_str(), NULL, drawing_)) {
             drawing_ ^= true;
        }
    }
    bool* IMApplication::IsOpen() {
        // Returns bool* to be used with imgui
        return &drawing_;
    }
    void IMApplication::Reset() {
        v_reset();
    }
    void IMApplication::SetupWindow() {
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(550, 550));
    }
    void IMApplication::DrawMenuEmulation(Emulator* emulator) {
        bool rom_loaded = (emulator != nullptr);
        if (!rom_loaded) {
            ImGui::MenuItem("Pause", "Ctrl+P", false, rom_loaded);
        }
        else {
            if (ImGui::MenuItem("Pause", "Ctrl+P", emulator->Paused.load(), rom_loaded)) {
                emulator->Paused.store(!emulator->Paused.load());
                emulator->Step.store(true);
                emulator->Step.notify_all();
            }
        }
        if (ImGui::MenuItem("Reset", "Ctrl+R", false, rom_loaded)) {
            emulator->ResetState();
            emulator->Start(EmuStartOptions::Debug);
        }
        if (ImGui::MenuItem("Stop", NULL, false, rom_loaded)) {
            emulator->ResetState();
        }
        ImGui::EndMenu();
    }
}