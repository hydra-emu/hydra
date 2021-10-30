#pragma once
#ifndef TKP_TOOLS_GBBP_H
#define TKP_TOOLS_GBBP_H
namespace TKPEmu::Gameboy::Utils {
	struct GameboyBreakpoint {
	public:
		// TODO: make into std::pair<bool, uint16_t> and an array that contains them
		using BreakFunction = std::function<bool()>;
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

		bool breakpoint_from_table = false;
		void SetChecks(BreakFunction&& func) {
			checks = std::move(func);
		}
		bool Check() const {
			return checks();
		}
		const std::string& GetName() {
			if (!loaded) {
				std::stringstream ss;
				ss << std::setfill('0');
				if (A_using) { ss << "A=" << std::hex << std::setw(2) << A_value << "&&"; }
				if (B_using) { ss << "B=" << std::hex << std::setw(2) << B_value << "&&"; }
				if (C_using) { ss << "C=" << std::hex << std::setw(2) << C_value << "&&"; }
				if (D_using) { ss << "D=" << std::hex << std::setw(2) << D_value << "&&"; }
				if (E_using) { ss << "E=" << std::hex << std::setw(2) << E_value << "&&"; }
				if (F_using) { ss << "F=" << std::hex << std::setw(2) << F_value << "&&"; }
				if (H_using) { ss << "H=" << std::hex << std::setw(2) << H_value << "&&"; }
				if (L_using) { ss << "L=" << std::hex << std::setw(2) << L_value << "&&"; }
				if (PC_using) { ss << "PC=" << std::hex << std::setw(4) << PC_value << "&&"; }
				if (SP_using) { ss << "SP=" << std::hex << std::setw(4) << SP_value << "&&"; }
				if (Ins_using) { ss << "Ins=" << std::hex << std::setw(2) << Ins_value << "&&"; }
				ss.seekp(-2, ss.cur);
				ss << "  ";
				name = ss.str();
				loaded = true;
			}
			return name;
		}
	private:
		bool loaded = false;
		std::string name = "error-breakpoint";
		BreakFunction checks;
	};
}
#endif