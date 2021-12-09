#pragma once
#ifndef TKP_GB_DISASSEMBLER_H
#define TKP_GB_DISASSEMBLER_H
#include <vector>
#include <algorithm>
#include <unordered_map>
// TODO: task.h is deprecated warning
// TODO: enter press on breakpoint adds breakpoint
// TODO: list of instructions combo box
#include <execution>
#include "../include/base_application.h"
#include "gb_breakpoint.h"
#include "gameboy.h"
namespace TKPEmu::Applications {
    class GameboyDisassembler : public IMApplication {
    private:
        using Gameboy = TKPEmu::Gameboy::Gameboy;
        using GameboyBreakpoint = TKPEmu::Gameboy::Utils::GameboyBreakpoint;
        using GBSelectionMap = std::vector<bool>;
    public:
        GameboyDisassembler(std::string menu_title, std::string window_title);
        void HandleShortcut(TKPShortcut& shortcut) override;
    private:
        GameboyBreakpoint debug_rvalues_;
        GBSelectionMap sel_map_{};
        uint8_t* LY_ = nullptr;
        bool clear_all_flag = false;
        bool popup_goto = false;
        int selected_bp = -1;
        void v_draw() override;
        void focus(int item);

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