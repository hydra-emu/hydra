#include <iostream>>
#include <iomanip>
#include "bus.h"
namespace TKPEmu::Gameboy::Devices {
	Bus::Bus() {
		inBios = true;
		std::fill(mem.begin(), mem.end(), 0);
	}

	uint8_t Bus::Read(uint16_t address) {
		switch (address & 0xF000) {
		[[unlikely]] case 0x0000: {
			if (inBios) {
				//if (address <= BIOS_SIZE) {
				//	return bios[address];
				//}
				//else {
					inBios = false;
				//}
			}
			return mem[address] & 0xFFFF;
		}
		case 0x1000:
		case 0x2000:
		case 0x3000: {
			return mem[address] & 0xFFFF;
		}
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000: {
			return mem[address] & 0xFFFF;
		}
		case 0x8000:
		case 0x9000: {
			return mem[address] & 0xFFFF;
		}
		case 0xA000:
		case 0xB000: {
			return mem[address] & 0xFFFF;
		}
		case 0xC000:
		case 0xD000: {
			return mem[address] & 0xFFFF;
		}
		case 0xE000: {
			return mem[address] & 0xFFFF;
		}
		[[likely]] case 0xF000: {
			switch (address & 0x0F00) {
			case 0x0:
			case 0x100:
			case 0x200:
			case 0x300:
			case 0x400:
			case 0x500:
			case 0x600:
			case 0x700:
			case 0x800:
			case 0x900:
			case 0xA00:
			case 0xB00:
			case 0xC00:
			case 0xD00: {
				return mem[address] & 0xFFFF;
			}

			case 0xE00: {
				return mem[address] & 0xFFFF;
			}

			case 0xF00:
			{
				if (address == 0xFF41) {
					return ((mem[0xFF44] == mem[0xFF45]) ? 4 : 0) | mem[0xFF41];
				}
				return mem[address] & 0xFFFF;
			}
			}
		}
		}

		return 0;
	}

	uint16_t Bus::ReadL(uint16_t address) {
		return Read(address) + (Read(address + 1) << 8);
	}

	void Bus::Write(uint16_t address, uint8_t data) {
		switch (address & 0xF000) {
		case 0x0000: {
			if (inBios && address < 0x0100) return;
		}
		case 0x8000:
		case 0x9000: {
			// VRAM
			backgroundMapChanged = true;
			mem[address] = data;
			break;
		}
		case 0xA000:
		case 0xB000:
		case 0xC000:
		case 0xD000:
		case 0xE000: {
			mem[address] = data;
			break;
		}
		[[likely]] case 0xF000: {
			switch (address & 0x0F00) {
			case 0x0:
			case 0x100:
			case 0x200:
			case 0x300:
			case 0x400:
			case 0x500:
			case 0x600:
			case 0x700:
			case 0x800:
			case 0x900:
			case 0xA00:
			case 0xB00:
			case 0xC00:
			case 0xD00: {
				mem[address] = data;
				break;
			}

			case 0xE00: {
				// OAM
				if (address <= OAM_END) {
					spriteDataChanged = true;
					int index = address - OAM_START;
					int i = (index / 4);
					int j = (index) % 4;
					switch (j) {
					case 0:
						sprites[i].yPosition = data;
						break;
					case 1:
						sprites[i].xPosition = data;
						break;
					case 2:
						sprites[i].tileNumber = data;
						break;
					case 3:
						sprites[i].spriteFlags = data;
						break;
					}
				}
				mem[address] = data;
				break;
			}

			case 0xF00:
			{
				switch (address) {
				case 0xFF00:
					break;
				case 0xFF01:
					// TODO: implement serial
					std::cout << data;
					break;
				case 0xFF04:
					// DIV
					mem[address] = 0;
					break;
				case 0xFF05:
				case 0xFF06:
					// Timer tima, tma
					mem[address] = data;
					break;
				case 0xFF07:
					mem[address] = data & 7;
					break;
				case 0xFF40:
					mem[address] = data;
					break;
				case 0xFF42:
				case 0xFF43:
					spriteDataChanged = true;
					mem[address] = data;
					break;
				case 0xFF46:
					// Update oam
					for (int i = 0; i < 160; i++) {
						int v = Read((data << 8) + i);
						Write(OAM_START + i, v);
					}
					break;
				case 0xFF47:
					// Set background palette
					backgroundPalette[0] = palette[data & 0x3];
					backgroundPalette[1] = palette[(data >> 2) & 0x3];
					backgroundPalette[2] = palette[(data >> 4) & 0x3];
					backgroundPalette[3] = palette[(data >> 6)];
					mem[address] = data;
					break;
				case 0xFF48:
					// Set sprite palette (OBJ0)
					spriteDataChanged = true;
					obj0Palette[0] = palette[(data >> 2) & 0x3];
					obj0Palette[1] = palette[(data >> 4) & 0x3];
					obj0Palette[2] = palette[(data >> 6)];
					mem[address] = data;
					break;
				case 0xFF49:
					// Set sprite palette (OBJ1)
					spriteDataChanged = true;
					obj1Palette[0] = palette[(data >> 2) & 0x3];
					obj1Palette[1] = palette[(data >> 4) & 0x3];
					obj1Palette[2] = palette[(data >> 6)];
					mem[address] = data;
					break;
				[[likely]] default :
					mem[address] = data;
					break;
				}
			}
			break;
			}
		}
		}
	}

	void Bus::WriteL(uint16_t address, uint16_t data) {
		Write(address, data & 0xFF);
		Write(address + 1, data >> 8);
	}

	void Bus::WriteInput(int key) {
		switch (key) {
		case 13:
			mem[0xFF00] &= 0xD7;
			break;
		}
	}

	void Bus::RemoveInput(int key) {
		switch (key) {
		case 13:
			mem[0xFF00] |= 0x8;
			break;
		}
	}

	void Bus::LoadCartridge(std::string fileName) {
		cartridge = std::unique_ptr<Cartridge>(new Cartridge());
		cartridge->Load(fileName, mem);
	}

	int Bus::GetIE() {
		return mem[0xFFFF];
	}

	int Bus::GetIF() {
		return mem[0xFF0F];
	}

	void Bus::SetIE(int value) {
		mem[0xFFFF] = value;
	}

	void Bus::SetIF(int value) {
		mem[0xFF0F] = value;
	}
}