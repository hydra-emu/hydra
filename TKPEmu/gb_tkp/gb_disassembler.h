#pragma once
#ifndef TKP_GB_DISASSEMBLER_H
#define TKP_GB_DISASSEMBLER_H
#include <vector>
#include <algorithm>
#include <unordered_map>
// TODO: task.h is deprecated warning
// TODO: enter press on breakpoint adds breakpoint
#include <execution>
#include "../include/base_disassembler.h"
#include "gb_breakpoint.h"
#include "gameboy.h"
namespace TKPEmu::Applications {
    using Gameboy = TKPEmu::Gameboy::Gameboy;
    using GameboyBreakpoint = TKPEmu::Gameboy::Utils::GameboyBreakpoint;
    using GBSelectionMap = std::vector<bool>;
    class GameboyDisassembler : public BaseDisassembler {
    public:
        GameboyDisassembler(bool* rom_loaded);
        void Focus(int item);
        void v_draw() override;
    private:
        GameboyBreakpoint debug_rvalues_;
        GBSelectionMap sel_map_{};
        uint8_t* LY_ = nullptr;
        bool clear_all_flag = false;
        int selected_bp = -1;
        template<typename T>
        void breakpoint_register_checkbox(const char* checkbox_l, T& value, bool& is_used, ImGuiDataType type = ImGuiDataType_U8) {
            ImGui::Checkbox(checkbox_l, &is_used);
            if (is_used) {
                ImGui::SameLine();
                int w = 20;
                if (type == ImGuiDataType_U16)
                    w = 40;
                ImGui::PushItemWidth(w);
                char id[6] = "##IDt";
                id[4] = checkbox_l[0];
                ImGui::InputScalar(id, type, &value, 0, 0, w == 20 ? "%02X" : "%04X",
                    ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CharsHexadecimal);
                ImGui::PopItemWidth();
            }
        }
    };
}
#endif
