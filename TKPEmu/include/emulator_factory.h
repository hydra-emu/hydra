#ifndef TKPEMU_EMUFACTORY_H
#define TKPEMU_EMUFACTORY_H
#include <type_traits>
#include "emulator_includes.h"
namespace TKPEmu {
    class EmulatorFactory {
        public:
        template<class EmuType, class... Args>
        static std::unique_ptr<Emulator> Create(Args&... args) { 
            static_assert(std::is_base_of<Emulator, EmuType>::value, 
                "Bad type passed to EmulatorFactory, is not derived from Emulator class");
            return std::make_unique<EmuType>(args...);
        }
    };
}
#endif