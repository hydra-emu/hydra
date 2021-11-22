#include <fstream>
#include <iostream>
#include <cmath>
#include "../include/gb_cartridge.h"
namespace TKPEmu::Gameboy::Devices {
	void Cartridge::Load(const std::string& fileName, std::vector<std::array<uint8_t, 0x4000>>& romBanks, std::vector<std::array<uint8_t, 0x2000>>& ramBanks) {
		std::ifstream is;
		is.open(fileName, std::ios::binary);
		if (is.is_open()) {
			is.seekg(ENTRY_POINT, std::ios_base::beg);
			is.read((char*)&header_, sizeof(Header));
			loaded = true;
			is.seekg(0, std::ios_base::beg);
			auto ct = GetCartridgeType();
			switch (ct) {
				case CartridgeType::ROM_ONLY:
				case CartridgeType::MBC1:
				case CartridgeType::MBC1_RAM:
				case CartridgeType::MBC1_RAM_BATTERY: {
					auto sz = GetRomSize();
					romBanks.resize(sz);
					for (int i = 0; i < sz; i++) {
						is.read((char*)(&romBanks[i]), sizeof(uint8_t) * 0x4000);
					}
					break;
				}
				default: {
					// TODO: better error or implement all cartridge types
					throw("error rom not implemented");
				}
			}
			is.close();
			// Empty init the rambanks
			ramBanks.resize(GetRamSize());
			PrintHeader();
		}
	}

	Cartridge::CartridgeType Cartridge::GetCartridgeType() {
		if (loaded) {
			return (CartridgeType)header_.cartridgeType;
		}
	}

	// Returns the number of 8KB RAM banks
	int Cartridge::GetRamSize() {
		switch (header_.ramSize) {
			case 0: return 0;
			case 2: return 1;
			case 3: return 4;
			case 4: return 16;
			case 5: return 8;
		}
	}

	// Returns the number of 16kb ROM banks
	int Cartridge::GetRomSize() {
		switch (header_.romSize) {
		// Likely inaccurate according to pandocs, no roms using these are known
		[[unlikely]] case 0x52: return 72;
		[[unlikely]] case 0x53: return 80;
		[[unlikely]] case 0x54: return 96;
		[[likely]] default:
			return std::pow(2, (header_.romSize + 1));
		}
	}
	void Cartridge::PrintHeader() {
		std::cout << "Header: {"
				<< "\nname: " << header_.name
				<< "\ncartridge_type:" << (int)header_.cartridgeType
				<< "\nrom_size: " << (int)header_.romSize
				<< "\nram_size: " << (int)header_.ramSize
				<< "\n}" << std::endl;
	}
}
