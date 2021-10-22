#ifndef TKP_GB_DISASSEMBLY_INSTR_H
#define TKP_GB_DISASSEMBLY_INSTR_H
#include <string>
#include <sstream>
#include <iomanip>
namespace TKPEmu::Tools {
	// TODO: instead of using uint16_t and 8_t, make this a template class and generalize it,
	// params should take more than 2 etc..
	struct DisInstr {
		DisInstr(uint16_t IPC, uint8_t Ins) {
			ID = get_id();
			InstructionProgramCode = IPC;
			Instruction = Ins;
			InstructionPCHex = int_to_hex(IPC, 4);
			InstructionHex = int_to_hex(Ins, 2);
		}

		int ID = 0;
		uint16_t InstructionProgramCode = 0;
		uint8_t Instruction = 0;
		uint8_t Params[2]{ 0, 0 };

		std::string InstructionPCHex;
		std::string InstructionHex;
		std::string InstructionFull;

		bool Selected = false;

	private:
		int get_id() {
			static int id = 0;
			id++;
			return id;
		}
		template<typename T>
		inline std::string int_to_hex(T val, size_t width = sizeof(T) * 2)
		{
			std::stringstream ss;
			ss << std::setfill('0') << std::setw(width) << std::uppercase << std::hex << (val | 0);
			return ss.str();
		}
	};
}
#endif