#include "cpu.h"
#include <stdexcept>
#include <iostream>
namespace TKPEmu::Gameboy::Devices {
	CPU::CPU(Bus* bus) : bus_(bus) {
		A = 0; B = 0; C = 0; D = 0; E = 0; H = 0; L = 0;
		F = 0; SP = 0; PC = 0x0; IME = 1; R = 0;
		mClock = 0; tClock = 0;
		halt = false; stop = false;
		instructions =
		{
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
		};

		cbMap = {
			{ "RLCB" , &CPU::RLCB }, { "RLCC" , &CPU::RLCC }, { "RLCD" , &CPU::RLCD }, { "RLCE" , &CPU::RLCE }, { "RLCH" , &CPU::RLCH }, { "RLCL" , &CPU::RLCL }, { "RLCHL" , &CPU::RLCHL }, { "RLCAr" , &CPU::RLCAr },  { "RRCB" , &CPU::RRCB }, { "RRCC" , &CPU::RRCC }, { "RRCD" , &CPU::RRCD }, { "RRCE" , &CPU::RRCE }, { "RRCH" , &CPU::RRCH }, { "RRCL" , &CPU::RRCL }, { "RRCHL" , &CPU::RRCHL }, { "RRCAr" , &CPU::RRCAr },
			{ "RLB" , &CPU::RLB }, { "RLC" , &CPU::RLC }, { "RLD" , &CPU::RLD }, { "RLE" , &CPU::RLE }, { "RLH" , &CPU::RLH }, { "RLL" , &CPU::RLL }, { "RLHL" , &CPU::RLHL }, { "RLAr" , &CPU::RLAr }, { "RRB" , &CPU::RRB }, { "RRC" , &CPU::RRC },  { "RRD" , &CPU::RRD },  { "RRE" , &CPU::RRE },  { "RRH" , &CPU::RRH },  { "RRL" , &CPU::RRL },  { "RRHL" , &CPU::RRHL },  { "RRAr" , &CPU::RRAr },
			{ "SLAB" , &CPU::SLAB }, { "SLAC" , &CPU::SLAC }, { "SLAD" , &CPU::SLAD }, { "SLAE" , &CPU::SLAE }, { "SLAH" , &CPU::SLAH }, { "SLAL" , &CPU::SLAL }, { "???" , &CPU::XXX }, { "SLAA" , &CPU::SLAA }, { "SRAB" , &CPU::SRAB }, { "SRAC" , &CPU::SRAC }, { "SRAD" , &CPU::SRAD }, { "SRAE" , &CPU::SRAE }, { "SRAH" , &CPU::SRAH }, { "SRAL" , &CPU::SRAL }, { "???" , &CPU::XXX }, { "SRAA" , &CPU::SRAA },
			{ "SWAPB" , &CPU::SWAPB }, { "SWAPC" , &CPU::SWAPC }, { "SWAPD" , &CPU::SWAPD }, { "SWAPE" , &CPU::SWAPE }, { "SWAPH" , &CPU::SWAPH }, { "SWAPL" , &CPU::SWAPL }, { "???" , &CPU::XXX }, { "SWAPA" , &CPU::SWAPA }, { "SRLB" , &CPU::SRLB }, { "SRLC" , &CPU::SRLC }, { "SRLD" , &CPU::SRLD }, { "SRLE" , &CPU::SRLE }, { "SRLH" , &CPU::SRLH }, { "SRLL" , &CPU::SRLL }, { "???" , &CPU::XXX }, { "SRLA" , &CPU::SRLA },
			{ "BIT0B" , &CPU::BIT0B }, { "BIT0C" , &CPU::BIT0C }, { "BIT0D" , &CPU::BIT0D }, { "BIT0E" , &CPU::BIT0E }, { "BIT0H" , &CPU::BIT0H }, { "BIT0L" , &CPU::BIT0L }, { "BIT0M" , &CPU::BIT0M }, { "BIT0A" , &CPU::BIT0A }, { "BIT1B" , &CPU::BIT1B }, { "BIT1C" , &CPU::BIT1C }, { "BIT1D" , &CPU::BIT1D }, { "BIT1E" , &CPU::BIT1E }, { "BIT1H" , &CPU::BIT1H }, { "BIT1L" , &CPU::BIT1L }, { "BIT1M" , &CPU::BIT1M }, { "BIT1A" , &CPU::BIT1A },
			{ "BIT2B" , &CPU::BIT2B }, { "BIT2C" , &CPU::BIT2C }, { "BIT2D" , &CPU::BIT2D }, { "BIT2E" , &CPU::BIT2E }, { "BIT2H" , &CPU::BIT2H }, { "BIT2L" , &CPU::BIT2L }, { "BIT2M" , &CPU::BIT2M }, { "BIT2A" , &CPU::BIT2A }, { "BIT3B" , &CPU::BIT3B }, { "BIT3C" , &CPU::BIT3C }, { "BIT3D" , &CPU::BIT3D }, { "BIT3E" , &CPU::BIT3E }, { "BIT3H" , &CPU::BIT3H }, { "BIT3L" , &CPU::BIT3L }, { "BIT3M" , &CPU::BIT3M }, { "BIT3A" , &CPU::BIT3A },
			{ "BIT4B" , &CPU::BIT4B }, { "BIT4C" , &CPU::BIT4C }, { "BIT4D" , &CPU::BIT4D }, { "BIT4E" , &CPU::BIT4E }, { "BIT4H" , &CPU::BIT4H }, { "BIT4L" , &CPU::BIT4L }, { "BIT4M" , &CPU::BIT4M }, { "BIT4A" , &CPU::BIT4A }, { "BIT5B" , &CPU::BIT5B }, { "BIT5C" , &CPU::BIT5C }, { "BIT5D" , &CPU::BIT5D }, { "BIT5E" , &CPU::BIT5E }, { "BIT5H" , &CPU::BIT5H }, { "BIT5L" , &CPU::BIT5L }, { "BIT5M" , &CPU::BIT5M }, { "BIT5A" , &CPU::BIT5A },
			{ "BIT6B" , &CPU::BIT6B }, { "BIT6C" , &CPU::BIT6C }, { "BIT6D" , &CPU::BIT6D }, { "BIT6E" , &CPU::BIT6E }, { "BIT6H" , &CPU::BIT6H }, { "BIT6L" , &CPU::BIT6L }, { "BIT6M" , &CPU::BIT6M }, { "BIT6A" , &CPU::BIT6A }, { "BIT7B" , &CPU::BIT7B }, { "BIT7C" , &CPU::BIT7C }, { "BIT7D" , &CPU::BIT7D }, { "BIT7E" , &CPU::BIT7E }, { "BIT7H" , &CPU::BIT7H }, { "BIT7L" , &CPU::BIT7L }, { "BIT7M" , &CPU::BIT7M }, { "BIT7A" , &CPU::BIT7A },
			{ "RES0B" , &CPU::XXX }, { "RES0C" , &CPU::XXX }, { "RES0D" , &CPU::XXX }, { "RES0E" , &CPU::XXX }, { "RES0H" , &CPU::XXX }, { "RES01" , &CPU::XXX }, { "RES08" , &CPU::XXX }, { "RES0A" , &CPU::RES0A }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX },
			{ "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX },
			{ "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX },
			{ "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX },
			{ "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX },
			{ "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX },
			{ "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX },
			{ "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX }, { "???" , &CPU::XXX },
		};
	}
	inline void CPU::reg_dec(RegisterType& reg) {
		auto temp = reg - 1;
		auto flag = FLAG_NEG_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		flag |= (((reg & 0xF) - (1 & 0xF)) < 0) << FLAG_HCARRY_SHIFT;
		// Carry doesn't reset after DEC
		F &= FLAG_CARRY_MASK;
		F |= flag;
		reg = temp & 0xFF;
		mTemp = 1; tTemp = 4;
	}

	inline void CPU::reg_inc(RegisterType& reg) {
		auto temp = reg + 1;
		auto flag = FLAG_EMPTY_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		flag |= (((reg & 0xF) + (1 & 0xF)) > 0xF) << FLAG_HCARRY_SHIFT;
		// Carry doesn't reset after INC 
		F &= FLAG_CARRY_MASK;
		F |= flag;
		temp &= 0xFF;
		reg = temp;
		mTemp = 1; tTemp = 4;
	}

	inline void CPU::reg_sub(RegisterType& reg) {
		auto temp = A - reg;
		auto flag = FLAG_NEG_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		flag |= (((A & 0xF) - (reg & 0xF)) < 0) << FLAG_HCARRY_SHIFT;
		flag |= (temp < 0) << FLAG_CARRY_SHIFT;
		F = flag;
		A = temp & 0xFF;
		mTemp = 1; tTemp = 4;
	}

	inline void CPU::reg_sbc(RegisterType& reg) {
		bool carry = F & FLAG_CARRY_MASK;
		auto temp = A - reg - carry;
		auto flag = FLAG_NEG_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		flag |= (((A & 0xF) - (reg & 0xF) - carry) < 0) << FLAG_HCARRY_SHIFT;
		flag |= (temp < 0) << FLAG_CARRY_SHIFT;
		F = flag;
		A = temp & 0xFF;
		mTemp = 1; tTemp = 4;
	}

	inline void CPU::reg_and(RegisterType& reg) {
		auto temp = A & reg;
		auto flag = FLAG_HCARRY_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		F = flag;
		A = temp & 0xFF;
		mTemp = 1; tTemp = 4;
	}

	inline void CPU::reg_add(RegisterType& reg) {
		auto temp = A + reg;
		auto flag = FLAG_EMPTY_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		flag |= (((A & 0xF) + (reg & 0xF)) > 0xF) << FLAG_HCARRY_SHIFT;
		flag |= (temp > 0xFF) << FLAG_CARRY_SHIFT;
		F = flag;
		A = temp & 0xFF;
		mTemp = 1; tTemp = 4;
	}

	inline void CPU::reg_adc(RegisterType& reg) {
		bool carry = F & FLAG_CARRY_MASK;
		auto temp = A + reg + carry;
		auto flag = FLAG_EMPTY_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		flag |= (((A & 0xF) + (reg & 0xF) + carry) > 0xF) << FLAG_HCARRY_SHIFT;
		flag |= (temp > 0xFF) << FLAG_CARRY_SHIFT;
		F = flag;
		A = temp & 0xFF;
		mTemp = 1; tTemp = 4;
	}

	inline void CPU::reg_cmp(RegisterType& reg) {
		auto temp = A - reg;
		auto flag = FLAG_NEG_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		flag |= (((A & 0xF) - (reg & 0xF)) < 0) << FLAG_HCARRY_SHIFT;
		flag |= (temp < 0) << FLAG_CARRY_SHIFT;
		F = flag;
		mTemp = 1; tTemp = 4;
	}

	inline void CPU::hl_add(BigRegisterType& big_reg) {
		auto HL = (H << 8) | L;
		auto temp = HL + big_reg;
		auto flag = FLAG_EMPTY_MASK;
		flag |= (((HL & 0xFFF) + (big_reg & 0xFFF)) > 0xFFF) << FLAG_HCARRY_SHIFT;
		flag |= (temp > 0xFFFF) << FLAG_CARRY_SHIFT;
		F &= FLAG_ZERO_MASK;
		F |= flag;
		temp &= 0xFFFF;
		H = temp >> 8;
		L = temp & 0xFF;
		mTemp = 2; tTemp = 8;
	}

	inline void CPU::bit_ch(RegisterType reg, unsigned shift) {
		auto temp = reg & (1 << shift);
		auto flag = FLAG_HCARRY_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		F &= FLAG_CARRY_MASK;
		F |= flag;
		mTemp = 2; tTemp = 8;
	}

	inline void CPU::bit_swap(RegisterType& reg) {
		auto temp = ((reg & 0xF0) >> 4) | ((reg & 0x0F) << 4);
		auto flag = FLAG_EMPTY_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		F = flag;
		reg = temp & 0xFF;
		PC++;
		mTemp = 2; tTemp = 8;
	}

	void CPU::FZ(int i, bool as) {
		F = 0;
		if (!(i & 0xFF)) {
			F |= 0x80;
		}

		F |= as ? 0x40 : 0;
	}

	void CPU::RSV() {
		rsvA = A; rsvB = B;
		rsvC = C; rsvD = D;
		rsvE = E; rsvH = H;
		rsvL = L; rsvF = F;
	}

	void CPU::RRS() {
		A = rsvA; B = rsvB;
		C = rsvC; D = rsvD;
		E = rsvE; H = rsvH;
		L = rsvL; F = rsvF;
	}

	#pragma region Instructions

	void CPU::ADDAA() {
		reg_add(A);
	}

	void CPU::ADDAB() {
		reg_add(B);
	}

	void CPU::ADDAC() {
		reg_add(C);
	}

	void CPU::ADDAD() {
		reg_add(D);
	}

	void CPU::ADDAE() {
		reg_add(E);
	}

	void CPU::ADDAH() {
		reg_add(H);
	}

	void CPU::ADDAL() {
		reg_add(L);
	}

	void CPU::ADDAHL() {
		uint8_t t = bus_->Read((H << 8) | L);
		reg_add(t);
		mTemp = 2; tTemp = 8;
	}

	void CPU::ADDA8() {
		uint8_t t = bus_->Read(PC++);
		reg_add(t);
		mTemp = 2; tTemp = 8;
	}

	void CPU::ADDHLBC() {
		uint16_t t = (B << 8) | C;
		hl_add(t);
	}

	void CPU::ADDHLDE() {
		uint16_t t = (D << 8) | E;
		hl_add(t);
	}

	void CPU::ADDHLHL() {
		uint16_t t = (H << 8) | L;
		hl_add(t);
	}

	void CPU::ADDHLSP() {
		hl_add(SP);
	}

	void CPU::ADDSPD() {
		int val = bus_->Read(PC);
		auto temp = SP + ((val ^ 0x80) - 0x80);
		auto flag = FLAG_EMPTY_MASK;
		flag |= (((SP & 0xF) + (val & 0xF)) > 0xF) << FLAG_HCARRY_SHIFT;
		flag |= (((SP & 0xFF) + (val & 0xFF)) > 0xFF) << FLAG_CARRY_SHIFT;
		F = flag;
		temp &= 0xFFFF;
		SP = temp;
		PC++;
		mTemp = 4; tTemp = 16;
	}

	void CPU::ADCAA() {
		reg_adc(A);
	}

	void CPU::ADCAB() {
		reg_adc(B);
	}

	void CPU::ADCAC() {
		reg_adc(C);
	}

	void CPU::ADCAD() {
		reg_adc(D);
	}

	void CPU::ADCAE() {
		reg_adc(E);
	}

	void CPU::ADCAH() {
		reg_adc(H);
	}

	void CPU::ADCAL() {
		reg_adc(L);
	}

	void CPU::ADCAHL() {
		uint8_t t = bus_->Read((H << 8) | L);
		reg_adc(t);
		mTemp = 2; tTemp = 8;
	}

	void CPU::ADCA8() {
		uint8_t t = bus_->Read(PC++);
		reg_adc(t);
		mTemp = 2; tTemp = 8;
	}

	void CPU::SUBAA() {
		reg_sub(A);
	}

	void CPU::SUBAB() {
		reg_sub(B);
	}

	void CPU::SUBAC() {
		reg_sub(C);
	}

	void CPU::SUBAD() {
		reg_sub(D);
	}

	void CPU::SUBAE() {
		reg_sub(E);
	}

	void CPU::SUBAH() {
		reg_sub(H);
	}

	void CPU::SUBAL() {
		reg_sub(L);
	}

	void CPU::SUBAHL() {
		A -= bus_->Read((H << 8) | L);
		FZ(A, true);
		if (A < 0) {
			F |= 0x10;
		}
		SetFlagSubtract(true);
		A &= 0xFF;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SUBA8() {
		uint8_t t = bus_->Read(PC++);
		reg_sub(t);
		mTemp = 2; tTemp = 8;
	}

	void CPU::SBCAA() {
		reg_sbc(A);
	}

	void CPU::SBCAB() {
		reg_sbc(B);
	}

	void CPU::SBCAC() {
		reg_sbc(C);
	}

	void CPU::SBCAD() {
		reg_sbc(D);
	}

	void CPU::SBCAE() {
		reg_sbc(E);
	}

	void CPU::SBCAH() {
		reg_sbc(H);
	}

	void CPU::SBCAL() {
		reg_sbc(L);
	}

	void CPU::SBCAHL() {
		uint8_t t = bus_->Read(H << 8 | L);
		reg_sbc(t);
		mTemp = 2; tTemp = 8;
	}

	void CPU::SBCA8() {
		uint8_t t = bus_->Read(PC++);
		reg_sbc(t);
		mTemp = 2; tTemp = 8;
	}

	void CPU::CPAA() {
		reg_cmp(A);
	}

	void CPU::CPAB() {
		reg_cmp(B);
	}

	void CPU::CPAC() {
		reg_cmp(C);
	}

	void CPU::CPAD() {
		reg_cmp(D);
	}

	void CPU::CPAE() {
		reg_cmp(E);
	}

	void CPU::CPAH() {
		reg_cmp(H);
	}

	void CPU::CPAL() {
		reg_cmp(L);
	}

	void CPU::PUSHBC() {
		SP--;
		bus_->Write(SP, B);
		SP--;
		bus_->Write(SP, C);
		mTemp = 3; tTemp = 12;
	}

	void CPU::PUSHAF() {
		SP--;
		bus_->Write(SP, A);
		SP--;
		bus_->Write(SP, F);
		mTemp = 3; tTemp = 12;
	}

	void CPU::PUSHDE() {
		SP--;
		bus_->Write(SP, D);
		SP--;
		bus_->Write(SP, E);
		mTemp = 3; tTemp = 12;
	}

	void CPU::PUSHHL() {
		SP--;
		bus_->Write(SP, H);
		SP--;
		bus_->Write(SP, L);
		mTemp = 3; tTemp = 12;
	}

	void CPU::POPBC() {
		// TODO: remove & 0xFFFF and make ReadL
		B = bus_->Read((SP + 1) & 0xFFFF);
		C = bus_->Read(SP);
		SP += 2;
		mTemp = 3; tTemp = 12;
	}

	void CPU::POPAF() {
		//A = bus_->Read((SP + 1) & 0xFFFF);
		//F = bus_->Read(SP) & 0xF0 & 0xF0;
		auto t = bus_->ReadL(SP);
		A = (t >> 8) & 0xFF;
		F = t & 0xF0;
		SP += 2;
		SP &= 0xFFFF;
		mTemp = 3; tTemp = 12;
	}

	void CPU::POPDE() {
		E = bus_->Read(SP);
		SP++;
		D = bus_->Read(SP);
		SP++;
		mTemp = 3; tTemp = 12;
	}

	void CPU::POPHL() {
		L = bus_->Read(SP);
		SP++;
		H = bus_->Read(SP);
		SP++;
		mTemp = 3; tTemp = 12;
	}

	void CPU::LDABC() {
		int addr = C | (B << 8);
		A = bus_->Read(addr);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDADE() {
		int addr = E | (D << 8);
		A = bus_->Read(addr);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDAA() {
		A = A;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDAB() {
		A = B;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDAC() {
		A = C;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDAD() {
		A = D;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDAE() {
		A = E;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDAH() {
		A = H;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDAL() {
		A = L;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDBA() {
		B = A;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDBB() {
		B = B;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDBC() {
		B = C;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDBD() {
		B = D;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDBE() {
		B = E;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDBH() {
		B = H;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDBL() {
		B = L;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDCA() {
		C = A;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDCB() {
		C = B;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDCC() {
		C = C;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDCD() {
		C = D;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDCE() {
		C = E;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDCH() {
		C = H;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDCL() {
		C = L;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDDA() {
		D = A;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDDB() {
		D = B;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDDC() {
		D = C;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDDD() {
		D = D;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDDE() {
		D = E;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDDH() {
		D = H;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDDL() {
		D = L;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDEA() {
		E = A;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDEB() {
		E = B;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDEC() {
		E = C;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDED() {
		E = D;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDEE() {
		E = E;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDEH() {
		E = H;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDEL() {
		E = L;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDHA() {
		H = A;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDHB() {
		H = B;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDHC() {
		H = C;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDHD() {
		H = D;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDHE() {
		H = E;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDHH() {
		H = H;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDHL() {
		H = L;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDLA() {
		L = A;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDLB() {
		L = B;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDLC() {
		L = C;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDLD() {
		L = D;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDLE() {
		L = E;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDLH() {
		L = H;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDLL() {
		L = L;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDAHL() {
		A = bus_->Read((H << 8) | L);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDBHL() {
		B = bus_->Read((H << 8) | L);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDCHL() {
		C = bus_->Read((H << 8) | L);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDDHL() {
		D = bus_->Read((H << 8) | L);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDEHL() {
		E = bus_->Read((H << 8) | L);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDHHL() {
		H = bus_->Read((H << 8) | L);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDLHL() {
		L = bus_->Read((H << 8) | L);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDHLA() {
		bus_->Write((H << 8) | L, A);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDHLB() {
		bus_->Write((H << 8) | L, B);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDHLC() {
		bus_->Write((H << 8) | L, C);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDHLD() {
		bus_->Write((H << 8) | L, D);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDHLE() {
		bus_->Write((H << 8) | L, E);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDHLH() {
		bus_->Write((H << 8) | L, H);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDHLL() {
		bus_->Write((H << 8) | L, L);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDA8() {
		A = bus_->Read(PC);
		PC++;
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDB8() {
		B = bus_->Read(PC);
		PC++;
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDC8() {
		C = bus_->Read(PC);
		PC++;
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDD8() {
		D = bus_->Read(PC);
		PC++;
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDE8() {
		E = bus_->Read(PC);
		PC++;
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDH8() {
		H = bus_->Read(PC);
		PC++;
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDL8() {
		L = bus_->Read(PC);
		PC++;
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDHL8() {
		bus_->Write((H << 8) | L, bus_->Read(PC));
		PC++;
		mTemp = 3; tTemp = 12;
	}

	void CPU::LDBCA() {
		bus_->Write((B << 8) | C, A);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDDEA() {
		bus_->Write((D << 8) | E, A);
		mTemp = 2; tTemp = 8;
	}

	void CPU::LD16A() {
		bus_->Write(bus_->ReadL(PC), A);
		PC += 2;
		mTemp = 4; tTemp = 16;
	}

	void CPU::LDA16() {
		A = bus_->Read(bus_->ReadL(PC));
		PC += 2;
		mTemp = 4; tTemp = 16;
	}

	void CPU::LDBC16() {
		C = bus_->Read(PC);
		B = bus_->Read(PC + 1);
		PC += 2;
		mTemp = 3; tTemp = 12;
	}

	void CPU::LDDE16() {
		E = bus_->Read(PC);
		D = bus_->Read(PC + 1);
		PC += 2;
		mTemp = 3; tTemp = 12;
	}

	void CPU::LDHL16() {
		L = bus_->Read(PC);
		H = bus_->Read(PC + 1);
		PC += 2;
		mTemp = 3; tTemp = 12;
	}

	void CPU::LD16SP() {
		bus_->Write(bus_->ReadL(PC), SP & 0xFF);
		bus_->Write(bus_->ReadL(PC) + 1, (SP >> 8) & 0xFF);
		PC += 2;
		mTemp = 5; tTemp = 20;
	}

	void CPU::INCA() {
		reg_inc(A);
	}

	void CPU::INCB() {
		reg_inc(B);
	}

	void CPU::INCC() {
		reg_inc(C);
	}

	void CPU::INCD() {
		reg_inc(D);
	}

	void CPU::INCE() {
		reg_inc(E);
	}

	void CPU::INCH() {
		reg_inc(H);
	}

	void CPU::INCL() {
		reg_inc(L);
	}

	void CPU::INCHLR() {
		uint8_t t = bus_->Read((H << 8) | L);
		reg_inc(t);
		bus_->Write((H << 8) | L, t);
		mTemp = 3; tTemp = 12;
	}

	void CPU::DECA() {
		reg_dec(A);
	}

	void CPU::DECB() {
		reg_dec(B);
	}

	void CPU::DECC() {
		reg_dec(C);
	}

	void CPU::DECD() {
		reg_dec(D);
	}

	void CPU::DECE() {
		reg_dec(E);
	}

	void CPU::DECH() {
		reg_dec(H);
	}

	void CPU::DECL() {
		reg_dec(L);
	}

	void CPU::DECHLR() {
		uint8_t t = bus_->Read((H << 8) | L);
		reg_dec(t);
		bus_->Write((H << 8) | L, t);
		mTemp = 3; tTemp = 12;
	}

	void CPU::INCBC() {
		C = (C + 1) & 0xFF;
		if (!C) {
			B = (B + 1) & 0xFF;
		}
		mTemp = 1; tTemp = 4;
	}

	void CPU::INCDE() {
		E = (E + 1) & 0xFF;
		if (!E) {
			D = (D + 1) & 0xFF;
		}
		mTemp = 1; tTemp = 4;
	}

	void CPU::INCHL() {
		L = (L + 1) & 0xFF;
		if (!L) {
			H = (H + 1) & 0xFF;
		}
		mTemp = 1; tTemp = 4;
	}

	void CPU::INCSP() {
		SP++;
		mTemp = 1; tTemp = 4;
	}

	void CPU::DECBC() {
		C = (C - 1) & 0xFF;
		if (C == 0xFF) {
			B = (B - 1) & 0xFF;
		}
		mTemp = 1; tTemp = 4;
	}

	void CPU::DECDE() {
		E = (E - 1) & 0xFF;
		if (E == 0xFF) {
			D = (D - 1) & 0xFF;
		}
		mTemp = 1; tTemp = 4;
	}

	void CPU::DECHL() {
		L = (L - 1) & 0xFF;
		if (L == 0xFF) {
			H = (H - 1) & 0xFF;
		}
		mTemp = 1; tTemp = 4;
	}

	void CPU::DECSP() {
		SP = (SP - 1) & 0xFFFF;
		mTemp = 1; tTemp = 4;
	}

	void CPU::JP16() {
		PC = bus_->ReadL(PC);
		mTemp = 4; tTemp = 16;
	}

	void CPU::JPHL() {
		PC = (H << 8) | L;
		mTemp = 1; tTemp = 4;
	}

	void CPU::JPNZ16() {
		mTemp = 3; tTemp = 12;
		if ((F & 0x80) == 0x00) {
			PC = bus_->ReadL(PC);
			mTemp++; tTemp += 4;
		}
		else
			PC += 2;
	}

	void CPU::JPZ16() {
		mTemp = 3; tTemp = 12;
		if ((F & 0x80) == 0x80) {
			PC = bus_->ReadL(PC);
			mTemp++; tTemp += 4;
		}
		else
			PC += 2;
	}

	void CPU::JPNC16() {
		mTemp = 3; tTemp = 12;
		if ((F & 0x10) == 0x00) {
			PC = bus_->ReadL(PC);
			mTemp++; tTemp += 4;
		}
		else
			PC += 2;
	}

	void CPU::JPC16() {
		mTemp = 3; tTemp = 12;
		if ((F & 0x10) == 0x10) {
			PC = bus_->ReadL(PC);
			mTemp++; tTemp += 4;
		}
		else
			PC += 2;
	}

	void CPU::JR8() {
		int i = bus_->Read(PC);
		if (i >= 0x80)
			i = -((~i + 1) & 255);
		PC++;
		mTemp = 2; tTemp = 8;
		PC += i;
		mTemp++; tTemp += 4;
	}

	void CPU::JRNZ8() {
		int i = bus_->Read(PC);
		PC++;
		if ((F & (FLAG_ZERO_MASK)) == 0) {
			PC += ((i ^ 0x80) - 0x80);
			mTemp = 3; tTemp = 12;
		}
		else {
			mTemp = 2; tTemp = 8;
		}
	}

	void CPU::JRZ8() {
		int i = bus_->Read(PC);
		if (i >= 0x80)
			i = -((~i + 1) & 255);
		PC++;
		mTemp = 2; tTemp = 8;
		if ((F & 0x80) == 0x80) {
			PC += i;
			mTemp += 1; tTemp += 4;
		}
	}

	void CPU::JRNC8() {
		int i = bus_->Read(PC);
		if (i >= 0x80)
			i = -((~i + 1) & 255);
		PC++;
		mTemp = 2; tTemp = 8;
		if ((F & 0x10) == 0x00) {
			PC += i;
			mTemp++; tTemp += 4;
		}
	}

	void CPU::JRC8() {
		int i = bus_->Read(PC);
		if (i >= 0x80)
			i = -((~i + 1) & 255);
		PC++;
		mTemp = 2; tTemp = 8;
		if ((F & 0x10) == 0x10) {
			PC += i;
			mTemp++; tTemp += 4;
		}
	}

	void CPU::ANDA() {
		reg_and(A);
	}

	void CPU::ANDB() {
		reg_and(B);
	}

	void CPU::ANDC() {
		reg_and(C);
	}

	void CPU::ANDD() {
		reg_and(D);
	}

	void CPU::ANDE() {
		reg_and(E);
	}

	void CPU::ANDH() {
		reg_and(H);
	}

	void CPU::ANDL() {
		reg_and(L);
	}

	void CPU::ANDHL() {
		uint8_t t = bus_->Read((H << 8) | L);
		reg_and(t);
		mTemp = 2; tTemp = 8;
	}

	void CPU::AND8() {
		uint8_t t = bus_->Read(PC++);
		reg_and(t);
		mTemp = 2; tTemp = 8;
	}

	void CPU::ORA() {
		A |= A;
		A &= 0xFF;
		FZ(A);
		SetFlagCarry(false);
		mTemp = 1; tTemp = 4;
	}

	void CPU::ORB() {
		A |= B;
		A &= 0xFF;
		FZ(A);
		SetFlagCarry(false);
		mTemp = 1; tTemp = 4;
	}

	void CPU::ORC() {
		A |= C;
		A &= 0xFF;
		FZ(A);
		SetFlagCarry(false);
		mTemp = 1; tTemp = 4;
	}

	void CPU::ORD() {
		A |= D;
		A &= 0xFF;
		FZ(A);
		SetFlagCarry(false);
		mTemp = 1; tTemp = 4;
	}

	void CPU::ORE() {
		A |= E;
		A &= 0xFF;
		FZ(A);
		SetFlagCarry(false);
		mTemp = 1; tTemp = 4;
	}

	void CPU::ORH() {
		A |= H;
		A &= 0xFF;
		FZ(A);
		SetFlagCarry(false);
		mTemp = 1; tTemp = 4;
	}

	void CPU::ORL() {
		A |= L;
		A &= 0xFF;
		FZ(A);
		SetFlagCarry(false);
		mTemp = 1; tTemp = 4;
	}

	void CPU::ORHL() {
		A |= bus_->Read((H << 8) | L);
		A &= 0xFF;
		F = A ? 0 : 0x80;
		mTemp = 2; tTemp = 8;
	}

	void CPU::OR8() {
		A |= bus_->Read(PC);
		A &= 0xFF;
		PC++;
		FZ(A);
		SetFlagCarry(false);
		mTemp = 2; tTemp = 8;
	}

	void CPU::XORA() {
		A ^= A;
		A &= 0xFF;
		FZ(A);
		mTemp = 1; tTemp = 4;
	}

	void CPU::XORB() {
		A ^= B;
		A &= 0xFF;
		FZ(A);
		mTemp = 1; tTemp = 4;
	}

	void CPU::XORC() {
		A ^= C;
		A &= 0xFF;
		FZ(A);
		mTemp = 1; tTemp = 4;
	}

	void CPU::XORD() {
		A ^= D;
		A &= 0xFF;
		FZ(A);
		mTemp = 1; tTemp = 4;
	}

	void CPU::XORE() {
		A ^= E;
		A &= 0xFF;
		FZ(A);
		mTemp = 1; tTemp = 4;
	}

	void CPU::XORH() {
		A ^= H;
		A &= 0xFF;
		FZ(A);
		mTemp = 1; tTemp = 4;
	}

	void CPU::XORL() {
		A ^= L;
		A &= 0xFF;
		FZ(A);
		mTemp = 1; tTemp = 4;
	}

	void CPU::XORHL() {
		A ^= bus_->Read((H << 8) | L);
		A &= 0xFF;
		FZ(A);
		mTemp = 2; tTemp = 8;
	}

	void CPU::XOR8() {
		A ^= bus_->Read(PC);
		A &= 0xFF;
		PC++;
		FZ(A);
		mTemp = 2; tTemp = 8;
	}

	void CPU::NOP() {
		mTemp = 1; tTemp = 4;
	}

	void CPU::STOP() {
		int i = bus_->Read(PC);
		if (i >= 0x80) {
			i = -((~i + 1) & 0xFF);
		}
		PC++;
		mTemp = 2;
		tTemp = 8;
		B--;
		if (B != 0) {
			PC += i;
			mTemp++; tTemp += 4;
		}
	}

	void CPU::RET() {
		PC = bus_->ReadL(SP);
		SP += 2;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RETI() {
		IME = 1;
		RRS();
		PC = bus_->ReadL(SP);
		SP += 2;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RETNZ() {
		mTemp = 1; tTemp = 4;
		if ((F & 0x80) == 0x00) {
			PC = bus_->ReadL(SP);
			SP += 2;
			mTemp += 2; tTemp += 8;
		}
	}

	void CPU::RETZ() {
		mTemp = 1; tTemp = 4;
		if ((F & 0x80) == 0x80) {
			PC = bus_->ReadL(SP);
			SP += 2;
			mTemp += 2; tTemp += 8;
		}
	}

	void CPU::RETNC() {
		mTemp = 1; tTemp = 4;
		if ((F & 0x10) == 0x00) {
			PC = bus_->ReadL(SP);
			SP += 2;
			mTemp += 2; tTemp += 8;
		}
	}

	void CPU::RETC() {
		mTemp = 1; tTemp = 4;
		if ((F & 0x10) == 0x10) {
			PC = bus_->ReadL(SP);
			SP += 2;
			mTemp += 2; tTemp += 8;
		}
	}

	void CPU::RST0() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x00;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST8() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x08;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST10() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x10;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST18() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x18;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST20() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x20;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST28() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x28;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST30() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x30;
		mTemp = 3; tTemp = 12;
	}

	void CPU::LDSPHL() {
		SP = (H << 8) | L;
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDAMC() {
		A = bus_->Read(0xFF00 + C);
		mTemp = 2; tTemp = 8;
	}

	void CPU::RST38() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x38;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST40() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x40;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST48() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x48;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST50() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x50;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST58() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x58;
		mTemp = 3; tTemp = 12;
	}

	void CPU::RST60() {
		RSV();
		SP -= 2;
		bus_->WriteL(SP, PC);
		PC = 0x60;
		mTemp = 3; tTemp = 12;
	}

	void CPU::DI() {
		IME = 0; mTemp = 1; tTemp = 4;
	}

	void CPU::EI() {
		IME = 1; mTemp = 1; tTemp = 4;
	}

	void CPU::RLA() {
		bool carry = F & FLAG_CARRY_MASK;
		auto temp = (A << 1) + carry;
		auto flag = FLAG_EMPTY_MASK;
		flag |= (temp > 0xFF) << FLAG_CARRY_SHIFT;
		F = flag;
		A = temp & 0xFF;
		mTemp = 1; tTemp = 4;
	}

	void CPU::RLCA() {
		auto temp = (A << 1) + (A >> 7);
		auto flag = FLAG_EMPTY_MASK;
		flag |= (temp > 0xFF) << FLAG_CARRY_SHIFT;
		F = flag;
		A = temp & 0xFF;
		mTemp = 1; tTemp = 4;
	}

	void CPU::RRA() {
		bool carry = F & FLAG_CARRY_MASK;
		auto temp = (A >> 1) + ((carry) << 7) + ((A & 1) << 8);
		auto flag = FLAG_EMPTY_MASK;
		flag |= (temp > 0xFF) << FLAG_CARRY_SHIFT;
		F = flag;
		temp &= 0xFF;
		A = temp;
		mTemp = 1; tTemp = 4;
	}

	void CPU::RRCA() {
		auto temp = (A >> 1) + ((A & 1) << 7) + ((A & 1) << 8);
		auto flag = FLAG_EMPTY_MASK;
		flag += (temp > 0xFF) << FLAG_CARRY_SHIFT;
		F = flag;
		temp &= 0xFF;
		A = temp;
		mTemp = 1; tTemp = 4;
	}

	void CPU::CALL16() {
		SP -= 2;
		bus_->WriteL(SP, PC + 2);
		PC = bus_->ReadL(PC);
		mTemp = 5; tTemp = 20;
	}

	void CPU::CALLNZ16() {
		if (!(F & FLAG_ZERO_MASK)) {
			SP -= 2;
			bus_->WriteL(SP, PC + 2);
			PC = bus_->ReadL(PC);
			mTemp += 3; tTemp += 12;
		}
		else {
			PC += 2;
		}
		mTemp += 3; tTemp += 12;
	}

	void CPU::CALLZ16() {
		if ((F & 0x80) == 0x80) {
			SP -= 2;
			bus_->WriteL(SP, PC + 2);
			PC = bus_->ReadL(PC);
			mTemp += 2; tTemp += 8;
		}
		else {
			PC += 2;
		}
		mTemp = 3; tTemp = 12;
	}

	void CPU::CALLNC16() {
		if ((F & 0x10) == 0x00) {
			SP -= 2;
			bus_->WriteL(SP, PC + 2);
			PC = bus_->ReadL(PC);
			mTemp += 2; tTemp += 8;
		}
		else {
			PC += 2;
		}
		mTemp = 3; tTemp = 12;
	}

	void CPU::CALLC16() {
		if ((F & 0x10) == 0x10) {
			SP -= 2;
			bus_->WriteL(SP, PC + 2);
			PC = bus_->ReadL(PC);
			mTemp += 2; tTemp += 8;
		}
		else {
			PC += 2;
		}
		mTemp = 3; tTemp = 12;
	}

	void CPU::LDSP16() {
		SP = bus_->ReadL(PC);
		PC += 2;
		mTemp = 3; tTemp = 12;
	}

	void CPU::LDDHLA() {
		bus_->Write((H << 8) | L, A);
		L = (L - 1) & 0xFF;
		if (L == 0xFF) {
			H = (H - 1) & 0xFF;
		}
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDDAHL() {
		A = bus_->Read((H << 8) | L);
		L = (L - 1) & 0xFF;
		if (L == 0xFF) {
			H = (H - 1) & 0xFF;
		}
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDIHLA() {
		bus_->Write((H << 8) | L, A);
		L = (L + 1) & 0xFF;
		if (!L) {
			H = (H + 1) & 0xFF;
		}
		mTemp = 2; tTemp = 8;
	}

	void CPU::LDIAHL() {
		A = bus_->Read((H << 8) | L);
		L = (L + 1) & 0xFF;
		if (!L) {
			H = (H + 1) & 0xFF;
		}
		mTemp = 2; tTemp = 8;
	}

	void CPU::DAA() {
		int temp = A;
		uint16_t corr = 0;
		corr |= ((F & FLAG_HCARRY_MASK) ? 0x06 : 0x00);
		corr |= ((F & FLAG_CARRY_MASK) ? 0x60 : 0x00);
		if (F & FLAG_NEG_MASK) {
			temp -= corr;
		}
		else {
			corr |= ((temp & 0x0F) > 0x09) ? 0x06 : 0;
			corr |= (temp > 0x99) ? 0x60 : 0;
			temp += corr;
		}
		uint16_t flag = FLAG_EMPTY_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		flag |= ((corr & 0x60) != 0) << FLAG_CARRY_SHIFT;
		F &= FLAG_NEG_MASK;
		F |= flag;
		temp &= 0xFF;
		A = temp;
		mTemp = 1; tTemp = 4;
	}

	void CPU::CPL() {
		A = (~A) & 0xFF;
		F &= 0b1001'0000;
		F |= 0b0110'0000;
		mTemp = 1; tTemp = 4;
	}

	void CPU::SCF() {
		F &= 0b1000'0000;
		F |= 0b0001'0000;
		mTemp = 1; tTemp = 4;
	}

	void CPU::CCF() {
		auto flag = (F & 0b0001'0000) ^ 0b0001'0000;
		F &= 0b1000'0000;
		F |= flag;
		mTemp = 1; tTemp = 4;
	}

	void CPU::LDHLSPD() {
		auto val = bus_->Read(PC);
		auto HL = SP + ((val ^ 0x80) - 0x80);
		H = (HL >> 8) & 0xFF;
		L = HL & 0xFF;
		auto flag = FLAG_EMPTY_MASK;
		flag |=	(((SP & 0xF) + (val & 0xF)) > 0xF) << FLAG_HCARRY_SHIFT;
		flag |= (((SP & 0xFF) + (val & 0xFF)) > 0xFF) << FLAG_CARRY_SHIFT;
		F = flag;
		PC++;
	}

	void CPU::LDHA8() {
		int k = 0xFF00 + bus_->Read(PC);
		A = bus_->Read(0xFF00 + bus_->Read(PC));
		PC++;
		mTemp = 3; tTemp = 12;
	}

	void CPU::LDH8A() {
		bus_->Write(0xFF00 + bus_->Read(PC), A);
		PC++;
		mTemp = 3; tTemp = 12;
	}

	void CPU::LDHCA() {
		bus_->Write(0xFF00 + C, A);
		mTemp = 2; tTemp = 8;
	}

	void CPU::EXT() {
		int i = bus_->Read(PC);
		auto s = cbMap[i].name;
		PC++;
		PC &= 0xFFFF;
		if (i < 0xFF) {
			(this->*cbMap[i].op)();
		}
	}

	void CPU::CPAHL() {
		int i = A;
		i -= bus_->Read(((H << 8) | L));
		FZ(i, true);
		if (i < 0) {
			F |= 0x10;
		}
		i &= 0xFF;
		mTemp = 2; tTemp = 8;
	}

	void CPU::CP8() {
		uint16_t m = bus_->Read(PC);
		int temp = A - m;
		auto flag = FLAG_NEG_MASK;
		flag |= ((temp & 0xFF) == 0) << FLAG_ZERO_SHIFT;
		flag |= (((A & 0x0F) - (m & 0x0F)) < 0) << FLAG_HCARRY_SHIFT;
		flag |= (temp < 0) << FLAG_CARRY_SHIFT;
		F = flag;
		PC++;
		mTemp = 2; tTemp = 8;
		//PC++;
		//F = (i < 0) ? 0x50 : 0x40;
		//i &= 0xFF;
		//if (i == 0) {
		//	SetFlagZero(true);
		//}
		//if ((A ^ i ^ m) & 0x10) F |= 0x20;
		//SetFlagSubtract(true);
	}

	void CPU::HALT() {
		halt = true;
		mTemp = 1; tTemp = 4;
	}

	void CPU::XXX() {
		stop = true;
		//std::cout << "Invalid instruction at 0x" + std::to_string(PC - 1) << std::endl;
		//std::cout << instructions[bus_->Read(PC - 1)].name << bus_->Read(PC - 1) << std::endl;
	}



	#pragma endregion

	#pragma region TwoByteInstructions

	void CPU::RLCB() {
		int i = B & 0x80 ? 1 : 0;
		int o = B & 0x80 ? 0x10 : 0;
		B = (B << 1) + i;
		B &= 0xFF;
		F = B ? 0 : 0x80;
		F = (F & 0xEF) + o;
		mTemp = 2;
	}

	void CPU::RLCC() {
		int i = C & 0x80 ? 1 : 0;
		int o = C & 0x80 ? 0x10 : 0;
		C = (C << 1) + i;
		C &= 0xFF;
		F = C ? 0 : 0x80;
		F = (F & 0xEF) + o;
		mTemp = 2;
	}

	void CPU::RLCD() {
		int i = D & 0x80 ? 1 : 0;
		int o = D & 0x80 ? 0x10 : 0;
		D = (D << 1) + i;
		D &= 0xFF;
		F = D ? 0 : 0x80;
		F = (F & 0xEF) + o;
		mTemp = 2;
	}

	void CPU::RLCE() {
		int i = E & 0x80 ? 1 : 0;
		int o = E & 0x80 ? 0x10 : 0;
		E = (E << 1) + i;
		E &= 0xFF;
		F = E ? 0 : 0x80;
		F = (F & 0xEF) + o;
		mTemp = 2;
	}

	void CPU::RLCH() {
		int i = H & 0x80 ? 1 : 0;
		int o = H & 0x80 ? 0x10 : 0;
		H = (H << 1) + i;
		H &= 0xFF;
		F = H ? 0 : 0x80;
		F = (F & 0xEF) + o;
		mTemp = 2;
	}

	void CPU::RLCL() {
		int i = L & 0x80 ? 1 : 0;
		int o = L & 0x80 ? 0x10 : 0;
		L = (L << 1) + i;
		L &= 0xFF;
		F = L ? 0 : 0x80;
		F = (F & 0xEF) + o;
		mTemp = 2;
	}

	void CPU::RLCAr() {
		int i = A & 0x80 ? 1 : 0;
		int o = A & 0x80 ? 0x10 : 0;
		A = (A << 1) + i;
		A &= 0xFF;
		F = A ? 0 : 0x80;
		F = (F & 0xEF) + o;
		mTemp = 2;
	}

	void CPU::RLCHL() {
		int i = bus_->Read((H << 8) | L);
		int ci = (i & 0x80) ? 1 : 0;
		int co = (i & 0x80) ? 0x10 : 0;
		i = (i << 1) + ci;
		i &= 0xFF;
		F = (i) ? 0 : 0x80;
		bus_->Write((H << 8) | L, i);
		F = (F & 0xEF) + co;
		mTemp = 4;
	}

	void CPU::RRCB() {
		int ci = B & 1 ? 0x80 : 0;
		int co = B & 1 ? 0x10 : 0;
		B = (B >> 1) + ci;
		B &= 0xFF;
		FZ(B);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRCC() {
		int ci = C & 1 ? 0x80 : 0;
		int co = C & 1 ? 0x10 : 0;
		C = (C >> 1) + ci;
		C &= 0xFF;
		FZ(C);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRCD() {
		int ci = D & 1 ? 0x80 : 0;
		int co = D & 1 ? 0x10 : 0;
		D = (D >> 1) + ci;
		D &= 0xFF;
		FZ(D);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRCE() {
		int ci = E & 1 ? 0x80 : 0;
		int co = E & 1 ? 0x10 : 0;
		E = (E >> 1) + ci;
		E &= 0xFF;
		FZ(E);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRCH() {
		int ci = H & 1 ? 0x80 : 0;
		int co = H & 1 ? 0x10 : 0;
		H = (H >> 1) + ci;
		H &= 0xFF;
		FZ(H);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRCL() {
		int ci = L & 1 ? 0x80 : 0;
		int co = L & 1 ? 0x10 : 0;
		L = (L >> 1) + ci;
		L &= 0xFF;
		FZ(L);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRCAr() {
		int ci = A & 1 ? 0x80 : 0;
		int co = A & 1 ? 0x10 : 0;
		A = (A >> 1) + ci;
		A &= 0xFF;
		FZ(A);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRCHL() {
		int i = bus_->Read((H << 8) | L);
		int ci = i & 1 ? 0x80 : 0;
		int co = i & 1 ? 0x10 : 0;
		i = (i >> 1) + ci;
		i &= 0xFF;
		bus_->Write((H << 8) | L, i);
		F = (i) ? 0 : 0x80;
		F = (F & 0xEF) + co;
		mTemp = 4; tTemp = 16;
	}

	void CPU::RLB() {
		int ci = F & 0x10 ? 1 : 0;
		int co = B & 0x80 ? 0x10 : 0;
		B = (B << 1) + ci;
		B &= 0xFF;
		FZ(B);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RLC() {
		int ci = F & 0x10 ? 1 : 0;
		int co = C & 0x80 ? 0x10 : 0;
		C = (C << 1) + ci;
		C &= 0xFF;
		FZ(C);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RLD() {
		int ci = F & 0x10 ? 1 : 0;
		int co = D & 0x80 ? 0x10 : 0;
		D = (D << 1) + ci;
		D &= 0xFF;
		FZ(D);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RLE() {
		int ci = F & 0x10 ? 1 : 0;
		int co = E & 0x80 ? 0x10 : 0;
		E = (E << 1) + ci;
		E &= 0xFF;
		FZ(E);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RLH() {
		int ci = F & 0x10 ? 1 : 0;
		int co = H & 0x80 ? 0x10 : 0;
		H = (H << 1) + ci;
		H &= 0xFF;
		FZ(H);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RLL() {
		int ci = F & 0x10 ? 1 : 0;
		int co = L & 0x80 ? 0x10 : 0;
		L = (L << 1) + ci;
		L &= 0xFF;
		FZ(L);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RLAr() {
		int ci = F & 0x10 ? 1 : 0;
		int co = A & 0x80 ? 0x10 : 0;
		A = (A << 1) + ci;
		A &= 0xFF;
		FZ(A);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RLHL() {
		int i = bus_->Read((H << 8) | L);
		int ci = F & 0x10 ? 1 : 0;
		int co = i & 0x80 ? 0x10 : 0;
		i = (i << 1) + ci;
		i &= 0xFF;
		FZ(i);
		bus_->Write((H << 8) | L, i);
		F = (F & 0xEF) + co;
		mTemp = 4; tTemp = 16;
	}

	void CPU::RRB() {
		int ci = F & 0x10 ? 0x80 : 0;
		int co = B & 1 ? 0x10 : 0;
		B = (B >> 1) + ci;
		B &= 0xFF;
		FZ(B);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRC() {
		int ci = F & 0x10 ? 0x80 : 0;
		int co = C & 1 ? 0x10 : 0;
		C = (C >> 1) + ci;
		C &= 0xFF;
		FZ(C);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRD() {
		int ci = F & 0x10 ? 0x80 : 0;
		int co = D & 1 ? 0x10 : 0;
		D = (D >> 1) + ci;
		D &= 0xFF;
		FZ(D);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRE() {
		int ci = F & 0x10 ? 0x80 : 0;
		int co = E & 1 ? 0x10 : 0;
		E = (E >> 1) + ci;
		E &= 0xFF;
		FZ(E);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRH() {
		int ci = F & 0x10 ? 0x80 : 0;
		int co = H & 1 ? 0x10 : 0;
		H = (H >> 1) + ci;
		H &= 0xFF;
		FZ(H);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRL() {
		int ci = F & 0x10 ? 0x80 : 0;
		int co = L & 1 ? 0x10 : 0;
		L = (L >> 1) + ci;
		L &= 0xFF;
		FZ(L);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRAr() {
		int ci = F & 0x10 ? 0x80 : 0;
		int co = A & 1 ? 0x10 : 0;
		A = (A >> 1) + ci;
		A &= 0xFF;
		FZ(A);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::RRHL() {
		int i = bus_->Read((H << 8) | L);
		int ci = F & 0x10 ? 1 : 0;
		int co = i & 0x80 ? 0x10 : 0;
		i = (i << 1) + ci;
		i &= 0xFF;
		FZ(i);
		bus_->Write((H << 8) | L, i);
		F = (F & 0xEF) + co;
		mTemp = 4; tTemp = 16;
	}

	void CPU::SLAB() {
		int co = B & 0x80 ? 0x10 : 0;
		B = (B << 1) & 0xFF;
		FZ(B);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SLAC() {
		int co = C & 0x80 ? 0x10 : 0;
		C = (C << 1) & 0xFF;
		FZ(C);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SLAD() {
		int co = D & 0x80 ? 0x10 : 0;
		D = (D << 1) & 0xFF;
		FZ(D);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SLAE() {
		int co = E & 0x80 ? 0x10 : 0;
		E = (E << 1) & 0xFF;
		FZ(E);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SLAH() {
		int co = H & 0x80 ? 0x10 : 0;
		H = (H << 1) & 0xFF;
		FZ(H);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SLAL() {
		int co = L & 0x80 ? 0x10 : 0;
		L = (L << 1) & 0xFF;
		FZ(L);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SLAA() {
		int co = A & 0x80 ? 0x10 : 0;
		A = (A << 1) & 0xFF;
		FZ(A);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRAB() {
		int ci = B & 0x80;
		int co = B & 1 ? 0x10 : 0;
		B = ((B >> 1) + ci) & 0xFF;
		FZ(B);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRAC() {
		int ci = C & 0x80;
		int co = C & 1 ? 0x10 : 0;
		C = ((C >> 1) + ci) & 0xFF;
		FZ(C);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRAD() {
		int ci = D & 0x80;
		int co = D & 1 ? 0x10 : 0;
		D = ((D >> 1) + ci) & 0xFF;
		FZ(D);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRAE() {
		int ci = E & 0x80;
		int co = E & 1 ? 0x10 : 0;
		E = ((E >> 1) + ci) & 0xFF;
		FZ(E);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRAH() {
		int ci = H & 0x80;
		int co = H & 1 ? 0x10 : 0;
		H = ((H >> 1) + ci) & 0xFF;
		FZ(H);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRAL() {
		int ci = L & 0x80;
		int co = L & 1 ? 0x10 : 0;
		L = ((L >> 1) + ci) & 0xFF;
		FZ(L);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRAA() {
		int ci = A & 0x80;
		int co = A & 1 ? 0x10 : 0;
		A = ((A >> 1) + ci) & 0xFF;
		FZ(A);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SWAPB() {
		bit_swap(B);
	}

	void CPU::SWAPC() {
		bit_swap(C);
	}

	void CPU::SWAPD() {
		bit_swap(D);
	}

	void CPU::SWAPE() {
		bit_swap(E);
	}

	void CPU::SWAPH() {
		bit_swap(H);
	}

	void CPU::SWAPL() {
		bit_swap(L);
	}

	void CPU::SWAPA() {
		bit_swap(A);
	}

	void CPU::SRLB() {
		int co = B & 1 ? 0x10 : 0;
		B = (B >> 1) & 0xFF;
		FZ(B);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRLC() {
		int co = C & 1 ? 0x10 : 0;
		C = (C >> 1) & 0xFF;
		FZ(C);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRLD() {
		int co = D & 1 ? 0x10 : 0;
		D = (D >> 1) & 0xFF;
		FZ(D);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRLE() {
		int co = E & 1 ? 0x10 : 0;
		E = (E >> 1) & 0xFF;
		FZ(E);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRLH() {
		int co = H & 1 ? 0x10 : 0;
		H = (H >> 1) & 0xFF;
		FZ(H);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRLL() {
		int co = L & 1 ? 0x10 : 0;
		L = (L >> 1) & 0xFF;
		FZ(L);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::SRLA() {
		int co = A & 1 ? 0x10 : 0;
		A = (A >> 1) & 0xFF;
		FZ(A);
		F = (F & 0xEF) + co;
		mTemp = 2; tTemp = 8;
	}

	void CPU::BIT0B() {
		bit_ch(B, 0);
	}

	void CPU::BIT0C() {
		bit_ch(C, 0);
	}

	void CPU::BIT0D() {
		bit_ch(D, 0);
	}

	void CPU::BIT0E() {
		bit_ch(E, 0);
	}

	void CPU::BIT0H() {
		bit_ch(H, 0);
	}

	void CPU::BIT0L() {
		bit_ch(L, 0);
	}

	void CPU::BIT0A() {
		bit_ch(A, 0);
	}

	void CPU::BIT0M() {
		bit_ch(bus_->Read((H << 8) | L), 0);
		mTemp = 3; tTemp = 12;
	}

	void CPU::BIT1B() {
		bit_ch(B, 1);
	}

	void CPU::BIT1C() {
		bit_ch(C, 1);
	}

	void CPU::BIT1D() {
		bit_ch(D, 1);
	}

	void CPU::BIT1E() {
		bit_ch(E, 1);
	}

	void CPU::BIT1H() {
		bit_ch(H, 1);
	}

	void CPU::BIT1L() {
		bit_ch(L, 1);
	}

	void CPU::BIT1A() {
		bit_ch(A, 1);
	}

	void CPU::BIT1M() {
		bit_ch(bus_->Read((H << 8) | L), 1);
		mTemp = 3; tTemp = 12;
	}

	void CPU::BIT2B() {
		bit_ch(B, 2);
	}

	void CPU::BIT2C() {
		bit_ch(C, 2);
	}

	void CPU::BIT2D() {
		bit_ch(D, 2);
	}

	void CPU::BIT2E() {
		bit_ch(E, 2);
	}

	void CPU::BIT2H() {
		bit_ch(H, 2);
	}

	void CPU::BIT2L() {
		bit_ch(L, 2);
	}

	void CPU::BIT2A() {
		bit_ch(A, 2);
	}

	void CPU::BIT2M() {
		bit_ch(bus_->Read((H << 8) | L), 2);
		mTemp = 3; tTemp = 12;
	}

	void CPU::BIT3B() {
		bit_ch(B, 3);
	}

	void CPU::BIT3C() {
		bit_ch(C, 3);
	}

	void CPU::BIT3D() {
		bit_ch(D, 3);
	}

	void CPU::BIT3E() {
		bit_ch(E, 3);
	}

	void CPU::BIT3H() {
		bit_ch(H, 3);
	}

	void CPU::BIT3L() {
		bit_ch(L, 3);
	}

	void CPU::BIT3A() {
		bit_ch(A, 3);
	}

	void CPU::BIT3M() {
		bit_ch(bus_->Read((H << 8) | L), 3);
		mTemp = 3; tTemp = 12;
	}

	void CPU::BIT4B() {
		bit_ch(B, 4);
	}

	void CPU::BIT4C() {
		bit_ch(C, 4);
	}

	void CPU::BIT4D() {
		bit_ch(D, 4);
	}

	void CPU::BIT4E() {
		bit_ch(E, 4);
	}

	void CPU::BIT4H() {
		bit_ch(H, 4);
	}

	void CPU::BIT4L() {
		bit_ch(L, 4);
	}

	void CPU::BIT4A() {
		bit_ch(A, 4);
	}

	void CPU::BIT4M() {
		bit_ch(bus_->Read((H << 8) | L), 4);
		mTemp = 3; tTemp = 12;
	}

	void CPU::BIT5B() {
		bit_ch(B, 5);
	}

	void CPU::BIT5C() {
		bit_ch(C, 5);
	}

	void CPU::BIT5D() {
		bit_ch(D, 5);
	}

	void CPU::BIT5E() {
		bit_ch(E, 5);
	}

	void CPU::BIT5H() {
		bit_ch(H, 5);
	}

	void CPU::BIT5L() {
		bit_ch(L, 5);
	}

	void CPU::BIT5A() {
		bit_ch(A, 5);
	}

	void CPU::BIT5M() {
		bit_ch(bus_->Read((H << 8) | L), 5);
		mTemp = 3; tTemp = 12;
	}

	void CPU::BIT6B() {
		bit_ch(B, 6);
	}

	void CPU::BIT6C() {
		bit_ch(C, 6);
	}

	void CPU::BIT6D() {
		bit_ch(D, 6);
	}

	void CPU::BIT6E() {
		bit_ch(E, 6);
	}

	void CPU::BIT6H() {
		bit_ch(H, 6);
	}

	void CPU::BIT6L() {
		bit_ch(L, 6);
	}

	void CPU::BIT6A() {
		bit_ch(A, 6);
	}

	void CPU::BIT6M() {
		bit_ch(bus_->Read((H << 8) | L), 6);
		mTemp = 3; tTemp = 12;
	}

	void CPU::BIT7B() {
		bit_ch(B, 7);
	}

	void CPU::BIT7C() {
		bit_ch(C, 7);
	}

	void CPU::BIT7D() {
		bit_ch(D, 7);
	}

	void CPU::BIT7E() {
		bit_ch(E, 7);
	}

	void CPU::BIT7H() {
		bit_ch(H, 7);
	}

	void CPU::BIT7L() {
		bit_ch(L, 7);
	}

	void CPU::BIT7A() {
		bit_ch(A, 7);
	}

	void CPU::BIT7M() {
		bit_ch(bus_->Read((H << 8) | L), 7);
		mTemp = 3; tTemp = 12;
	}

	void CPU::RES0A() {
		A &= 0xFE;
		mTemp = 2;
		tTemp = 8;
	}

	#pragma endregion

	void CPU::Reset() {
		bus_->inBios = false;
		A = 0x1;
		B = 0x0;
		C = 0x13;
		D = 0x0;
		E = 0xD8;
		F = 0x90;
		H = 0x1;
		L = 0x4D;
		SP = 0xFFFE;
		PC = 0x100;
		IME = 1;
		bus_->SetIF(0xE1);
		bus_->SetIE(0);
		// TODO: Implement ppu reset
		//gpu->Reset();
		mClock = 0; tClock = 0; totalClock = 0;
		halt = false; stop = false;
	}

	int CPU::Update() {
		if (halt) {
			mTemp = 1; tTemp = 4;
		}
		else {
	#ifdef DEBUG_LOG_FILE
			debugLog << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << PC << ":" << std::nouppercase << " A:" << std::setfill('0') << std::setw(2) << A << " B:" << std::setfill('0') << std::setw(2) << B << " C:" << std::setfill('0') << std::setw(2) << C << " D:" << std::setfill('0') << std::setw(2) << D << " E:" << std::setfill('0') << std::setw(2) << E << " F:" << std::setfill('0') << std::setw(2) << F << " H:" << std::setfill('0') << std::setw(2) << H << " L:" << std::setfill('0') << std::setw(2) << L << " SP:" << std::setfill('0') << std::setw(4) << SP << "\r\n";
	#endif
			(this->*instructions[bus_->Read(PC++)].op)();
			totalClock++;
			F &= 0xF0;
			PC &= 0xFFFF;
		}
		mClock += mTemp;
		tClock += tTemp;
		return tTemp;
	}
	void CPU::CheckInterr() {
		int ieT, ifT;
		ieT = bus_->GetIE();
		ifT = bus_->GetIF();
		if (IME && ieT && ifT) {
			halt = 0; IME = 0;
			bool ifired = ieT & ifT;
			if (ifired & 1) {
				ifT &= 0xFE;
				bus_->SetIF(ifT);
				RST40();
			}
			else if (ifired & 2) {
				ifT &= 0xFD;
				bus_->SetIF(ifT);
				RST48();
			}
			else if (ifired & 4) {
				ifT &= 0xFB;
				bus_->SetIF(ifT);
				RST50();
			}
			else if (ifired & 8) {
				ifT &= 0xF7;
				bus_->SetIF(ifT);
				RST58();
			}
			else if (ifired & 16) {
				ifT &= 0xEF;
				bus_->SetIF(ifT);
				RST60();
			}
			else {
				IME = 1;
			}
		}
	}
}