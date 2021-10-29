#ifndef TKP_DISASSEMBLER_H
#define TKP_DISASSEMBLER_H
#include <vector>
#include "base_application.h"
// TODO: make this disassembler the gameboy disassembler, and make a generic disassembler class
#include "../../Gameboy/Utils/breakpoint.h"
#include "../../Gameboy/gameboy.h"
namespace TKPEmu::Applications { 
    class Disassembler : public IMApplication {
    private:
        using GameboyBreakpoint = TKPEmu::Gameboy::Utils::GameboyBreakpoint;
        using Gameboy = TKPEmu::Gameboy::Gameboy;
    public:
        bool Loaded = false;
        std::vector<DisInstr> Instrs;
        Disassembler(bool* rom_loaded) : IMApplication(rom_loaded) {}
        void Focus(int item) noexcept {
            ImGuiContext& g = *ImGui::GetCurrentContext();
            ImGuiWindow* window = g.CurrentWindow;
            // TODO: find a way to make item_height not hard coded
            static const int item_height = 17;
            static const int offset_y = 24;
            window->Scroll.y = IM_FLOOR(item_height * item + offset_y);
        }
        void Reset() noexcept final override {
            DisInstr::ResetId();
            Instrs.clear();
            Loaded = false;
        }
        void Draw(const char* title, bool* p_open = NULL) noexcept final override {
            // TODO: draw "Loading..." window if caught waiting
            if (!Loaded)
                return;
            if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar)) {
                ImGui::End();
                return;
            }
            bool goto_popup = false;
            bool bp_add_popup = false;
            int goto_pc = -1;
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Emulation")) {
                    DrawMenuEmulation(emulator_, rom_loaded_);
                }
                if (ImGui::BeginMenu("Navigation")) {
                    if (ImGui::MenuItem("Step")) {
                        if (emulator_->Paused.load())
                            emulator_->Step.store(true);
                    }
                    if (ImGui::MenuItem("Reset")) {
                        // Sets the stopped flag on the thread to true and then waits for it to become false
                        // The thread sets the flag to false upon exiting
                        emulator_->Stopped.store(true, std::memory_order_seq_cst);
                        emulator_->StartDebug();
                    }
                    if (ImGui::MenuItem("Goto PC")) {
                        goto_popup = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }      
            static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
            {
                ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, ImGui::GetContentRegionAvail().y), true);
                if (ImGui::BeginTable("cmds", 3, flags)) {
                    ImGui::TableSetupColumn("PC");
                    ImGui::TableSetupColumn("Instruction");
                    ImGui::TableSetupColumn("Description");
                    ImGui::TableHeadersRow();
                }
                // TODO: implement breakpoint adder
                if (emulator_->Break.load()) {
                    emulator_->Break.store(false);
                    Focus(emulator_->InstructionBreak.load());
                }
                if (goto_pc != -1) {
                    Focus(goto_pc);
                }
                ImGuiListClipper clipper;
                clipper.Begin(Instrs.size());
                while (clipper.Step())
                {
                    for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                        DisInstr* ins = &Instrs[row_n];
                        ImGui::PushID(ins->ID);
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        if (ImGui::Selectable(ins->InstructionPCHex.c_str(), ins->Selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                            // TODO: Create breakpoint upon selecting
                        }
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(ins->InstructionHex.c_str());
                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(ins->InstructionFull.c_str());
                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
                ImGui::EndChild();
            }
            ImGui::SameLine();
            {
                ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.75f), true);
                if (ImGui::BeginTable("bps", 1, flags, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.9f))) {
                    ImGui::TableSetupColumn("Breakpoints");
                    ImGui::TableHeadersRow();
                }
                ImGuiListClipper clipper;
                clipper.Begin(emulator_->Breakpoints.size());
                while (clipper.Step()) {
                    for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                        ImGui::PushID(row_n);
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);

                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
                if (ImGui::Button("Add", ImVec2(ImGui::GetContentRegionAvail().x * (1.0f / 3.0f), ImGui::GetContentRegionAvail().y))) {
                    bp_add_popup = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("Remove", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y))) {

                }
                ImGui::SameLine();
                if (ImGui::Button("Clear", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y))) {
                    emulator_->Breakpoints.clear();
                }
                ImGui::EndChild();
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
                static char buf3[64] = ""; ImGui::InputText("hexadecimal", buf3, static_cast<size_t>(emulator_->GetPCHexCharSize()) + 1, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    unsigned x;
                    std::stringstream ss;
                    ss << std::hex << buf3;
                    ss >> x;
                    int ind = 0;
                    for (auto& k : Instrs) {
                        if (k.InstructionProgramCode == x) {
                            goto_pc = ind;
                            break;
                        }
                        ind++;
                    }
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
            if (bp_add_popup) {
                ImGui::OpenPopup("Add breakpoint");
                bp_add_popup = false;
            }
            // TODO: add switch from hex to binary on every textbox here
            ImGui::SetNextWindowSize(ImVec2(200, 200));
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Add breakpoint", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                static GameboyBreakpoint gbbp{
                    .PC_using = true,
                    .PC_value = 0x100
                };
                ImGui::Text("Configure the breakpoint:");
                ImGui::Separator();
                {
                    ImGui::BeginChild("bpChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, ImGui::GetContentRegionAvail().y * 0.75f));
                    breakpoint_register_checkbox(gbbp.A_value, gbbp.A_using);
                    ImGui::EndChild();
                }
                ImGui::SameLine();
                {
                    ImGui::BeginChild("bpChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.75f));
                    ImGui::EndChild();
                }
                if (ImGui::Button("Add", ImVec2(120, 0))) {
                    dynamic_cast<Gameboy&>(*emulator_).AddBreakpoint(gbbp);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
            ImGui::End();
        }
        static void DrawMenuEmulation(Emulator* emulator, bool* rom_loaded) {
            if (!*rom_loaded) {
                ImGui::MenuItem("Pause", NULL, false, *rom_loaded);
            }
            else {
                if (ImGui::MenuItem("Pause", NULL, emulator->Paused.load(), *rom_loaded)) {
                    emulator->Paused.store(!emulator->Paused.load());
                    emulator->Paused.notify_all();
                }
            }
            if (ImGui::MenuItem("Stop", 0, false, *rom_loaded)) {
                *rom_loaded = false;
                emulator->Stopped.store(true, std::memory_order_seq_cst);
            }
            ImGui::EndMenu();
        }
    private:
        template<typename T>
        void breakpoint_register_checkbox(T& value, bool& is_used) {
            ImGui::Checkbox("A", &is_used);
            ImGui::SameLine();
            if (is_used) {
                ImGui::DragInt("drag int", (int*)(&value), 1, 0, 255, NULL);
            }
        }
    };
}
#endif