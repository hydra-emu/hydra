/**
 * This is a generic disassembler class to replace the gb disassembler
 * and hopefully to easily work later for other emulators.
 */
#pragma once
#ifndef TKP_GENERIC_DISASSEMBLER_H
#define TKP_GENERIC_DISASSEMBLER_H
#include <cstdint>
#include <memory>
#include <vector>
#include <array>
#include "emulator.h"
#include "generic_drawable.h"

namespace TKPEmu::Applications {
    namespace {
        std::array<const char*, 0x100> hex_lut =
        {
            "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
            "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
            "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
            "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
            "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
            "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
            "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
            "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
            "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
            "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
            "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
            "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
            "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
            "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
            "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
            "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF",
        };
    }
    struct GenericInstruction {
        std::vector<std::string> disasm;
        std::string pc;
    };
    template <class InstructionSize = uint8_t> 
    class GameboyInstruction {
    public:
        using size_type = InstructionSize;
    protected:
        constexpr int get_instr_size(InstructionSize instr) {
            return sizeof(InstructionSize) + GameboyInstruction::instr_size_map[instr] * sizeof(InstructionSize);
        }
    private:
        constexpr static int instr_size_map[0x100] = {
            0, 2, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 1, 0,
            0, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0,
            1, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0,
            1, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 2, 2, 2, 0, 1, 0, 0, 0, 2, 1, 2, 2, 2, 0,
            0, 0, 2, 0, 2, 0, 1, 0, 0, 0, 2, 0, 2, 0, 1, 0,
            1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 2, 0, 0, 0, 1, 0,
            1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 2, 0, 0, 0, 1, 0,
        };
    };
    template <class InstructionSize>
    class FixedSizeInstruction {
    protected:
        constexpr int get_instr_size(InstructionSize instr) {
            return sizeof(InstructionSize);
        }
    public:
        using size_type = InstructionSize;
    };
    template <
        class InstructionPolicy
    >
    class Disassembler : public Drawable, public InstructionPolicy {
    public:
        Disassembler(std::shared_ptr<Emulator> emulator) : emulator_(emulator) {}
        void Draw() override {
            static bool view_hex = true;
            if (emulator_ && drawing_) {
                if (!loaded_instrs_) {
                    load_instrs();
                }
                ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
                if (ImGui::Begin(window_title_, IsDrawing())) {
                    float mult = view_hex ? 0.6f : 1.0f;
                    auto left_size = ImVec2(ImGui::GetContentRegionAvail().x * mult, 0);
                    ImGui::BeginChild("left pane disasm", left_size, true);
                    for (int i = 0; i < 300; i++) {
                        draw_instr(instrs_[i].disasm);
                    }
                    ImGui::EndChild();
                    if (view_hex) {
                        ImGui::SameLine();
                        ImGui::BeginChild("right pane disasm", ImVec2(0, 0), true);
                        
                        ImGui::EndChild();
                    }
                    ImGui::End();
                }
            }
        }
        void Reset() override {
            emulator_.reset();
        }
        bool* IsDrawing() override {
            return &drawing_;
        }
        const char* GetName() override {
            return window_title_;
        }
    private:
        std::shared_ptr<Emulator> emulator_;
        constexpr static const char* window_title_ = "Disassembler";
        bool drawing_ = false;
        bool loaded_instrs_ = false;
        std::vector<GenericInstruction> instrs_;
        // TODO: add "must reload" bool for when code changed
        void load_instrs() {
            uint64_t nops = 0;
            instrs_.clear();
            const auto& rom_vec = emulator_->GetRomData();
            for (size_t i = 0; i < emulator_->GetRomSize();) {
                std::string cur_instr_str;
                auto cur_instr = rom_vec[i];
                auto start_i = i;
                for (int j = 0; j < InstructionPolicy::get_instr_size(cur_instr); ++j) {
                    cur_instr_str += hex_lut[rom_vec[start_i + j]];
                    ++i;
                }
                auto vec = emulator_->Disassemble(cur_instr_str);
                if (vec.size() == 1 && vec[0] == "NOP") {
                    ++nops;
                    continue;
                } else if (nops > 1) {
                    std::vector<std::string> vec_nops;
                    vec_nops.push_back("NOPS");
                    vec_nops.push_back(std::to_string(nops));
                    GenericInstruction gi;
                    gi.disasm = std::move(vec_nops);
                    gi.pc = "";
                    instrs_.push_back(gi);
                    nops = 0;
                } else if (nops == 1) {
                    std::vector<std::string> vec_nops;
                    vec_nops.push_back("NOP");
                    GenericInstruction gi;
                    gi.disasm = std::move(vec_nops);
                    gi.pc = std::to_string(i);
                    instrs_.push_back(gi);
                    nops = 0;
                }
                GenericInstruction gi;
                gi.disasm = std::move(vec);
                gi.pc = std::to_string(i);
                instrs_.push_back(gi);
            }
            loaded_instrs_ = true;
        }
        void draw_instr(const std::vector<std::string>& instr) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(122, 140, 6, 255));
            ImGui::TextUnformatted(instr[0].c_str());
            ImGui::PopStyleColor();
            for (size_t i = 1; i < instr.size(); ++i) {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(41, 157, 148, 255));
                ImGui::TextUnformatted(instr[i].c_str());
                ImGui::PopStyleColor();
            }
        }
        // Seek to the instruction with a given PC
        // Used by goto instr or when a breakpoint is hit
        void seek(std::string pc) {

        }
    };
}
#endif