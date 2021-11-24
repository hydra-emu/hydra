#include "emulator_factory.h"
#include "../gb_tkp/gameboy.h";
namespace TKPEmu {
    template<>
    std::unique_ptr<Emulator> EmulatorFactory::Create<TKPEmu::Gameboy::Gameboy>() {
        return std::make_unique<TKPEmu::Gameboy::Gameboy>(args...);
    }
}