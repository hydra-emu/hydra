#include "gb_romdata.h"
namespace TKPEmu::Applications {
    GameboyRomData::GameboyRomData(std::string menu_title, std::string window_title) 
        : IMApplication(menu_title, window_title) 
    {
        min_size = ImVec2(400, 400);
        max_size = ImVec2(500, 400);
    }
    void GameboyRomData::v_draw() {
        if (ImGui::BeginTabBar("RomDataTabs", ImGuiTabBarFlags_None)) {
            if (ImGui::BeginTabItem("Info")) {
                draw_info();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tilesets")) {
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    void GameboyRomData::draw_info() {
        Gameboy* gb_ptr = static_cast<Gameboy*>(emulator_);
        ImGui::TextUnformatted(gb_ptr->CurrentFilename.c_str());
        ImGui::TextUnformatted("Mapper: "); ImGui::SameLine(); ImGui::TextUnformatted(gb_ptr->GetCartridge()->GetCartridgeTypeName());
        ImGui::InputText("Rom hash", gb_ptr->RomHash.data(), gb_ptr->RomHash.length(), ImGuiInputTextFlags_ReadOnly);
        static bool hashed = false;
        static std::string hash = "?";
        static std::string result = "?";
        if (gb_ptr->Paused) {
            hashed = true;
            hash = gb_ptr->GetScreenshotHash();
            result = "{ \"" + gb_ptr->RomHash + "\", { " + std::to_string(gb_ptr->GetCPU().TotalClocks) + ", \"" + hash + "\" } },";
        } else if (hashed && !gb_ptr->Paused) {
            hashed = false;
            hash = "Pause to get current screenshot hash";
        }
        ImGui::InputText("VRAM hash", hash.data(), hash.length(), ImGuiInputTextFlags_ReadOnly);
        if (hashed) {
            ImGui::InputText("emulator_results.h hash", result.data(), result.length(), ImGuiInputTextFlags_ReadOnly);
        }
    }
}