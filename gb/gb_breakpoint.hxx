#pragma once
#ifndef TKP_TOOLS_GBBP_H
#define TKP_TOOLS_GBBP_H
#include <functional>
#include <string>

namespace hydra::Gameboy::Utils {
    struct GBBPArguments {
        bool A_using = false; uint16_t A_value = 0;
        bool B_using = false; uint16_t B_value = 0;
        bool C_using = false; uint16_t C_value = 0;
        bool D_using = false; uint16_t D_value = 0;
        bool E_using = false; uint16_t E_value = 0;
        bool F_using = false; uint16_t F_value = 0;
        bool H_using = false; uint16_t H_value = 0;
        bool L_using = false; uint16_t L_value = 0;
        bool PC_using = false; uint16_t PC_value = 0;
        bool SP_using = false; uint16_t SP_value = 0;
        bool Ins_using = false; uint16_t Ins_value = 0;
        bool Clocks_using = false; unsigned long long Clocks_value;
    };
    struct GameboyBreakpoint {
    public:
        using BreakFunction = std::function<bool()>;
        bool Loaded = false;
        bool BPFromTable = true;
        GBBPArguments Args;
        GameboyBreakpoint() = default;
        GameboyBreakpoint(const GameboyBreakpoint&) = delete;
        GameboyBreakpoint& operator=(const GameboyBreakpoint&) = delete;
        GameboyBreakpoint(GameboyBreakpoint&&) = default;
        GameboyBreakpoint& operator=(GameboyBreakpoint&&) = default;
        void SetChecks(BreakFunction func) {
            checks = std::move(func);
        }
        bool Check() const {
            return checks();
        }
        const std::string& GetName() {
            if (!Loaded) {
                std::stringstream ss;
                ss << std::setfill('0');
                if (Args.A_using) { ss << "A=" << std::hex << std::setw(2) << Args.A_value << "&&"; BPFromTable = false; }
                if (Args.B_using) { ss << "B=" << std::hex << std::setw(2) << Args.B_value << "&&"; BPFromTable = false; }
                if (Args.C_using) { ss << "C=" << std::hex << std::setw(2) << Args.C_value << "&&"; BPFromTable = false; }
                if (Args.D_using) { ss << "D=" << std::hex << std::setw(2) << Args.D_value << "&&"; BPFromTable = false; }
                if (Args.E_using) { ss << "E=" << std::hex << std::setw(2) << Args.E_value << "&&"; BPFromTable = false; }
                if (Args.F_using) { ss << "F=" << std::hex << std::setw(2) << Args.F_value << "&&"; BPFromTable = false; }
                if (Args.H_using) { ss << "H=" << std::hex << std::setw(2) << Args.H_value << "&&"; BPFromTable = false; }
                if (Args.L_using) { ss << "L=" << std::hex << std::setw(2) << Args.L_value << "&&"; BPFromTable = false; }
                if (Args.PC_using) { ss << "PC=" << std::hex << std::setw(4) << Args.PC_value << "&&"; }
                if (Args.SP_using) { ss << "SP=" << std::hex << std::setw(4) << Args.SP_value << "&&"; BPFromTable = false; }
                if (Args.Ins_using) { ss << "Ins=" << std::hex << std::setw(2) << Args.Ins_value << "&&"; BPFromTable = false; }
                if (Args.Clocks_using) { ss << "Clocks=" << Args.Clocks_value << "&&"; BPFromTable = false; }
                ss.seekp(-2, ss.cur);
                ss << "  ";
                name = ss.str();
                Loaded = true;
            }
            return name;
        }
    private:
        std::string name = "error-breakpoint";
        BreakFunction checks;
    };
}
#endif
