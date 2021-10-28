#ifndef TKP_DISASSEMBLER_H
#define TKP_DISASSEMBLER_H
#include <vector>
#include "base_application.h"
namespace TKPEmu::Applications {
    class Disassembler : public IMApplication {
    public:
        bool Loaded = false;
        std::vector<DisInstr> Instrs;
        Disassembler(bool* rom_loaded) : IMApplication(rom_loaded) {}
        void Focus(int item) noexcept {
            ImGuiContext& g = *ImGui::GetCurrentContext();
            ImGuiWindow* window = g.CurrentWindow;
            // TODO: find a way to make item_height not hard coded
            static const int item_height = 17;
            static const int offset_y = 16;
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
            int goto_pc = -1;
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Navigation")) {
                    if (ImGui::MenuItem("Step")) {
                        if (emulator_->Paused.load())
                          emulator_->Step.store(true);
                    }
                    if (ImGui::MenuItem("Pause", NULL, emulator_->Paused.load())) {
                        emulator_->Paused.store(!emulator_->Paused.load());
                    }
                    if (ImGui::MenuItem("Stop")) {
                        *rom_loaded_ = false;
                        emulator_->Stopped.store(true, std::memory_order_seq_cst);
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
                if (ImGui::BeginMenu("Breakpoints")) {

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            if (goto_popup) {
                ImGui::OpenPopup("goto");
                goto_popup = false;
            }
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("goto", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
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
            
            static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
            {
                ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), false);
                if (ImGui::BeginTable("table_advanced", 3, flags)) {
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
            {
                //if (ImGui::BeginTable("table_advanced", 1, flags)) {
                //    ImGui::TableSetupColumn("Breakpoints");
                //    ImGui::TableHeadersRow();
                //}
            }
            ImGui::End();
        }
    };
}
#endif