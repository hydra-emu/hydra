#pragma once
#ifndef TKP_GB_DISASSEMBLER_H
#define TKP_GB_DISASSEMBLER_H
#include <vector>
#include <algorithm>
#include <unordered_map>
// TODO: task.h is deprecated warning
#include <execution>
#include "../include/base_disassembler.h"
#include "gb_breakpoint.h"
#include "gameboy.h"
// TODO: put code in .cpp file
namespace TKPEmu::Applications {
    class GameboyDisassembler : public BaseDisassembler {
    private:
        using GameboyBreakpoint = TKPEmu::Gameboy::Utils::GameboyBreakpoint;
        using Gameboy = TKPEmu::Gameboy::Gameboy;
        using GBSelectionMap = std::vector<bool>;
        GameboyBreakpoint debug_rvalues_;
        GBSelectionMap sel_map_{};
        Gameboy* emulator_ = nullptr;
        uint8_t* LY_ = nullptr;
        bool clear_all_flag = false;
        int selected_bp = -1;
    public:
        GameboyDisassembler(bool* rom_loaded) : BaseDisassembler(rom_loaded) {
            sel_map_.resize(0x10000);
        };
        void Focus(int item) noexcept {
            ImGuiContext& g = *ImGui::GetCurrentContext();
            ImGuiWindow* window = g.CurrentWindow;
            // TODO: find a way to make item_height not hard coded
            static const int item_height = 17;
            window->Scroll.y = IM_FLOOR(item_height * item);
        }
        void SetEmulator(Emulator* emulator) override {
            emulator_ = dynamic_cast<Gameboy*>(emulator);
        }
        void v_draw() noexcept {
            bool bp_add_popup = false;
            int goto_pc = -1;
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Emulation")) {
                    DrawMenuEmulation(emulator_, rom_loaded_);
                }
                if (ImGui::BeginMenu("Navigation")) {
                    if (ImGui::MenuItem("Step", "F7", false, emulator_->Paused.load())) {
                        emulator_->Step.store(true);
                        emulator_->Step.notify_all();
                    }
                    if (ImGui::MenuItem("Goto PC")) {
                        // TODO: if PC not found, go to nearest close to that value
                        OpenGotoPopup = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            if (OpenGotoPopup) {
                ImGui::OpenPopup("Goto Program Code");
            }
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Goto Program Code", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Program Code (in hex) to go to:");
                ImGui::Separator();
                // 4 hexadecimal characters and a null terminator
                constexpr size_t buf_size = 4 + 1;
                static char buf[buf_size] = "";
                bool close = false;
                if (OpenGotoPopup) {
                    if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
                        ImGui::SetKeyboardFocusHere(0);
                    OpenGotoPopup = false;
                }
                if (ImGui::InputText("hexadecimal", buf, buf_size, 
                        ImGuiInputTextFlags_CharsHexadecimal |
                        ImGuiInputTextFlags_CharsUppercase |
                        ImGuiInputTextFlags_EnterReturnsTrue)) {
                    close = true;
                }
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    close = true;
                }
                if (close) {
                    // Convert hex to int
                    unsigned x = 0;
                    std::stringstream ss;
                    ss << std::hex << buf;
                    ss >> x;
                    goto_pc = x;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            static ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV
                | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV;
            {
                ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, ImGui::GetContentRegionAvail().y), true);
                if (ImGui::BeginTable("cmds", 3, flags)) {
                    ImGui::TableSetupColumn("PC");
                    ImGui::TableSetupColumn("Opcode");
                    ImGui::TableSetupColumn("Description");
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();
                    if (clear_all_flag) {
                        clear_all_flag = false;
                        std::fill(sel_map_.begin(), sel_map_.end(), false);
                    }
                    ImGuiListClipper clipper;
                    clipper.Begin(0x10000);
                    // TODO: cache disassembly instructions (overhead might not be enough for it to matter however)

                    // Issue #006:
                    // On first Clipper.Step() displaystart and displayend are 0 and 1 no matter how far down we scrolled
                    // This means that if we aren't actually at the top, and instruction 0 uses parameters, the top 1/2 instructions will always
                    // appear as (Parameter). This boolean helps fix this bug on that special edge case.
                    bool imgui_bug = false;
                    int skip = 0;
                    while (clipper.Step()) {
                        if (imgui_bug) {
                            if (clipper.DisplayStart != 1) {
                                // Edge case detected. Set skip to 0 to avoid bug.
                                imgui_bug = false;
                                skip = 0;
                            }
                        }
                        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                            bool is_skipped = false;
                            DisInstr ins = emulator_->GetInstruction(row_n);
                            if (skip == 0) {
                                skip += ins.ParamSize;
                                if (row_n == 0) 
                                    imgui_bug = true;
                            } else {
                                skip--;
                                is_skipped = true;
                            }
                            ImGui::PushID(row_n);
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            if (ImGui::Selectable("$", sel_map_[row_n], ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                                if (emulator_->Paused.load()) {
                                    sel_map_[row_n].flip();
                                    if (sel_map_[row_n]) {
                                        GBBPArguments bp_arg;
                                        bp_arg.PC_using = true;
                                        bp_arg.PC_value = ins.InstructionProgramCode;
                                        emulator_->AddBreakpoint(bp_arg);
                                    }
                                    else {
                                        auto it = std::find_if(
                                            std::execution::par_unseq,
                                            emulator_->Breakpoints.begin(),
                                            emulator_->Breakpoints.end(),
                                            [target = ins.InstructionProgramCode](const GameboyBreakpoint& bp) {
                                                return bp.BPFromTable && bp.Args.PC_value == target;
                                            }
                                        );
                                        if (it != emulator_->Breakpoints.end())
                                            emulator_->Breakpoints.erase(it);
                                    }
                                }
                            }
                            ImGui::SameLine();
                            ImGui::Text("%04X", ins.InstructionProgramCode);
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%02X", ins.Instruction);
                            switch (ins.ParamSize) {
                            case 0:
                                ImGui::SameLine();
                                ImGui::TextUnformatted("     ");
                                break;
                            case 1:
                                ImGui::SameLine();
                                ImGui::Text("%02X   ", ins.Params[0]);
                                break;
                            case 2:
                                ImGui::SameLine();
                                ImGui::Text("%02X,%02X", ins.Params[0], ins.Params[1]);
                                break;
                            }
                            ImGui::TableSetColumnIndex(2);
                            if (!is_skipped){
                                ImGui::TextUnformatted(emulator_->GetCPU().Instructions[ins.Instruction].name.c_str());
                                switch (ins.ParamSize) {
                                    case 1:
                                        ImGui::SameLine();
                                        ImGui::Text("0x%02X", ins.Params[0]);
                                        break;
                                    case 2:
                                        ImGui::SameLine();
                                        ImGui::Text("0x%02X%02X", ins.Params[1], ins.Params[0]);
                                        break;
                                }
                            } else {
                                ImGui::SameLine();
                                ImGui::TextUnformatted("(Parameter)");
                            }
                            ImGui::PopID();
                        }
                    }
                    if (auto inst = emulator_->InstructionBreak.load(); inst != -1) {
                        goto_pc = inst;
                        emulator_->InstructionBreak.store(-1);
                    }
                    if (goto_pc != -1) {
                        Focus(goto_pc);
                    }
                    ImGui::EndTable();
                }
                ImGui::EndChild();
            }
            ImGui::SameLine();
            {
                ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false);
                if (ImGui::BeginTable("bps", 1, flags, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.6f))) {
                    ImGui::TableSetupColumn("Breakpoints");
                    ImGui::TableHeadersRow();

                    ImGuiListClipper clipper;
                    clipper.Begin(emulator_->Breakpoints.size());
                    while (clipper.Step()) {
                        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                            auto& bp = emulator_->Breakpoints[row_n];
                            ImGui::PushID(row_n);
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            bool sel = false;
                            if (row_n == selected_bp)
                                sel = true;
                            if (ImGui::Selectable(bp.GetName().c_str(), sel, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                                if (!sel)
                                    selected_bp = row_n;
                                else
                                    selected_bp = -1;
                            }
                            ImGui::PopID();
                        }
                    }
                    ImGui::EndTable();
                }
                if (ImGui::Button("Add", ImVec2(ImGui::GetContentRegionAvail().x * (1.0f / 3.0f), ImGui::GetContentRegionAvail().y * 0.15f))) {
                    bp_add_popup = true;
                }
                ImGui::SameLine();
                bool disable = false;
                // If a breakpoint isn't selected, disable the "Remove" button
                if (selected_bp == -1) {
                    disable = true;
                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                }
                if (ImGui::Button("Remove", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y * 0.15f))) {
                    if (emulator_->Breakpoints[selected_bp].BPFromTable) {
                        // We have to remove the breakpoint selection from the table too
                        sel_map_[emulator_->Breakpoints[selected_bp].Args.PC_value] = false;
                    }
                    emulator_->Breakpoints.erase(emulator_->Breakpoints.begin() + selected_bp);
                    selected_bp = -1;
                }
                if (disable) {
                    ImGui::PopItemFlag();
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.15f))) {
                    emulator_->Breakpoints.clear();
                    clear_all_flag = true;
                }
                static bool use_hex = true;
                auto& t = emulator_->GetCPU();
                if (use_hex) {
                    ImGui::Text("AF: 0x%02x%02x", t.A, t.F); ImGui::SameLine(); ImGui::Text("PC: 0x%04x", t.PC);
                    ImGui::Text("BC: 0x%02x%02x", t.B, t.C); ImGui::SameLine(); ImGui::Text("SP: 0x%04x", t.SP);
                    ImGui::Text("DE: 0x%02x%02x", t.D, t.E); ImGui::SameLine(); ImGui::Text("IE: 0x%02x", t.IE);
                    ImGui::Text("HL: 0x%02x%02x", t.H, t.L); ImGui::SameLine(); ImGui::Text("IF: 0x%02x", t.IF);
                }
                else {
                    ImGui::Text("AF: %d,%d", t.A, t.F); ImGui::SameLine(); ImGui::Text("PC: %d", t.PC);
                    ImGui::Text("BC: %d,%d", t.B, t.C); ImGui::SameLine(); ImGui::Text("SP: %d", t.SP);
                    ImGui::Text("DE: %d,%d", t.D, t.E); ImGui::SameLine(); ImGui::Text("IE: %d", t.IE);
                    ImGui::Text("HL: %d,%d", t.H, t.L); ImGui::SameLine(); ImGui::Text("IF: %d", t.IF);
                }
                ImGui::Checkbox("Hex", &use_hex);
                ImGui::EndChild();
                // TODO: add switch from hex to binary on every textbox here
                if (bp_add_popup) {
                    ImGui::OpenPopup("Add breakpoint");
                    bp_add_popup = false;
                }
                ImGui::SetNextWindowSize(ImVec2(250, 250));
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal("Add breakpoint", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Configure the breakpoint:");
                    ImGui::Separator();
                    static GBBPArguments bp_arg;
                    {
                        ImGui::BeginChild("bpChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y * 0.9f));
                        breakpoint_register_checkbox("A:", bp_arg.A_value, bp_arg.A_using);
                        breakpoint_register_checkbox("B:", bp_arg.B_value, bp_arg.B_using);
                        breakpoint_register_checkbox("D:", bp_arg.D_value, bp_arg.D_using);
                        breakpoint_register_checkbox("H:", bp_arg.H_value, bp_arg.H_using);
                        breakpoint_register_checkbox("PC:", bp_arg.PC_value, bp_arg.PC_using, ImGuiDataType_U16);
                        breakpoint_register_checkbox("Instr:", bp_arg.Ins_value, bp_arg.Ins_using);
                        ImGui::EndChild();
                    }
                    ImGui::SameLine();
                    {
                        ImGui::BeginChild("bpChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.9f));
                        breakpoint_register_checkbox("F:", bp_arg.F_value, bp_arg.F_using);
                        breakpoint_register_checkbox("C:", bp_arg.C_value, bp_arg.C_using);
                        breakpoint_register_checkbox("E:", bp_arg.E_value, bp_arg.E_using);
                        breakpoint_register_checkbox("L:", bp_arg.L_value, bp_arg.L_using);
                        breakpoint_register_checkbox("SP:", bp_arg.SP_value, bp_arg.SP_using, ImGuiDataType_U16);
                        ImGui::EndChild();
                    }
                    if (ImGui::Button("Add", ImVec2(120, 0))) {
                        bool pc_only = emulator_->AddBreakpoint(bp_arg);
                        if (pc_only) {
                            sel_map_[bp_arg.PC_value] = true;
                        }
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            }
        }
    private:
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
