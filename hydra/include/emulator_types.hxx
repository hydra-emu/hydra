#pragma once
#ifndef TKP_EMUTYPES_H
#define TKP_EMUTYPES_H
namespace TKPEmu {
    enum class EmuType {
        Gameboy,
        NES,
        N64,
        Chip8,

        EmuTypeSize, // Used to iterate all emutypes like so: for (int i = 0; i < EmuTypeSize; i++)
        Error
    };
}
constexpr static int EmuTypeSize = static_cast<int>(TKPEmu::EmuType::EmuTypeSize);
#endif