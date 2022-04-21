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
        static std::string GetOpcodeName(EmuType type, std::any instruction) {
            std::string ret;
            switch (type) {
                case EmuType::N64: {
                    uint32_t instr_inner = std::any_cast<uint32_t>(instruction);
                    TKPEmu::N64::Instruction instr;
                    instr.Full = instr_inner;
                    if (instr.Full == 0) {
                        ret = "nop";
                    } else if (instr.Full == 0xFFFFFFFF) {
                        ret = "   ";
                    } else {
                        switch(instr.IType.op) {
                            case 0: {
                                ret = TKPEmu::N64::SpecialCodes[instr.RType.func];
                                break;
                            }
                            default: {
                                ret = TKPEmu::N64::OperationCodes[instr.IType.op];
                            }
                        }
                    }
                    break;
                }
                default: {
                    ret = "Unknown emulator type - GetOpcodeName";
                    break;
                }
            }
            return ret;
        }
        static std::string GetDisassembledString(EmuType type, std::any instruction) {
            std::string ret;
            switch (type) {
                case EmuType::N64: {
                    uint32_t instr_inner = std::any_cast<uint32_t>(instruction);
                    TKPEmu::N64::Instruction instr;
                    instr.Full = instr_inner;
                    std::stringstream ss;
                    ss << GetOpcodeName(type, instr.Full);
                    if (instr_inner != 0 && instr_inner != -1) {
                        if (instr.IType.op != 0) {
                            ss << " $r" << std::hex << instr.IType.rs;
                            ss << " $r" << std::hex << instr.IType.rt;
                            ss << " 0x" << std::hex << instr.IType.immediate;
                        } else {
                            ss << " $r" << std::hex << instr.RType.rs;
                            ss << " $r" << std::hex << instr.RType.rt;
                            ss << " $r" << std::hex << instr.RType.rd;
                            ss << " 0x" << std::hex << instr.RType.sa;
                        }
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