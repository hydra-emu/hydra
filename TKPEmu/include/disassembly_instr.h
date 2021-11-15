#pragma once
#ifndef TKP_GB_DISASSEMBLY_INSTR_H
#define TKP_GB_DISASSEMBLY_INSTR_H
#include <string>
#include <sstream>
#include <iomanip>
namespace TKPEmu::Tools {
	// TODO: instead of using uint16_t and 8_t, make this a template class and generalize it,
	// params should take more than 2 etc..
	// TODO: remove "ID"
	struct DisInstr {
		DisInstr(uint16_t iPC, uint8_t ins, uint8_t skip) : 
			InstructionProgramCode(iPC), 
			Instruction(ins), 
			ParamSize(skip) 
		{}
		DisInstr(const DisInstr&) = delete;
		DisInstr& operator=(const DisInstr&) = delete;
		DisInstr(DisInstr&&) = default;
		DisInstr& operator=(DisInstr&&) = default;
		uint16_t InstructionProgramCode = 0;
		uint8_t Instruction = 0;
		uint8_t Params[2]{};
		uint8_t ParamSize = 0;
		bool Selected = false;
		static int s_id;
	private:
	};
}
#endif
