#pragma once

// TODO: fix crashes when loading bad roms (utils)
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#define ENTRY_POINT 0x100

namespace hydra::Gameboy
{
    class Bus;
    enum class CartridgeType {
        ERROR = 0x4,
        ROM_ONLY = 0x0,
        MBC1 = 0x1,
        MBC1_RAM = 0x2,
        MBC1_RAM_BATTERY = 0x3,
        MBC2 = 0x5,
        MBC2_BATTERY = 0x6,
        ROM_RAM = 0x8,
        ROM_RAM_BATTERY = 0x9,
        MMM01 = 0xB,
        MMM01_RAM = 0xC,
        MMM01_RAM_BATTERY = 0xD,
        MBC3_TIMER_BATTERY = 0xF,
        MBC3_TIMER_RAM_BATTERY = 0x10,
        MBC3 = 0x11,
        MBC3_RAM = 0x12,
        MBC3_RAM_BATTERY = 0x13,
        MBC5 = 0x19,
        MBC5_RAM = 0x1A,
        MBC5_RAM_BATTERY = 0x1B,
        MBC5_RUMBLE = 0x1C,
        MBC5_RUMBLE_RAM = 0x1D,
        MBC5_RUMBLE_RAM_BATTERY = 0x1E,
        MBC6_RAM_BATTERY = 0x20,
        MBC7_RAM_BATTERY_ACCELEROMETER = 0x22,
        POCKET_CAMERA = 0xFC,
        BANDAITAMA5 = 0xFD,
        HuC3 = 0xFE,
        HuC1_RAM_BATTERY = 0xFF
    };

    struct Header
    {
        char unusedData[0x34];
        char name[0xB];
        char manufacturer[0x4];
        char gameboyColor;
        char newLicenseeCode[0x2];
        char sgbFlag;
        char cartridgeType;
        char romSize;
        char ramSize;
        char destinationCode;
        char oldLicenseeCode;
        char romVersionNumber;
        char headerChecksum;
        char globalChecksum[2];
    };

    class Cartridge
    {
    private:
        Header header_{};
        int k = sizeof(header_);
        static constexpr std::array<int, 6> ram_sizes_{0, 0, 1, 4, 16, 8};
        bool text_cached_ = false;
        bool using_battery_ = false;

    public:
        bool Load(const std::string& filename, std::vector<std::array<uint8_t, 0x4000>>& romBanks,
                  std::vector<std::array<uint8_t, 0x2000>>& ramBanks);
        CartridgeType GetCartridgeType();
        int GetRamSize();
        int GetRomSize();
        bool UsingBattery();
        std::string GetCartridgeTypeName();
        std::string GetHeaderText();
        std::string GetLicenseeNew();
        std::string GetLicenseeOld();
        bool UseCGB = false;
        friend class Bus;
    };
} // namespace hydra::Gameboy
