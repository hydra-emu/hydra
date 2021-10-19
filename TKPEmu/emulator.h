#ifndef TKP_EMULATOR_H
#define TKP_EMULATOR_H
#include "Tools/TKPImage.h"
#include <mutex>
namespace TKPEmu {
	using TKPImage = TKPEmu::Tools::TKPImage;
	class Emulator {
	public:
		Emulator(TKPImage* image_ptr);
		virtual ~Emulator() = default;
		Emulator(const Emulator&) = delete;
		Emulator& operator=(const Emulator&) = delete;
		virtual void Reset() = 0;
		virtual void Update() = 0;
		std::mutex ScreenDataMutex;
		TKPImage* EmulatorImage = nullptr;
	};
}
#endif