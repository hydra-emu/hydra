#include "disassembler.h"
#include <sstream>

std::string disassembler::Disassemble(const std::vector<CPU::Instruction>& instr, int PC) {
	std::stringstream ss;
	int j = 0;
	for (CPU::Instruction i : instr) {
		ss << std::uppercase << std::hex << i.PC << ":";
		ss << i.name;
		int diff = 7 - i.name.length();
		for (int i = 0; i < diff; i++)
			ss << " ";
		if (i.byte1 != 0)
			ss << " 0x" << std::uppercase << std::hex << i.byte1;
		if (i.byte2 != 0)
			ss << " 0x" << std::uppercase << std::hex << i.byte2;
		ss << "\n";
	}
	return ss.str();
}
