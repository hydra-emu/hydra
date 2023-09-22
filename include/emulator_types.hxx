#pragma once

#include <string>

#define EMULATORS   X(N64, "Nintendo 64")\
                    X(NDS, "Nintendo DS")

namespace hydra
{
    enum class EmuType
    {
#define X(name, _) name,
        EMULATORS
#undef X

            EmuTypeSize, // Used to iterate all emutypes like so: for (int i = 0; i < EmuTypeSize;
                         // i++)
    };

    inline std::string serialize_emu_type(EmuType type)
    {
        switch (type)
        {
#define X(name, str_name) \
    case EmuType::name:   \
        return #name;
            EMULATORS
#undef X
            default:
                return "Unknown";
        }
    }

    inline std::string get_emu_type_name(EmuType type)
    {
        switch (type)
        {
#define X(name, str_name) \
    case EmuType::name:   \
        return str_name;
            EMULATORS
#undef X
            default:
                return "Unknown";
        }
    }
} // namespace hydra

constexpr static int EmuTypeSize = static_cast<int>(hydra::EmuType::EmuTypeSize);
