#include <iostream>>
#include <iomanip>
#include "bus.h"
namespace TKPEmu::Gameboy::Devices {

	Bus::Bus() {
		inBios = true;
	}

	uint8_t& Bus::redirect_address(uint16_t address) {
		using CT = Cartridge::CartridgeType;
		// Return address from ROM banks
		// TODO: create better exceptions
		switch (address & 0xF000) {
			case 0x0000:
			case 0x1000:
			case 0x2000:
			case 0x3000:
			case 0x4000:
			case 0x5000:
			case 0x6000:
			case 0x7000: {
				auto ct = cartridge_->GetCartridgeType();
				switch (ct) {
					case CT::ROM_ONLY: {
						int index = address / 0x4000;
						return (rom_banks_[index])[address % 0x4000];
					}
					case CT::MBC1:
					case CT::MBC1_RAM:
					case CT::MBC1_RAM_BATTERY: {
						if (address <= 0x3FFF) {
							return (rom_banks_[0])[address % 0x4000];
						}
						else {
							return (rom_banks_[selected_rom_bank_])[address % 0x4000];
						}
						break;
					}
				}
				throw("Bad cartridge type");
				break;
			}
			case 0x8000:
			case 0x9000: {
				return vram_[address % 0x2000];
			}
			case 0xA000:
			case 0xB000: {
				if (cartridge_->GetRamSize() == 0)
					return eram_default_[address % 0x2000];

				return (ram_banks_[selected_ram_bank_])[address % 0x2000];
			}
			case 0xC000:
			case 0xD000: {
				return wram_[address % 0x2000];
			}
			case 0xE000: {
				return redirect_address(address - 0x2000);
			}
			case 0xF000: {
				if (address <= 0xFDFF) {
					return redirect_address(address - 0x2000);
				}
				else if (address <= 0xFE9F) {
					// OAM
					return oam_[address & 0x9F];
				}
				else if (address <= 0xFEFF) {
					// TODO: check if this is actually unused area
					return unused_mem_area_;
				}
				else if (address <= 0xFFFF) {
					// I/O Registers, HRAM and IE
					return hram_[address & 0xFF];
				}
				else {
					throw("Bad memory address");
				}
			}
		}
	}

	uint8_t Bus::Read(uint16_t address) {
		// Make copy so you can't write to this
		uint8_t read = redirect_address(address);
		return read;
	}

	uint16_t Bus::ReadL(uint16_t address) {
		return Read(address) + (Read(address + 1) << 8);
	}

	uint8_t& Bus::GetReference(uint16_t address) {
		return redirect_address(address);
	}

	void Bus::Write(uint16_t address, uint8_t data) {
		// TODO: implement serial correctly, remove this
		if (address == 0xFF01) {
			std::cout << data;
		}
		if (address <= 0x7FFF) {
			if (address <= 0x1FFF) {
				// Any value "written" here with lower 4 bits == 0xA enables eram,
				// other values disable eram
				if ((data & 0xF) == 0xA) {
					ram_enabled_ = true;
				}
				else {
					ram_enabled_ = false;
				}
			}
			else if (address <= 0x3FFF) {
				// Keep 3 highest bits
				selected_rom_bank_ &= 0b11100000;
				selected_rom_bank_ |= data & 0b11111;
				selected_rom_bank_ %= rom_banks_size_;
				if (selected_rom_bank_ == 0)
					selected_rom_bank_ = 1;
			}
			else if (address <= 0x5FFF) {
				selected_rom_bank_ &= 0b11111;
				selected_rom_bank_ |= (data << 5);
				selected_rom_bank_ %= rom_banks_size_;
			}
			else if (address <= 0x7FFF) {
				// TODO: MBC1 1MB multi-carts might have different behavior, investigate
				// This enables mbc1 1mb mode?
			}
		}
		else {
			switch (address) {
				case addr_bgp: {
					for (int i = 0; i < 4; i++) {
						BGPalette[i] = (data >> (i * 2)) & 0b11;
					}
					break;
				}
				case addr_obp0: {
					for (int i = 0; i < 4; i++) {
						OBJ0Palette[i] = (data >> (i * 2)) & 0b11;
					}
					break;
				}
				case addr_obp1: {
					for (int i = 0; i < 4; i++) {
						OBJ1Palette[i] = (data >> (i * 2)) & 0b11;
					}
					break;
				}
			}
			redirect_address(address) = data;
		}
	}

	void Bus::WriteL(uint16_t address, uint16_t data) {
		Write(address, data & 0xFF);
		Write(address + 1, data >> 8);
	}

	void Bus::Reset() {
		SoftReset();
		for (auto& rom : rom_banks_) {
			rom.fill(0);
		}
		
	}

	void Bus::SoftReset() {
		for (auto& ram : ram_banks_) {
			ram.fill(0);
		}
		hram_.fill(0);
		oam_.fill(0);
		selected_rom_bank_ = 1;
		selected_ram_bank_ = 0;
	}

	void Bus::LoadCartridge(std::string&& fileName) {
		Reset();
		cartridge_ = std::unique_ptr<Cartridge>(new Cartridge());
		cartridge_->Load(fileName, rom_banks_, ram_banks_);
		rom_banks_size_ = cartridge_->GetRomSize();
	}
}