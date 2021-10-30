#pragma once
#ifndef TKP_DISASSEMBLER_H
#define TKP_DISASSEMBLER_H
#include <vector>
#include <algorithm>
#include <execution>
#include "base_application.h"
// TODO: make this disassembler the gameboy disassembler, and make a generic disassembler class
#include "../../Gameboy/Utils/breakpoint.h"
#include "../../Gameboy/gameboy.h"
#define WRAM_START 0xC000
namespace TKPEmu::Applications { 
    class Disassembler : public IMApplication {
    private:
        using GameboyBreakpoint = TKPEmu::Gameboy::Utils::GameboyBreakpoint;
        using Gameboy = TKPEmu::Gameboy::Gameboy;
        Gameboy* emulator_ = nullptr;
        GameboyBreakpoint debug_rvalues_;
        bool clear_all_flag = false;
    public:
        std::atomic_bool Loaded = false;
        std::vector<DisInstr> Instrs;
        Disassembler(bool* rom_loaded) : IMApplication(rom_loaded) {}
        void Focus(int item) noexcept {
            ImGuiContext& g = *ImGui::GetCurrentContext();
            ImGuiWindow* window = g.CurrentWindow;
            // TODO: find a way to make item_height not hard coded
            static const int item_height = 17;
            window->Scroll.y = IM_FLOOR(item_height * item);
        }
        void Reset() noexcept final override {
            DisInstr::ResetId();
            Loaded = false;
        }
        void Draw(const char* title, bool* p_open = NULL) noexcept final override {
            if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar)) {
                ImGui::End();
                return;
            }
            bool goto_popup = false;
            bool bp_add_popup = false;
            int goto_pc = -1;
            int selected_bp = -1;
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Emulation")) {
                    DrawMenuEmulation(emulator_, rom_loaded_);
                }
                if (ImGui::BeginMenu("Navigation")) {
                    if (ImGui::MenuItem("Step", NULL, false, emulator_->Paused.load())) {
                        emulator_->Step.store(true);
                        emulator_->Step.notify_all();
                    }
                    if (ImGui::MenuItem("Reset")) {
                        // Sets the stopped flag on the thread to true and then waits for it to become false
                        // The thread sets the flag to false upon exiting
                        ResetEmulatorState(emulator_);
                        emulator_->StartDebug();
                    }
                    if (ImGui::MenuItem("Goto PC")) {
                        goto_popup = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }      
            if (goto_popup) {
                ImGui::OpenPopup("Goto Program Code");
                goto_popup = false;
            }
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Goto Program Code", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Program Code (in hex) to go to:");
                ImGui::Separator();
                static char buf[16] = "";
                ImGui::InputText("hexadecimal", buf, static_cast<size_t>(emulator_->GetPCHexCharSize()) + 1, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    unsigned x = 0;
                    std::stringstream ss;
                    ss << std::hex << buf;
                    ss >> x;
                    SetGotoPC(goto_pc, x);
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
                        for (auto& i : Instrs) {
                            i.Selected = false;
                        }
                    }
                    ImGuiListClipper clipper;
                    clipper.Begin(Instrs.size());
                    while (clipper.Step()) {
                        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                            DisInstr* ins = &Instrs[row_n];
                            ImGui::PushID(ins->ID);
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            if (ImGui::Selectable("$", ins->Selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                                if (emulator_->Paused.load()) {
                                    ins->Selected ^= true;
                                    if (ins->Selected) {
                                        GameboyBreakpoint bp;
                                        bp.PC_using = true;
                                        bp.PC_value = ins->InstructionProgramCode;
                                        bp.breakpoint_from_table = true;
                                        emulator_->AddBreakpoint(bp);
                                    }
                                    else {
                                        auto it = std::find_if(
                                            std::execution::par_unseq,
                                            emulator_->Breakpoints.begin(),
                                            emulator_->Breakpoints.end(),
                                            [target = ins->InstructionProgramCode](GameboyBreakpoint bp) {
                                            return bp.breakpoint_from_table && bp.PC_value == target;
                                        }
                                        );
                                        emulator_->Breakpoints.erase(it);
                                    }
                                }
                            }
                            ImGui::SameLine();
                            ImGui::Text("%04X", ins->InstructionProgramCode);
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%02X", ins->Instruction);
                            switch (ins->ParamSize) {
                            case 0:
                                ImGui::SameLine();
                                ImGui::TextUnformatted("     ");
                                break;
                            case 1:
                                ImGui::SameLine();
                                ImGui::Text("%02X   ", ins->Params[0]);
                                break;
                            case 2:
                                ImGui::SameLine();
                                ImGui::Text("%02X,%02X", ins->Params[0], ins->Params[1]);
                                break;
                            }
                            ImGui::TableSetColumnIndex(2);
                            ImGui::TextUnformatted(emulator_->GetCPU().instructions[ins->Instruction].name.c_str());
                            switch (ins->ParamSize) {
                            case 1:
                                ImGui::SameLine();
                                ImGui::Text("0x%02X", ins->Params[0]);
                                break;
                            case 2:
                                ImGui::SameLine();
                                ImGui::Text("0x%02X%02X", ins->Params[1], ins->Params[0]);
                                break;
                            }
                            if (ins->InstructionProgramCode > WRAM_START) {
                                ImGui::SameLine();
                                ImGui::TextColored(ImVec4(128, 128, 0, 255), "(!)");
                                if (ImGui::IsItemHovered()) {
                                    ImGui::BeginTooltip();
                                    // TODO: find a way to reload instructions in real time
                                    ImGui::TextUnformatted("WRAM instruction loading not implemented yet!");
                                    ImGui::EndTooltip();
                                }
                            }
                            ImGui::PopID();
                        }
                    }
                    if (emulator_->Break.load()) {
                        emulator_->Break.store(false);
                        if (auto inst = emulator_->InstructionBreak.load(); inst != -1) {
                            SetGotoPC(goto_pc, inst);
                            emulator_->InstructionBreak.store(-1);
                        }
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
                            if (ImGui::Selectable(bp.GetName().c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                                // TODO: Create breakpoint upon selecting
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
                if (ImGui::Button("Remove", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y * 0.15f))) {

                }
                ImGui::SameLine();
                if (ImGui::Button("Clear", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.15f))) {
                    emulator_->Breakpoints.clear();
                    clear_all_flag = true;
                }
                static GameboyBreakpoint db_reg;
                if (emulator_->Paused.load()) {
                    emulator_->CopyRegToBreakpoint(db_reg);
                }
                std::stringstream ss;
                ss << std::hex << std::setfill('0') 
                    << "AF: " << std::setw(2) << db_reg.A_value << std::setw(2) << db_reg.F_value << "   BC: " << std::setw(2) << db_reg.B_value << std::setw(2) << db_reg.C_value << "\n"
                    << "DE: " << std::setw(2) << db_reg.D_value << std::setw(2) << db_reg.E_value << "   HL: " << std::setw(2) << db_reg.H_value << std::setw(2) << db_reg.L_value << "\n"
                    << "PC: " << std::setw(4) << db_reg.PC_value << "\n"
                    << "SP: " << std::setw(4) << db_reg.SP_value << "\n";
                ImGui::Text(ss.str().c_str());
                ImGui::EndChild();
                // TODO: add switch from hex to binary on every textbox here
                if (bp_add_popup) {
                    ImGui::OpenPopup("Add breakpoint");
                    bp_add_popup = false;
                }
                ImGui::SetNextWindowSize(ImVec2(400, 400));
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal("Add breakpoint", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    static GameboyBreakpoint gbbp;
                    ImGui::Text("Configure the breakpoint:");
                    ImGui::Separator();
                    {
                        ImGui::BeginChild("bpChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y * 0.75f));
                        breakpoint_register_checkbox(gbbp.A_value, gbbp.A_using, "A");
                        breakpoint_register_checkbox(gbbp.Ins_value, gbbp.Ins_using, "Ins");
                        ImGui::EndChild();
                    }
                    ImGui::SameLine();
                    {
                        ImGui::BeginChild("bpChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.75f));
                        ImGui::EndChild();
                    }
                    if (ImGui::Button("Add", ImVec2(120, 0))) {
                        emulator_->AddBreakpoint(gbbp);
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            }
            ImGui::End();
        }
        void SetEmulator(Gameboy* emulator) {
            emulator_ = emulator;
        }
        auto FindByPC(int target) {
            return std::find_if(
                std::execution::par_unseq,
                Instrs.begin(),
                Instrs.end(),
                [&target](DisInstr ins) {
                    return ins.InstructionProgramCode == target;
                }
            );
        }
        void SetGotoPC(int& goto_pc, int target) {
            auto it = FindByPC(target);
            goto_pc = it - Instrs.begin();
        }
        static void DrawMenuEmulation(Emulator* emulator, bool* rom_loaded) {
            if (!*rom_loaded) {
                ImGui::MenuItem("Pause", NULL, false, *rom_loaded);
            }
            else {
                if (ImGui::MenuItem("Pause", NULL, emulator->Paused.load(), *rom_loaded)) {
                    emulator->Paused.store(!emulator->Paused.load());
                    emulator->Step.store(true);
                    emulator->Step.notify_all();
                }
            }
            if (ImGui::MenuItem("Stop", 0, false, *rom_loaded)) {
                *rom_loaded = false;
                ResetEmulatorState(emulator);
            }
            ImGui::EndMenu();
        }
        static void ResetEmulatorState(Emulator* emulator) {
            emulator->Step.store(true);
            emulator->Paused.store(false);
            emulator->Stopped.store(true);
            emulator->Step.notify_all();
        }
    private:
        template<typename T>
        void breakpoint_register_checkbox(T& value, bool& is_used, std::string str) {
            ImGui::Checkbox(str.c_str(), &is_used);
            ImGui::SameLine();
            if (is_used) {
                // TODO: set to type max
                ImGui::DragInt("drag int", (int*)(&value), 1, 0, 255, NULL);
            }
        }
    };
}
#endif