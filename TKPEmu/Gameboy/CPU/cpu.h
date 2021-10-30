#pragma once
#ifndef TKP_GB_CPU_H
#define TKP_GB_CPU_H
#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <iomanip>
#include <fstream>
#include "../Bus/bus.h"
#include "../PPU/ppu.h"
#include "../Utils/cpu_const.h"
#include "../../Tools/disassembly_instr.h"
namespace TKPEmu::Gameboy::Devices {
	using namespace TKPEmu::Gameboy::Utils;
	class CPU {
	private:
		using RegisterType = uint16_t;
	private:
		int bios[256] = {
			0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
			0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
			0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
			0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
			0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
			0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
			0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
			0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
			0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xF2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
			0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
			0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
			0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
			0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
			0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x4C,
			0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
			0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50,
		};

		int R;
		int mTemp;
		int tTemp;

		bool halt;
		bool stop;

		// Instruction functions
		void NOP(); void LDBC16(); void LDBCA(); void INCBC(); void INCB(); void DECB(); void LDB8(); void RLCA(); void LD16SP(); void ADDHLBC(); void LDABC(); void DECBC(); void INCC(); void DECC(); void LDC8(); void RRCA();
		void STOP(); void LDDE16(); void LDDEA(); void INCDE(); void INCD(); void DECD(); void LDD8(); void RLA(); void JR8(); void ADDHLDE(); void LDADE(); void DECDE(); void INCE(); void DECE(); void LDE8(); void RRA();
		void JRNZ8(); void LDHL16(); void LDIHLA(); void INCHL(); void INCH(); void DECH(); void LDH8(); void DAA(); void JRZ8(); void ADDHLHL(); void LDIAHL(); void DECHL(); void INCL(); void DECL(); void LDL8(); void CPL();
		void JRNC8(); void LDSP16(); void LDDHLA(); void INCSP(); void INCHLR(); void DECHLR(); void LDHL8(); void SCF(); void JRC8(); void ADDHLSP(); void LDDAHL(); void DECSP(); void INCA(); void DECA(); void LDA8(); void CCF();
		void LDBB(); void LDBC(); void LDBD(); void LDBE(); void LDBH(); void LDBL(); void LDBHL(); void LDBA(); void LDCB(); void LDCC(); void LDCD(); void LDCE(); void LDCH(); void LDCL(); void LDCHL(); void LDCA();
		void LDDB(); void LDDC(); void LDDD(); void LDDE(); void LDDH(); void LDDL(); void LDDHL(); void LDDA(); void LDEB(); void LDEC(); void LDED(); void LDEE(); void LDEH(); void LDEL(); void LDEHL(); void LDEA();
		void LDHB(); void LDHC(); void LDHD(); void LDHE(); void LDHH(); void LDHL(); void LDHHL(); void LDHA(); void LDLB(); void LDLC(); void LDLD(); void LDLE(); void LDLH(); void LDLL(); void LDLHL(); void LDLA();
		void LDHLB(); void LDHLC(); void LDHLD(); void LDHLE(); void LDHLH(); void LDHLL(); void HALT(); void LDHLA(); void LDAB(); void LDAC(); void LDAD(); void LDAE(); void LDAH(); void LDAL(); void LDAHL(); void LDAA();
		void ADDAB(); void ADDAC(); void ADDAD(); void ADDAE(); void ADDAH(); void ADDAL(); void ADDAHL(); void ADDAA(); void ADCAB(); void ADCAC(); void ADCAD(); void ADCAE(); void ADCAH(); void ADCAL(); void ADCAHL(); void ADCAA();
		void SUBAB(); void SUBAC(); void SUBAD(); void SUBAE(); void SUBAH(); void SUBAL(); void SUBAHL(); void SUBAA(); void SBCAB(); void SBCAC(); void SBCAD(); void SBCAE(); void SBCAH(); void SBCAL(); void SBCAHL(); void SBCAA();
		void ANDB(); void ANDC(); void ANDD(); void ANDE(); void ANDH(); void ANDL(); void ANDHL(); void ANDA(); void XORB(); void XORC(); void XORD(); void XORE(); void XORH(); void XORL(); void XORHL(); void XORA();
		void ORB(); void ORC(); void ORD(); void ORE(); void ORH(); void ORL(); void ORHL(); void ORA(); void CPAB(); void CPAC(); void CPAD(); void CPAE(); void CPAH(); void CPAL(); void CPAHL(); void CPAA();
		void RETNZ(); void POPBC(); void JPNZ16(); void JP16(); void CALLNZ16(); void PUSHBC(); void ADDA8(); void RST0(); void RETZ(); void RET(); void JPZ16(); void EXT(); void CALLZ16(); void CALL16(); void ADCA8(); void RST8();
		void RETNC(); void POPDE(); void JPNC16(); void CALLNC16(); void PUSHDE(); void SUBA8(); void RST10(); void RETC(); void RETI(); void JPC16();  void CALLC16();  void SBCA8(); void RST18();
		void LDH8A(); void POPHL(); void LDHCA(); void PUSHHL(); void AND8(); void RST20(); void ADDSPD(); void JPHL(); void LD16A(); void XOR8(); void RST28();
		void LDHA8(); void POPAF();  void DI();  void PUSHAF(); void OR8(); void RST30(); void LDHLSPD(); void LDSPHL(); void LDA16(); void EI(); void CP8(); void RST38();

		void RST40(); void RST48(); void RST50(); void RST58(); void RST60();

		// Two byte instructions
		void RLCB(); void RLCC(); void RLCD(); void RLCE(); void RLCH(); void RLCL(); void RLCHL(); void RLCAr(); void RRCB(); void RRCC(); void RRCD(); void RRCE(); void RRCH(); void RRCL(); void RRCHL(); void RRCAr();
		void RLB(); void RLC(); void RLD(); void RLE(); void RLH(); void RLL(); void RLHL(); void RLAr(); void RRB(); void RRC(); void RRD(); void RRE(); void RRH(); void RRL(); void RRHL(); void RRAr();
		void SLAB(); void SLAC(); void SLAD(); void SLAE(); void SLAH(); void SLAL(); void SLAA(); void SRAB(); void SRAC(); void SRAD(); void SRAE(); void SRAH(); void SRAL(); void SRAA();
		void SWAPB(); void SWAPC(); void SWAPD(); void SWAPE(); void SWAPH(); void SWAPL(); void SWAPA(); void SRLB(); void SRLC(); void SRLD(); void SRLE(); void SRLH(); void SRLL(); void SRLA();
		void BIT0B(); void BIT0C(); void BIT0D(); void BIT0E(); void BIT0H(); void BIT0L(); void BIT0M(); void BIT0A(); void BIT1B(); void BIT1C(); void BIT1D(); void BIT1E(); void BIT1H(); void BIT1L(); void BIT1M(); void BIT1A();
		void BIT2B(); void BIT2C(); void BIT2D(); void BIT2E(); void BIT2H(); void BIT2L(); void BIT2M(); void BIT2A(); void BIT3B(); void BIT3C(); void BIT3D(); void BIT3E(); void BIT3H(); void BIT3L(); void BIT3M(); void BIT3A();
		void BIT4B(); void BIT4C(); void BIT4D(); void BIT4E(); void BIT4H(); void BIT4L(); void BIT4M(); void BIT4A(); void BIT5B(); void BIT5C(); void BIT5D(); void BIT5E(); void BIT5H(); void BIT5L(); void BIT5M(); void BIT5A();
		void BIT6B(); void BIT6C(); void BIT6D(); void BIT6E(); void BIT6H(); void BIT6L(); void BIT6M(); void BIT6A(); void BIT7B(); void BIT7C(); void BIT7D(); void BIT7E(); void BIT7H(); void BIT7L(); void BIT7M(); void BIT7A();
		void RES0A();


		// Undefined instructions
		void XXX();

		// Helper functions that also deal with the flags
		inline void reg_dec(RegisterType& reg);
		inline void reg_inc(RegisterType& reg);
		inline void reg_sub(RegisterType& reg);
		inline void reg_and(RegisterType& reg);
		inline void bit_ch(RegisterType reg, unsigned shift);

		// TODO: remove FZ
		void FZ(int i, bool as = false);
		void RSV();
		void RRS();
		/// <summary>Flag 7 0x80</summary>
		void SetFlagZero(unsigned long value) {
			F = (F & ~(1UL << 7)) | (value << 7);
		}
		/// <summary>Flag 6 0x40</summary>
		void SetFlagSubtract(unsigned long value) {
			F = (F & ~(1UL << 6)) | (value << 6);
		}
		/// <summary>Flag 5 0x20</summary>
		void SetFlagHalfCarry(unsigned long value) {
			F = (F & ~(1UL << 5)) | (value << 5);
		}
		/// <summary>Flag 4 0x10</summary>
		void SetFlagCarry(unsigned long value) {
			F = (F & ~(1UL << 4)) | (value << 4);
		}

	public:
		RegisterType A, B, C, D, E, H, L, F, PC, SP;
		RegisterType rsvA, rsvB, rsvC, rsvD, rsvE, rsvH, rsvL, rsvF;
		bool IME;
		unsigned totalClock = 0;
		int mClock;
		int tClock;
		CPU(Bus* bus);
		struct Instruction {
			std::string name;
			void(CPU::* op)() = nullptr;
			int skip = 0;
		};
		Bus* bus_;
		std::vector<Instruction> instructions;
		std::vector<Instruction> cbMap;

		void Reset();
		void Update();
	};
}
#endif