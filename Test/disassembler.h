#ifndef disassembler_H
#define disassembler_H

#include <vector>
#include <string>

#include "cpu.h"

class disassembler
{
public:
	std::string Disassemble(const std::vector<CPU::Instruction>& instr, int PC);
};
#endif
