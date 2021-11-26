#ifndef TKPEMU_EMUFACTORY_H
#define TKPEMU_EMUFACTORY_H
#include <type_traits>
#include "emulator.h"
#include "base_application.h"
namespace TKPEmu {
    enum class EmuType {
        Gameboy,
    };
    class EmulatorFactory {
    private:
        using IMApplication = TKPEmu::Applications::IMApplication;
    public:
        template<class Type, class... Args>
        static std::unique_ptr<Emulator> Create(Args&... args) { 
            static_assert(std::is_base_of<Emulator, Type>::value, 
                "Bad type passed to EmulatorFactory, is not derived from Emulator class");
            return std::make_unique<Type>(args...);
        }
        static void LoadEmulatorTools(std::vector<std::unique_ptr<IMApplication>>& tools, EmuType emu_type);
    };
}
#endif