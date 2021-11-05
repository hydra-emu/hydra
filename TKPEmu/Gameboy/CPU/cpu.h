#pragma once
#ifndef TKP_GB_CPU_H
#define TKP_GB_CPU_H
#include <cstdint>
#include <string>
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
		using RegisterType = uint8_t;
		using BigRegisterType = uint16_t;
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

		Bus* bus_;
		bool IME = false;
		int mTemp = 0;
		int tTemp = 0;
		int div_index_ = 0;
		int div_reset_index_ = -1;
		int old_tac_ = 0;
		int tac_index_ = 0x1000;
		bool halt = false;
		bool stop = false;

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
		void LDHA8(); void POPAF();  void LDAMC(); void DI();  void PUSHAF(); void OR8(); void RST30(); void LDHLSPD(); void LDSPHL(); void LDA16(); void EI(); void CP8(); void RST38();

		// Two byte instructions
		void RLCB(); void RLCC(); void RLCD(); void RLCE(); void RLCH(); void RLCL(); void RLCHL(); void RLCAr(); void RRCB(); void RRCC(); void RRCD(); void RRCE(); void RRCH(); void RRCL(); void RRCHL(); void RRCAr();
		void RLB(); void RLC(); void RLD(); void RLE(); void RLH(); void RLL(); void RLHL(); void RLAr(); void RRB(); void RRC(); void RRD(); void RRE(); void RRH(); void RRL(); void RRHL(); void RRAr();
		void SLAB(); void SLAC(); void SLAD(); void SLAE(); void SLAH(); void SLAL(); void SLAHL(); void SLAA(); void SRAB(); void SRAC(); void SRAD(); void SRAE(); void SRAH(); void SRAL(); void SRAHL(); void SRAA();
		void SWAPB(); void SWAPC(); void SWAPD(); void SWAPE(); void SWAPH(); void SWAPL(); void SWAPHL();  void SWAPA(); void SRLB(); void SRLC(); void SRLD(); void SRLE(); void SRLH(); void SRLL(); void SRLHL(); void SRLA();
		void BIT0B(); void BIT0C(); void BIT0D(); void BIT0E(); void BIT0H(); void BIT0L(); void BIT0M(); void BIT0A(); void BIT1B(); void BIT1C(); void BIT1D(); void BIT1E(); void BIT1H(); void BIT1L(); void BIT1M(); void BIT1A();
		void BIT2B(); void BIT2C(); void BIT2D(); void BIT2E(); void BIT2H(); void BIT2L(); void BIT2M(); void BIT2A(); void BIT3B(); void BIT3C(); void BIT3D(); void BIT3E(); void BIT3H(); void BIT3L(); void BIT3M(); void BIT3A();
		void BIT4B(); void BIT4C(); void BIT4D(); void BIT4E(); void BIT4H(); void BIT4L(); void BIT4M(); void BIT4A(); void BIT5B(); void BIT5C(); void BIT5D(); void BIT5E(); void BIT5H(); void BIT5L(); void BIT5M(); void BIT5A();
		void BIT6B(); void BIT6C(); void BIT6D(); void BIT6E(); void BIT6H(); void BIT6L(); void BIT6M(); void BIT6A(); void BIT7B(); void BIT7C(); void BIT7D(); void BIT7E(); void BIT7H(); void BIT7L(); void BIT7M(); void BIT7A();
		void RES0B(); void RES0C(); void RES0D(); void RES0E(); void RES0H(); void RES0L(); void RES0HL(); void RES0A(); void RES1B(); void RES1C(); void RES1D(); void RES1E(); void RES1H(); void RES1L(); void RES1HL(); void RES1A();
		void RES2B(); void RES2C(); void RES2D(); void RES2E(); void RES2H(); void RES2L(); void RES2HL(); void RES2A(); void RES3B(); void RES3C(); void RES3D(); void RES3E(); void RES3H(); void RES3L(); void RES3HL(); void RES3A();
		void RES4B(); void RES4C(); void RES4D(); void RES4E(); void RES4H(); void RES4L(); void RES4HL(); void RES4A(); void RES5B(); void RES5C(); void RES5D(); void RES5E(); void RES5H(); void RES5L(); void RES5HL(); void RES5A();
		void RES6B(); void RES6C(); void RES6D(); void RES6E(); void RES6H(); void RES6L(); void RES6HL(); void RES6A(); void RES7B(); void RES7C(); void RES7D(); void RES7E(); void RES7H(); void RES7L(); void RES7HL(); void RES7A();
		void SET0B(); void SET0C(); void SET0D(); void SET0E(); void SET0H(); void SET0L(); void SET0HL(); void SET0A(); void SET1B(); void SET1C(); void SET1D(); void SET1E(); void SET1H(); void SET1L(); void SET1HL(); void SET1A();
		void SET2B(); void SET2C(); void SET2D(); void SET2E(); void SET2H(); void SET2L(); void SET2HL(); void SET2A(); void SET3B(); void SET3C(); void SET3D(); void SET3E(); void SET3H(); void SET3L(); void SET3HL(); void SET3A();
		void SET4B(); void SET4C(); void SET4D(); void SET4E(); void SET4H(); void SET4L(); void SET4HL(); void SET4A(); void SET5B(); void SET5C(); void SET5D(); void SET5E(); void SET5H(); void SET5L(); void SET5HL(); void SET5A();
		void SET6B(); void SET6C(); void SET6D(); void SET6E(); void SET6H(); void SET6L(); void SET6HL(); void SET6A(); void SET7B(); void SET7C(); void SET7D(); void SET7E(); void SET7H(); void SET7L(); void SET7HL(); void SET7A();

		// Undefined instructions
		void XXX();

		// Helper functions that also deal with the flags
		inline void reg_dec(RegisterType& reg);
		inline void reg_inc(RegisterType& reg);
		inline void reg_sub(RegisterType& reg);
		inline void reg_sbc(RegisterType& reg);
		inline void reg_and(RegisterType& reg);
		inline void reg_add(RegisterType& reg);
		inline void reg_adc(RegisterType& reg);
		inline void reg_cmp(RegisterType& reg);
		inline void reg_or (RegisterType& reg);
		inline void reg_xor(RegisterType& reg);
		inline void hl_add(BigRegisterType& big_reg);
		inline void bit_ch(RegisterType reg, unsigned shift);
		inline void bit_res(RegisterType& reg, unsigned shift);
		inline void bit_set(RegisterType& reg, unsigned shift);
		inline void bit_swap(RegisterType& reg);
		inline void bit_rrc(RegisterType& reg);
		inline void bit_rl(RegisterType& reg);
		inline void bit_rr(RegisterType& reg);
		inline void bit_sl(RegisterType& reg);
		inline void bit_sr(RegisterType& reg);
		inline void bit_srl(RegisterType& reg);
		inline void rst(RegisterType addr);
		void handle_interrupts();
		void execute_interrupt(int bit);
		void update_timers(int cycles);
		int get_clk_freq();

	public:
		CPU(Bus* bus);
		struct Instruction {
			std::string name;
			void(CPU::* op)() = nullptr;
			int skip = 0;
		};
		std::array<Instruction, 0x100> Instructions = { {
			{ "NOP" , &CPU::NOP }, { "LDBC16" , &CPU::LDBC16 , 2}, { "LDBCA" , &CPU::LDBCA }, { "INCBC" , &CPU::INCBC }, { "INCB" , &CPU::INCB }, { "DECB" , &CPU::DECB }, { "LDB8" , &CPU::LDB8 , 1}, { "RLCA" , &CPU::RLCA }, { "LD16SP" , &CPU::LD16SP }, { "ADDHLBC" , &CPU::ADDHLBC }, { "LDABC" , &CPU::LDABC }, { "DECBC" , &CPU::DECBC }, { "INCC" , &CPU::INCC }, { "DECC" , &CPU::DECC }, { "LDC8" , &CPU::LDC8 , 1}, { "RRCA" , &CPU::RRCA },
			{ "STOP" , &CPU::STOP }, { "LDDE16" , &CPU::LDDE16 , 2}, { "LDDEA" , &CPU::LDDEA }, { "INCDE" , &CPU::INCDE }, { "INCD" , &CPU::INCD }, { "DECD" , &CPU::DECD }, { "LDD8" , &CPU::LDD8 , 1}, { "RLA" , &CPU::RLA }, { "JR8" , &CPU::JR8 , 1}, { "ADDHLDE" , &CPU::ADDHLDE }, { "LDADE" , &CPU::LDADE }, { "DECDE" , &CPU::DECDE }, { "INCE" , &CPU::INCE }, { "DECE" , &CPU::DECE }, { "LDE8" , &CPU::LDE8 , 1}, { "RRA" , &CPU::RRA },
			{ "JRNZ8" , &CPU::JRNZ8 , 1}, { "LDHL16" , &CPU::LDHL16 , 2}, { "LDIHLA" , &CPU::LDIHLA }, { "INCHL" , &CPU::INCHL }, { "INCH" , &CPU::INCH }, { "DECH" , &CPU::DECH }, { "LDH8" , &CPU::LDH8 , 1}, { "DAA" , &CPU::DAA }, { "JRZ8" , &CPU::JRZ8 , 1}, { "ADDHLHL" , &CPU::ADDHLHL }, { "LDIAHL" , &CPU::LDIAHL }, { "DECHL" , &CPU::DECHL }, { "INCL" , &CPU::INCL }, { "DECL" , &CPU::DECL }, { "LDL8" , &CPU::LDL8 , 1}, { "CPL" , &CPU::CPL },
			{ "JRNC8" , &CPU::JRNC8 , 1}, { "LDSP16" , &CPU::LDSP16 , 2}, { "LDDHLA" , &CPU::LDDHLA }, { "INCSP" , &CPU::INCSP }, { "INCHLR" , &CPU::INCHLR }, { "DECHLR" , &CPU::DECHLR }, { "LDHL8" , &CPU::LDHL8 , 1}, { "SCF" , &CPU::SCF }, { "JRC8" , &CPU::JRC8 , 1}, { "ADDHLSP" , &CPU::ADDHLSP }, { "LDDAHL" , &CPU::LDDAHL }, { "DECSP" , &CPU::DECSP }, { "INCA" , &CPU::INCA }, { "DECA" , &CPU::DECA }, { "LDA8" , &CPU::LDA8 , 1}, { "CCF" , &CPU::CCF },
			{ "LDBB" , &CPU::LDBB }, { "LDBC" , &CPU::LDBC }, { "LDBD" , &CPU::LDBD }, { "LDBE" , &CPU::LDBE }, { "LDBH" , &CPU::LDBH }, { "LDBL" , &CPU::LDBL }, { "LDBHL" , &CPU::LDBHL }, { "LDBA" , &CPU::LDBA }, { "LDCB" , &CPU::LDCB }, { "LDCC" , &CPU::LDCC }, { "LDCD" , &CPU::LDCD }, { "LDCE" , &CPU::LDCE }, { "LDCH" , &CPU::LDCH }, { "LDCL" , &CPU::LDCL }, { "LDCHL" , &CPU::LDCHL }, { "LDCA" , &CPU::LDCA },
			{ "LDDB" , &CPU::LDDB }, { "LDDC" , &CPU::LDDC }, { "LDDD" , &CPU::LDDD }, { "LDDE" , &CPU::LDDE }, { "LDDH" , &CPU::LDDH }, { "LDDL" , &CPU::LDDL }, { "LDDHL" , &CPU::LDDHL }, { "LDDA" , &CPU::LDDA }, { "LDEB" , &CPU::LDEB }, { "LDEC" , &CPU::LDEC }, { "LDED" , &CPU::LDED }, { "LDEE" , &CPU::LDEE }, { "LDEH" , &CPU::LDEH }, { "LDEL" , &CPU::LDEL }, { "LDEHL" , &CPU::LDEHL }, { "LDEA" , &CPU::LDEA },
			{ "LDHB" , &CPU::LDHB }, { "LDHC" , &CPU::LDHC }, { "LDHD" , &CPU::LDHD }, { "LDHE" , &CPU::LDHE }, { "LDHH" , &CPU::LDHH }, { "LDHL" , &CPU::LDHL }, { "LDHHL" , &CPU::LDHHL }, { "LDHA" , &CPU::LDHA }, { "LDLB" , &CPU::LDLB }, { "LDLC" , &CPU::LDLC }, { "LDLD" , &CPU::LDLD }, { "LDLE" , &CPU::LDLE }, { "LDLH" , &CPU::LDLH }, { "LDLL" , &CPU::LDLL }, { "LDLHL" , &CPU::LDLHL }, { "LDLA" , &CPU::LDLA },
			{ "LDHLB" , &CPU::LDHLB }, { "LDHLC" , &CPU::LDHLC }, { "LDHLD" , &CPU::LDHLD }, { "LDHLE" , &CPU::LDHLE }, { "LDHLH" , &CPU::LDHLH }, { "LDHLL" , &CPU::LDHLL }, { "HALT" , &CPU::HALT }, { "LDHLA" , &CPU::LDHLA }, { "LDAB" , &CPU::LDAB }, { "LDAC" , &CPU::LDAC }, { "LDAD" , &CPU::LDAD }, { "LDAE" , &CPU::LDAE }, { "LDAH" , &CPU::LDAH }, { "LDAL" , &CPU::LDAL }, { "LDAHL" , &CPU::LDAHL }, { "LDAA" , &CPU::LDAA },
			{ "ADDAB" , &CPU::ADDAB }, { "ADDAC" , &CPU::ADDAC }, { "ADDAD" , &CPU::ADDAD }, { "ADDAE" , &CPU::ADDAE }, { "ADDAH" , &CPU::ADDAH }, { "ADDAL" , &CPU::ADDAL }, { "ADDAHL" , &CPU::ADDAHL }, { "ADDAA" , &CPU::ADDAA }, { "ADCAB" , &CPU::ADCAB }, { "ADCAC" , &CPU::ADCAC }, { "ADCAD" , &CPU::ADCAD }, { "ADCAE" , &CPU::ADCAE }, { "ADCAH" , &CPU::ADCAH }, { "ADCAL" , &CPU::ADCAL }, { "ADCAHL" , &CPU::ADCAHL }, { "ADCAA" , &CPU::ADCAA },
			{ "SUBAB" , &CPU::SUBAB }, { "SUBAC" , &CPU::SUBAC }, { "SUBAD" , &CPU::SUBAD }, { "SUBAE" , &CPU::SUBAE }, { "SUBAH" , &CPU::SUBAH }, { "SUBAL" , &CPU::SUBAL }, { "SUBAHL" , &CPU::SUBAHL }, { "SUBAA" , &CPU::SUBAA }, { "SBCAB" , &CPU::SBCAB }, { "SBCAC" , &CPU::SBCAC }, { "SBCAD" , &CPU::SBCAD }, { "SBCAE" , &CPU::SBCAE }, { "SBCAH" , &CPU::SBCAH }, { "SBCAL" , &CPU::SBCAL }, { "SBCAHL" , &CPU::SBCAHL }, { "SBCAA" , &CPU::SBCAA },
			{ "ANDB" , &CPU::ANDB }, { "ANDC" , &CPU::ANDC }, { "ANDD" , &CPU::ANDD }, { "ANDE" , &CPU::ANDE }, { "ANDH" , &CPU::ANDH }, { "ANDL" , &CPU::ANDL }, { "ANDHL" , &CPU::ANDHL }, { "ANDA" , &CPU::ANDA }, { "XORB" , &CPU::XORB }, { "XORC" , &CPU::XORC }, { "XORD" , &CPU::XORD }, { "XORE" , &CPU::XORE }, { "XORH" , &CPU::XORH }, { "XORL" , &CPU::XORL }, { "XORHL" , &CPU::XORHL }, { "XORA" , &CPU::XORA },
			{ "ORB" , &CPU::ORB }, { "ORC" , &CPU::ORC }, { "ORD" , &CPU::ORD }, { "ORE" , &CPU::ORE }, { "ORH" , &CPU::ORH }, { "ORL" , &CPU::ORL }, { "ORHL" , &CPU::ORHL }, { "ORA" , &CPU::ORA }, { "CPAB" , &CPU::CPAB }, { "CPAC" , &CPU::CPAC }, { "CPAD" , &CPU::CPAD }, { "CPAE" , &CPU::CPAE }, { "CPAH" , &CPU::CPAH }, { "CPAL" , &CPU::CPAL }, { "CPAHL" , &CPU::CPAHL }, { "CPAA" , &CPU::CPAA },
			{ "RETNZ" , &CPU::RETNZ }, { "POPBC" , &CPU::POPBC }, { "JPNZ16" , &CPU::JPNZ16 , 2}, { "JP16" , &CPU::JP16 , 2}, { "CALLNZ16" , &CPU::CALLNZ16 , 2}, { "PUSHBC" , &CPU::PUSHBC }, { "ADDA8" , &CPU::ADDA8 , 1}, { "RST0" , &CPU::RST0 }, { "RETZ" , &CPU::RETZ }, { "RET" , &CPU::RET }, { "JPZ16" , &CPU::JPZ16 , 2}, { "EXT" , &CPU::EXT , 1 }, { "CALLZ16" , &CPU::CALLZ16 , 2}, { "CALL16" , &CPU::CALL16 , 2}, { "ADCA8" , &CPU::ADCA8 , 2}, { "RST8" , &CPU::RST8 },
			{ "RETNC" , &CPU::RETNC }, { "POPDE" , &CPU::POPDE }, { "JPNC16" , &CPU::JPNC16 , 2}, { "???" , &CPU::XXX }, { "CALLNC16" , &CPU::CALLNC16 , 2}, { "PUSHDE" , &CPU::PUSHDE }, { "SUBA8" , &CPU::SUBA8 , 1}, { "RST10" , &CPU::RST10 }, { "RETC" , &CPU::RETC }, { "RETI" , &CPU::RETI }, { "JPC16" , &CPU::JPC16 , 2}, { "???" , &CPU::XXX }, { "CALLC16" , &CPU::CALLC16 , 2}, { "???" , &CPU::XXX }, { "SBCA8" , &CPU::SBCA8 , 1}, { "RST18" , &CPU::RST18 },
			{ "LDH8A" , &CPU::LDH8A }, { "POPHL" , &CPU::POPHL }, { "LDHCA" , &CPU::LDHCA }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "PUSHHL" , &CPU::PUSHHL }, { "AND8" , &CPU::AND8 , 1}, { "RST20" , &CPU::RST20 }, { "ADDSPD" , &CPU::ADDSPD , 1}, { "JPHL" , &CPU::JPHL }, { "LD16A" , &CPU::LD16A }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "XOR8" , &CPU::XOR8 , 1}, { "RST28" , &CPU::RST28 },
			{ "LDHA8" , &CPU::LDHA8 , 1}, { "POPAF" , &CPU::POPAF }, { "LDAMC" , &CPU::LDAMC }, { "DI" , &CPU::DI }, { "???" , &CPU::XXX }, { "PUSHAF" , &CPU::PUSHAF }, { "OR8" , &CPU::OR8, 1 }, { "RST30" , &CPU::RST30 }, { "LDHLSPD" , &CPU::LDHLSPD , 1 }, { "LDSPHL", &CPU::LDSPHL }, { "LDA16" , &CPU::LDA16 }, { "EI" , &CPU::EI }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "CP8" , &CPU::CP8 , 1}, { "RST38" , &CPU::RST38 }
		} };
		std::array<Instruction, 0x100> CBInstructions = { {
			{ "RLCB" , &CPU::RLCB }, { "RLCC" , &CPU::RLCC }, { "RLCD" , &CPU::RLCD }, { "RLCE" , &CPU::RLCE }, { "RLCH" , &CPU::RLCH }, { "RLCL" , &CPU::RLCL }, { "RLCHL" , &CPU::RLCHL }, { "RLCAr" , &CPU::RLCAr },  { "RRCB" , &CPU::RRCB }, { "RRCC" , &CPU::RRCC }, { "RRCD" , &CPU::RRCD }, { "RRCE" , &CPU::RRCE }, { "RRCH" , &CPU::RRCH }, { "RRCL" , &CPU::RRCL }, { "RRCHL" , &CPU::RRCHL }, { "RRCAr" , &CPU::RRCAr },
			{ "RLB" , &CPU::RLB }, { "RLC" , &CPU::RLC }, { "RLD" , &CPU::RLD }, { "RLE" , &CPU::RLE }, { "RLH" , &CPU::RLH }, { "RLL" , &CPU::RLL }, { "RLHL" , &CPU::RLHL }, { "RLAr" , &CPU::RLAr }, { "RRB" , &CPU::RRB }, { "RRC" , &CPU::RRC },  { "RRD" , &CPU::RRD },  { "RRE" , &CPU::RRE },  { "RRH" , &CPU::RRH },  { "RRL" , &CPU::RRL },  { "RRHL" , &CPU::RRHL },  { "RRAr" , &CPU::RRAr },
			{ "SLAB" , &CPU::SLAB }, { "SLAC" , &CPU::SLAC }, { "SLAD" , &CPU::SLAD }, { "SLAE" , &CPU::SLAE }, { "SLAH" , &CPU::SLAH }, { "SLAL" , &CPU::SLAL }, { "SLAHL" , &CPU::SLAHL }, { "SLAA" , &CPU::SLAA }, { "SRAB" , &CPU::SRAB }, { "SRAC" , &CPU::SRAC }, { "SRAD" , &CPU::SRAD }, { "SRAE" , &CPU::SRAE }, { "SRAH" , &CPU::SRAH }, { "SRAL" , &CPU::SRAL }, { "SRAHL" , &CPU::SRAHL }, { "SRAA" , &CPU::SRAA },
			{ "SWAPB" , &CPU::SWAPB }, { "SWAPC" , &CPU::SWAPC }, { "SWAPD" , &CPU::SWAPD }, { "SWAPE" , &CPU::SWAPE }, { "SWAPH" , &CPU::SWAPH }, { "SWAPL" , &CPU::SWAPL }, { "SWAPHL" , &CPU::SWAPHL }, { "SWAPA" , &CPU::SWAPA }, { "SRLB" , &CPU::SRLB }, { "SRLC" , &CPU::SRLC }, { "SRLD" , &CPU::SRLD }, { "SRLE" , &CPU::SRLE }, { "SRLH" , &CPU::SRLH }, { "SRLL" , &CPU::SRLL }, { "SRLHL" , &CPU::SRLHL }, { "SRLA" , &CPU::SRLA },
			{ "BIT0B" , &CPU::BIT0B }, { "BIT0C" , &CPU::BIT0C }, { "BIT0D" , &CPU::BIT0D }, { "BIT0E" , &CPU::BIT0E }, { "BIT0H" , &CPU::BIT0H }, { "BIT0L" , &CPU::BIT0L }, { "BIT0M" , &CPU::BIT0M }, { "BIT0A" , &CPU::BIT0A }, { "BIT1B" , &CPU::BIT1B }, { "BIT1C" , &CPU::BIT1C }, { "BIT1D" , &CPU::BIT1D }, { "BIT1E" , &CPU::BIT1E }, { "BIT1H" , &CPU::BIT1H }, { "BIT1L" , &CPU::BIT1L }, { "BIT1M" , &CPU::BIT1M }, { "BIT1A" , &CPU::BIT1A },
			{ "BIT2B" , &CPU::BIT2B }, { "BIT2C" , &CPU::BIT2C }, { "BIT2D" , &CPU::BIT2D }, { "BIT2E" , &CPU::BIT2E }, { "BIT2H" , &CPU::BIT2H }, { "BIT2L" , &CPU::BIT2L }, { "BIT2M" , &CPU::BIT2M }, { "BIT2A" , &CPU::BIT2A }, { "BIT3B" , &CPU::BIT3B }, { "BIT3C" , &CPU::BIT3C }, { "BIT3D" , &CPU::BIT3D }, { "BIT3E" , &CPU::BIT3E }, { "BIT3H" , &CPU::BIT3H }, { "BIT3L" , &CPU::BIT3L }, { "BIT3M" , &CPU::BIT3M }, { "BIT3A" , &CPU::BIT3A },
			{ "BIT4B" , &CPU::BIT4B }, { "BIT4C" , &CPU::BIT4C }, { "BIT4D" , &CPU::BIT4D }, { "BIT4E" , &CPU::BIT4E }, { "BIT4H" , &CPU::BIT4H }, { "BIT4L" , &CPU::BIT4L }, { "BIT4M" , &CPU::BIT4M }, { "BIT4A" , &CPU::BIT4A }, { "BIT5B" , &CPU::BIT5B }, { "BIT5C" , &CPU::BIT5C }, { "BIT5D" , &CPU::BIT5D }, { "BIT5E" , &CPU::BIT5E }, { "BIT5H" , &CPU::BIT5H }, { "BIT5L" , &CPU::BIT5L }, { "BIT5M" , &CPU::BIT5M }, { "BIT5A" , &CPU::BIT5A },
			{ "BIT6B" , &CPU::BIT6B }, { "BIT6C" , &CPU::BIT6C }, { "BIT6D" , &CPU::BIT6D }, { "BIT6E" , &CPU::BIT6E }, { "BIT6H" , &CPU::BIT6H }, { "BIT6L" , &CPU::BIT6L }, { "BIT6M" , &CPU::BIT6M }, { "BIT6A" , &CPU::BIT6A }, { "BIT7B" , &CPU::BIT7B }, { "BIT7C" , &CPU::BIT7C }, { "BIT7D" , &CPU::BIT7D }, { "BIT7E" , &CPU::BIT7E }, { "BIT7H" , &CPU::BIT7H }, { "BIT7L" , &CPU::BIT7L }, { "BIT7M" , &CPU::BIT7M }, { "BIT7A" , &CPU::BIT7A },
			{ "RES0B" , &CPU::RES0B }, { "RES0C" , &CPU::RES0C }, { "RES0D" , &CPU::RES0D }, { "RES0E" , &CPU::RES0E }, { "RES0H" , &CPU::RES0H }, { "RES0L" , &CPU::RES0L }, { "RES0HL" , &CPU::RES0HL }, { "RES0A" , &CPU::RES0A }, { "RES1B" , &CPU::RES1B }, { "RES1C" , &CPU::RES1C }, { "RES1D" , &CPU::RES1D }, { "RES1E" , &CPU::RES1E }, { "RES1H" , &CPU::RES1H }, { "RES1L" , &CPU::RES1L }, { "RES1HL" , &CPU::RES1HL }, { "RES1A" , &CPU::RES1A },
			{ "RES2B" , &CPU::RES2B }, { "RES2C" , &CPU::RES2C }, { "RES2D" , &CPU::RES2D }, { "RES2E" , &CPU::RES2E }, { "RES2H" , &CPU::RES2H }, { "RES2L" , &CPU::RES2L }, { "RES2HL" , &CPU::RES2HL }, { "RES2A" , &CPU::RES2A }, { "RES3B" , &CPU::RES3B }, { "RES3C" , &CPU::RES3C }, { "RES3D" , &CPU::RES3D }, { "RES3E" , &CPU::RES3E }, { "RES3H" , &CPU::RES3H }, { "RES3L" , &CPU::RES3L }, { "RES3HL" , &CPU::RES3HL }, { "RES3A" , &CPU::RES3A },
			{ "RES4B" , &CPU::RES4B }, { "RES4C" , &CPU::RES4C }, { "RES4D" , &CPU::RES4D }, { "RES4E" , &CPU::RES4E }, { "RES4H" , &CPU::RES4H }, { "RES4L" , &CPU::RES4L }, { "RES4HL" , &CPU::RES4HL }, { "RES4A" , &CPU::RES4A }, { "RES5B" , &CPU::RES5B }, { "RES5C" , &CPU::RES5C }, { "RES5D" , &CPU::RES5D }, { "RES5E" , &CPU::RES5E }, { "RES5H" , &CPU::RES5H }, { "RES5L" , &CPU::RES5L }, { "RES5HL" , &CPU::RES5HL }, { "RES5A" , &CPU::RES5A },
			{ "RES6B" , &CPU::RES6B }, { "RES6C" , &CPU::RES6C }, { "RES6D" , &CPU::RES6D }, { "RES6E" , &CPU::RES6E }, { "RES6H" , &CPU::RES6H }, { "RES6L" , &CPU::RES6L }, { "RES6HL" , &CPU::RES6HL }, { "RES6A" , &CPU::RES6A }, { "RES7B" , &CPU::RES7B }, { "RES7C" , &CPU::RES7C }, { "RES7D" , &CPU::RES7D }, { "RES7E" , &CPU::RES7E }, { "RES7H" , &CPU::RES7H }, { "RES7L" , &CPU::RES7L }, { "RES7HL" , &CPU::RES7HL }, { "RES7A" , &CPU::RES7A },
			{ "SET0B" , &CPU::SET0B }, { "SET0C" , &CPU::SET0C }, { "SET0D" , &CPU::SET0D }, { "SET0E" , &CPU::SET0E }, { "SET0H" , &CPU::SET0H }, { "SET0L" , &CPU::SET0L }, { "SET0HL" , &CPU::SET0HL }, { "SET0A" , &CPU::SET0A }, { "SET1B" , &CPU::SET1B }, { "SET1C" , &CPU::SET1C }, { "SET1D" , &CPU::SET1D }, { "SET1E" , &CPU::SET1E }, { "SET1H" , &CPU::SET1H }, { "SET1L" , &CPU::SET1L }, { "SET1HL" , &CPU::SET1HL }, { "SET1A" , &CPU::SET1A },
			{ "SET2B" , &CPU::SET2B }, { "SET2C" , &CPU::SET2C }, { "SET2D" , &CPU::SET2D }, { "SET2E" , &CPU::SET2E }, { "SET2H" , &CPU::SET2H }, { "SET2L" , &CPU::SET2L }, { "SET2HL" , &CPU::SET2HL }, { "SET2A" , &CPU::SET2A }, { "SET3B" , &CPU::SET3B }, { "SET3C" , &CPU::SET3C }, { "SET3D" , &CPU::SET3D }, { "SET3E" , &CPU::SET3E }, { "SET3H" , &CPU::SET3H }, { "SET3L" , &CPU::SET3L }, { "SET3HL" , &CPU::SET3HL }, { "SET3A" , &CPU::SET3A },
			{ "SET4B" , &CPU::SET4B }, { "SET4C" , &CPU::SET4C }, { "SET4D" , &CPU::SET4D }, { "SET4E" , &CPU::SET4E }, { "SET4H" , &CPU::SET4H }, { "SET4L" , &CPU::SET4L }, { "SET4HL" , &CPU::SET4HL }, { "SET4A" , &CPU::SET4A }, { "SET5B" , &CPU::SET5B }, { "SET5C" , &CPU::SET5C }, { "SET5D" , &CPU::SET5D }, { "SET5E" , &CPU::SET5E }, { "SET5H" , &CPU::SET5H }, { "SET5L" , &CPU::SET5L }, { "SET5HL" , &CPU::SET5HL }, { "SET5A" , &CPU::SET5A },
			{ "SET6B" , &CPU::SET6B }, { "SET6C" , &CPU::SET6C }, { "SET6D" , &CPU::SET6D }, { "SET6E" , &CPU::SET6E }, { "SET6H" , &CPU::SET6H }, { "SET6L" , &CPU::SET6L }, { "SET6HL" , &CPU::SET6HL }, { "SET6A" , &CPU::SET6A }, { "SET7B" , &CPU::SET7B }, { "SET7C" , &CPU::SET7C }, { "SET7D" , &CPU::SET7D }, { "SET7E" , &CPU::SET7E }, { "SET7H" , &CPU::SET7H }, { "SET7L" , &CPU::SET7L }, { "SET7HL" , &CPU::SET7HL }, { "SET7A" , &CPU::SET7A }
		} };

		// CPU registers
		RegisterType A, B, C, D, E, H, L, F;
		BigRegisterType PC, SP;

		// Memory mapped registers, they are a reference to a position in memory
		RegisterType &IF, &IE, &DIVIDER, &TIMA, &TMA, &TAC;

		const int ClockSpeed = 4194304;
		const int MaxCycles = ClockSpeed / 60;
		int TimerCounter = ClockSpeed / tac_index_;
		int tClock = 0;

		void Reset();
		int Update();
	};
}
#endif