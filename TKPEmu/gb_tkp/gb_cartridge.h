#pragma once
#ifndef TKP_GB_CARTRIDGE_H
#define TKP_GB_CARTRIDGE_H
// TODO: fix crashes when loading bad roms (utils)
#include <string>
#include <vector>
#include <array>
#define ENTRY_POINT 0x100
namespace TKPEmu::Gameboy::Devices {
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
		MBC3_TIMER_RAM_BATTERY = 0x10,
		MBC3 = 0x11,
		MBC3_RAM = 0x12,
		MBC3_RAM_BATTERY = 0x13,
		MBC4 = 0x15,
		MBC4_RAM = 0x16,
		MBC4_RAM_BATTERY = 0x17,
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
	class Cartridge {
	private:
		struct Header {
			char unusedData1[0x34];
			char name[14];
			char gameboyColor;
			char unusedData2[4];
			char cartridgeType;
			char romSize;
			char ramSize;
			char unusedData3[6];
		} header_;
		static constexpr std::array<int, 6> ram_sizes_ { 0, 0, 1, 4, 16, 8 };
		bool loaded;
	public:
		void Load(const std::string& fileName, std::vector<std::array<uint8_t, 0x4000>>& romBanks, std::vector<std::array<uint8_t, 0x2000>>& ramBanks);
		CartridgeType GetCartridgeType() const;
		int GetRamSize() const;
		int GetRomSize() const;
		bool IsGameboyColor() const;
		const char* GetCartridgeTypeName() const;
		const char* GetCartridgeName() const;
	};
}
#endif
