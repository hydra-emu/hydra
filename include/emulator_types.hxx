#pragma once

#include <string>

#define EMULATORS \
    X(Gameboy)    \
    X(NES)        \
    X(N64)        \
    X(c8)

namespace hydra
{
    enum class EmuType
    {
#define X(name) name,
        EMULATORS
#undef X

            EmuTypeSize, // Used to iterate all emutypes like so: for (int i = 0; i < EmuTypeSize;
                         // i++)
    };

    inline std::string serialize_emu_type(EmuType type)
    {
        switch (type)
        {
#define X(name)         \
    case EmuType::name: \
        return #name;
            EMULATORS
#undef X
            default:
                return "Unknown";
        }
    }
} // namespace hydra

constexpr static int EmuTypeSize = static_cast<int>(hydra::EmuType::EmuTypeSize);
