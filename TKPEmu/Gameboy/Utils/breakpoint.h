#pragma once
#ifndef TKP_TOOLS_GBBP_H
#define TKP_TOOLS_GBBP_H
namespace TKPEmu::Gameboy::Utils {
	struct GameboyBreakpoint {
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
	};
}
#endif