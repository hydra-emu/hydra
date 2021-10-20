#include <fstream>
#include <iostream>
#include "cartridge.h"
namespace TKPEmu::Gameboy::Devices {
	void Cartridge::Load(const std::string& fileName, std::array<uint8_t, 0x10000>& rom) {
		std::ifstream is;
		is.open(fileName, std::ios::binary);
		if (is.is_open()) {
			is.seekg(ENTRY_POINT, std::ios_base::beg);
			is.read((char*)&header, sizeof(Header));
			is.seekg(0, std::ios_base::beg);
			if (GetCartridgeType() == CartridgeType::ROM_ONLY) {
				is.read((char*)&rom, sizeof(uint8_t) * 0x8000);
			}
			else {
				throw "This cartridge type is not implemented yet.";
			}
			is.close();
			loaded = true;
		}
	}

	Cartridge::CartridgeType Cartridge::GetCartridgeType() {
		if (loaded) {
			return (CartridgeType)header.cartridgeType;
		}
	}

	Cartridge::RomSize Cartridge::GetRomSize() {
		if (loaded) {
			return (RomSize)header.romSize;
		}
	}

	Cartridge::RamSize Cartridge::GetRamSize() {
		if (loaded) {
			return (RamSize)header.ramSize;
		}
	}
}