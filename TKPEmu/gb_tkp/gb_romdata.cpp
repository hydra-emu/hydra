#include "gb_romdata.h"
namespace TKPEmu::Applications {
    GameboyRomData::GameboyRomData(std::string menu_title, std::string window_title) 
        : IMApplication(menu_title, window_title) 
    {
        max_size = ImVec2(400, 400);
    }
    void GameboyRomData::v_draw() {
        static Gameboy* gb_ptr = static_cast<Gameboy*>(emulator_);
        ImGui::Text(gb_ptr->GetCartridge()->GetCartridgeTypeName());
    }
}