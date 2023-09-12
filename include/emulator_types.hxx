#pragma once

namespace hydra
{
    enum class EmuType
    {
        Gameboy,
        NES,
        N64,
        c8,

        EmuTypeSize, // Used to iterate all emutypes like so: for (int i = 0; i < EmuTypeSize; i++)
        Error
    };
}

constexpr static int EmuTypeSize = static_cast<int>(hydra::EmuType::EmuTypeSize);
