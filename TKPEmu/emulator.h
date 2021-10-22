#ifndef TKP_EMULATOR_H
#define TKP_EMULATOR_H
#include "Tools/TKPImage.h"
#include "Tools/disassembly_instr.h"
#include <mutex>
#include <vector>
namespace TKPEmu {
	using TKPImage = TKPEmu::Tools::TKPImage;
	using DisInstr = TKPEmu::Tools::DisInstr;
	class Emulator {
	public:
		Emulator() = default;
		virtual ~Emulator() = default;
		Emulator(const Emulator&) = delete;
		Emulator& operator=(const Emulator&) = delete;
		virtual void Reset() = 0;
		virtual void Update() = 0;
		virtual void LoadFromFile(const std::string& path) = 0;
		virtual void LoadInstrToVec(std::vector<DisInstr>& vec, bool& finished) = 0;
		std::mutex ScreenDataMutex;
		TKPImage* EmulatorImage = nullptr;
	};
}
#endif