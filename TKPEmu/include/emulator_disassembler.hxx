#pragma once
#ifndef TKP_EMULATORDIS_H
#define TKP_EMULATORDIS_H
#include <string>
#include <sstream>
#include <any>
#include "../N64TKP/core/n64_types.hxx"
#include "emulator_types.hxx"

namespace TKPEmu {
    struct GeneralDisassembler {
        static std::string Disassemble(EmuType type, std::any instruction) {
            std::string ret;
            switch (type) {
                case EmuType::N64: {
                    uint32_t instr_inner = std::any_cast<uint32_t>(instruction);
                    TKPEmu::N64::Instruction instr;
                    instr.Full = instr_inner;
                    std::stringstream ss;
                    if (instr.IType.op != 0) {
                        ss << TKPEmu::N64::OperationCodes[instr.IType.op];
                        ss << " " << std::hex << instr.IType.rs;
                        ss << " " << std::hex << instr.IType.rt;
                        ss << " " << std::hex << instr.IType.immediate;
                    } else {
                        ss << TKPEmu::N64::SpecialCodes[instr.RType.func];
                        ss << " " << std::hex << instr.RType.rs;
                        ss << " " << std::hex << instr.RType.rt;
                        ss << " " << std::hex << instr.RType.rd;
                        ss << " " << std::hex << instr.RType.sa;
                    }
                    ret = ss.str();
                    break;
                }
                default: {
                    ret = "Bad emulator type";
                    break;
                }
            }
            return ret;
        }
    };
}
#endif