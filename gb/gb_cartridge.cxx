#include <cmath>
#include <fstream>
#include <gb/gb_cartridge.hxx>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace hydra::Gameboy
{
    bool Cartridge::Load(const std::string& filename,
                         std::vector<std::array<uint8_t, 0x4000>>& romBanks,
                         std::vector<std::array<uint8_t, 0x2000>>& ramBanks)
    {
        romBanks.clear();
        ramBanks.clear();
        std::ifstream is;
        is.open(filename, std::ios::binary);
        if (is.is_open())
        {
            text_cached_ = false;
            is.seekg(ENTRY_POINT, std::ios_base::beg);
            is.read(reinterpret_cast<char*>(&header_), sizeof(Header));
            if (header_.gameboyColor & 0x80)
            {
                // TODO: implement gbc
                if (header_.gameboyColor & 0x40)
                {
                    UseCGB = true;
                } else
                {
                    // check if force gbc
                    UseCGB = true;
                }
            }
            is.seekg(0, std::ios_base::beg);
            auto ct = GetCartridgeType();
            switch (ct)
            {
                case CartridgeType::ROM_RAM_BATTERY:
                case CartridgeType::MBC1_RAM_BATTERY:
                case CartridgeType::MBC2_BATTERY:
                case CartridgeType::MBC3_RAM_BATTERY:
                case CartridgeType::MBC3_TIMER_BATTERY:
                case CartridgeType::MBC3_TIMER_RAM_BATTERY:
                case CartridgeType::MBC5_RAM_BATTERY:
                case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
                case CartridgeType::MBC6_RAM_BATTERY:
                case CartridgeType::MBC7_RAM_BATTERY_ACCELEROMETER:
                case CartridgeType::MMM01_RAM_BATTERY:
                case CartridgeType::HuC1_RAM_BATTERY:
                {
                    using_battery_ = true;
                    break;
                }
            }
            switch (ct)
            {
                case CartridgeType::MBC2:
                case CartridgeType::MBC2_BATTERY:
                {
                    // MBC2 always has 4 ram banks
                    header_.ramSize = 4;
                    [[fallthrough]];
                }
                case CartridgeType::ROM_ONLY:
                case CartridgeType::MBC1:
                case CartridgeType::MBC1_RAM:
                case CartridgeType::MBC1_RAM_BATTERY:
                case CartridgeType::MBC3:
                case CartridgeType::MBC3_RAM:
                case CartridgeType::MBC3_RAM_BATTERY:
                case CartridgeType::MBC3_TIMER_BATTERY:
                case CartridgeType::MBC3_TIMER_RAM_BATTERY:
                case CartridgeType::MBC5:
                case CartridgeType::MBC5_RAM:
                case CartridgeType::MBC5_RAM_BATTERY:
                case CartridgeType::MBC5_RUMBLE:
                case CartridgeType::MBC5_RUMBLE_RAM:
                case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
                {
                    auto sz = GetRomSize();
                    romBanks.resize(sz);
                    for (int i = 0; i < sz; i++)
                    {
                        is.read(reinterpret_cast<char*>(&romBanks[i]), sizeof(uint8_t) * 0x4000);
                    }
                    break;
                }
                default:
                {
                    std::cerr << "This cartridge type is not implemented yet - " << (int)ct
                              << std::endl;
                    return false;
                }
            }
            is.close();
            // Empty init the rambanks
            ramBanks.resize(GetRamSize());
        } else
        {
            std::cerr << "Error: Could not open file" << std::endl;
        }
        return true;
    }

    bool Cartridge::UsingBattery() { return using_battery_; }

    CartridgeType Cartridge::GetCartridgeType()
    {
        return static_cast<CartridgeType>(header_.cartridgeType);
    }

    int Cartridge::GetRamSize()
    {
        // Returns the number of 8KB RAM banks
        return ram_sizes_[header_.ramSize];
    }

    int Cartridge::GetRomSize()
    {
        // Returns the number of 0x16kb ROM banks
        switch (header_.romSize)
        {
            // Likely inaccurate according to pandocs, no roms using these are known
            [[unlikely]] case 0x52:
                return 0x72;
            [[unlikely]] case 0x53:
                return 0x80;
            [[unlikely]] case 0x54:
                return 0x96;
            [[likely]] default:
                return std::pow(2, (header_.romSize + 1));
        }
    }

    const char* Cartridge::GetCartridgeTypeName()
    {
        auto ct = GetCartridgeType();
        switch (ct)
        {
            case CartridgeType::ROM_ONLY:
            {
                return "None (32Kb rom)";
            }
            case CartridgeType::MBC1:
            {
                return "MBC1";
            }
            case CartridgeType::MBC1_RAM:
            {
                return "MBC1 w/ RAM";
            }
            case CartridgeType::MBC1_RAM_BATTERY:
            {
                return "MBC1 w/ RAM, BATTERY";
            }
            case CartridgeType::MBC2:
            {
                return "MBC2";
            }
            case CartridgeType::MBC2_BATTERY:
            {
                return "MBC2 w/ BATTERY";
            }
            case CartridgeType::MBC3:
            {
                return "MBC3";
            }
            case CartridgeType::MBC3_RAM:
            {
                return "MBC3 w/ RAM";
            }
            case CartridgeType::MBC3_RAM_BATTERY:
            {
                return "MBC3 w/ RAM, BATTERY";
            }
            case CartridgeType::MBC3_TIMER_BATTERY:
            {
                return "MBC3 w/ TIMER, BATTERY";
            }
            case CartridgeType::MBC3_TIMER_RAM_BATTERY:
            {
                return "MBC3 w/ RAM, BATTERY, TIMER";
            }
            case CartridgeType::MBC5:
            {
                return "MBC5";
            }
            case CartridgeType::MBC5_RAM:
            {
                return "MBC5 w/ RAM";
            }
            case CartridgeType::MBC5_RAM_BATTERY:
            {
                return "MBC5 w/ RAM, BATTERY";
            }
            case CartridgeType::MBC5_RUMBLE:
            {
                return "MBC5 w/ RUMBLE";
            }
            case CartridgeType::MBC5_RUMBLE_RAM:
            {
                return "MBC5 w/ RAM, RUMBLE";
            }
            case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
            {
                return "MBC5 w/ RAM, BATTERY, RUMBLE";
            }
            default:
            {
                return std::to_string(static_cast<int>(ct)).c_str();
            }
        }
    }

    const char* Cartridge::GetHeaderText()
    {
        static std::string header_text;
        if (!text_cached_)
        {
            text_cached_ = true;
            bool cgb = false;
            std::string name = header_.name;
            std::string cgb_type = "No";
            if (header_.gameboyColor & 0x80)
            {
                cgb = true;
                if (header_.gameboyColor & 0x40)
                {
                    cgb_type = "GBC only";
                } else
                {
                    cgb_type = "Supports GBC";
                }
                // TODO: what is pgb mode? Values with Bit 7 set, and either Bit 2 or 3 set, will
                // switch the Game Boy into a special non-CGB-mode called “PGB mode”.
            }
            std::string licensee = GetLicenseeNew();
            std::string sgb = "No";
            if (header_.oldLicenseeCode != 0x33)
            {
                if (licensee == "Unknown")
                {
                    licensee = GetLicenseeOld();
                }
            } else
            {
                sgb = (header_.sgbFlag == 0x3) ? "Supports SGB" : "No";
            }
            std::string dest = header_.destinationCode ? "Non-Japanese" : "Japanese";
            uint8_t sum = 0;
            for (size_t i = 0x034; i <= 0x04C; i++)
            {
                sum = sum - (reinterpret_cast<char*>(&header_))[i] - 1;
            }
            std::string check = (sum == header_.headerChecksum) ? "Passed" : "Failed";
            header_text = "Name:";
            header_text += name.c_str();
            header_text += "\n";
            header_text += "Type:";
            header_text += GetCartridgeTypeName();
            header_text += "\n";
            header_text += "Licensee:" + licensee + "\n";
            header_text += "GBC:";
            header_text += cgb_type.c_str();
            header_text += "\n";
            header_text += "SGB:" + sgb + "\n";
            header_text += "ROM count:" + std::to_string(header_.romSize) + "\n";
            header_text += "RAM count:" + std::to_string(header_.ramSize) + "\n";
            header_text += "Destination:" + dest + "\n";
            header_text += "Header checksum:";
            std::stringstream ss;
            ss << "0x" << std::setfill('0') << std::setw(2) << std::hex
               << (short)header_.headerChecksum;
            header_text += ss.str();
            // std::format not yet supported by gcc c++20
            // header_text += std::format("{:x}", header_.headerChecksum);
            header_text += " (" + check + ")\n";
        }
        return header_text.c_str();
    }

    std::string Cartridge::GetLicenseeNew()
    {
        uint8_t lic = header_.newLicenseeCode[1];
        switch (lic)
        {
            case 0x00:
                return "Unknown";
            case 0x01:
                return "Nintendo R&D1";
            case 0x08:
                return "Capcom";
            case 0x13:
                return "Electronic Arts";
            case 0x18:
                return "Hudson Soft";
            case 0x19:
                return "b-ai";
            case 0x20:
                return "kss";
            case 0x22:
                return "pow";
            case 0x24:
                return "PCM Complete";
            case 0x25:
                return "san-x";
            case 0x28:
                return "Kemco Japan";
            case 0x29:
                return "seta";
            case 0x30:
                return "Viacom";
            case 0x31:
                return "Nintendo";
            case 0x32:
                return "Bandai";
            case 0x33:
                return "Ocean/Acclaim";
            case 0x34:
                return "Konami";
            case 0x35:
                return "Hector";
            case 0x37:
                return "Taito";
            case 0x38:
                return "Hudson";
            case 0x39:
                return "Banpresto";
            case 0x41:
                return "Ubi Soft";
            case 0x42:
                return "Atlus";
            case 0x44:
                return "Malibu";
            case 0x46:
                return "angel";
            case 0x47:
                return "Bullet-Proof";
            case 0x49:
                return "irem";
            case 0x50:
                return "Absolute";
            case 0x51:
                return "Acclaim";
            case 0x52:
                return "Activision";
            case 0x53:
                return "American sammy";
            case 0x54:
                return "Konami";
            case 0x55:
                return "Hi tech entertainment";
            case 0x56:
                return "LJN";
            case 0x57:
                return "Matchbox";
            case 0x58:
                return "Mattel";
            case 0x59:
                return "Milton Bradley";
            case 0x60:
                return "Titus";
            case 0x61:
                return "Virgin";
            case 0x64:
                return "LucasArts";
            case 0x67:
                return "Ocean";
            case 0x69:
                return "Electronic Arts";
            case 0x70:
                return "Infogrames";
            case 0x71:
                return "Interplay";
            case 0x72:
                return "Broderbund";
            case 0x73:
                return "sculptured";
            case 0x75:
                return "sci";
            case 0x78:
                return "THQ";
            case 0x79:
                return "Accolade";
            case 0x80:
                return "misawa";
            case 0x83:
                return "lozc";
            case 0x86:
                return "Tokuma Shoten Intermedia";
            case 0x87:
                return "Tsukuda Original";
            case 0x91:
                return "Chunsoft";
            case 0x92:
                return "Video system";
            case 0x93:
                return "Ocean/Acclaim";
            case 0x95:
                return "Varie";
            case 0x96:
                return "Yonezawa/s’pal";
            case 0x97:
                return "Kaneko";
            case 0x99:
                return "Pack in soft";
            case 0xA4:
                return "Konami (Yu-Gi-Oh!)";
            default:
                return "Unknown";
        }
    }

    std::string Cartridge::GetLicenseeOld()
    {
        uint8_t lic = header_.oldLicenseeCode;
        switch (lic)
        {
            case 0x00:
                return "None";
            case 0x01:
            case 0x31:
                return "Nintendo";
            case 0x08:
            case 0x38:
                return "Capcom";
            case 0x09:
                return "Hot-b";
            case 0x0A:
            case 0xE0:
                return "Jaleco";
            case 0x0B:
                return "Coconuts";
            case 0x0C:
            case 0x6E:
                return "Elite systems";
            case 0x13:
            case 0x69:
                return "Electronic arts";
            case 0x18:
                return "Hudsonsoft";
            case 0x19:
                return "Itc entertainment";
            case 0x1A:
                return "Yanoman";
            case 0x1D:
                return "Clary";
            case 0x1F:
            case 0x61:
            case 0x4A:
                return "Virgin";
            case 0x24:
                return "Pcm complete";
            case 0x25:
                return "San-x";
            case 0x28:
                return "Kotobuki systems";
            case 0x29:
                return "Seta";
            case 0x30:
            case 0x70:
                return "Infogrames";
            case 0x32:
            case 0xA2:
            case 0xB2:
                return "Bandai";
            case 0x34:
            case 0xA4:
                return "Konami";
            case 0x35:
                return "Hector";
            case 0x39:
            case 0x9D:
            case 0xD9:
                return "Banpresto";
            case 0x3C:
                return "Entertainment i (Unknown)";
            case 0x3E:
                return "Gremlin";
            case 0x41:
                return "Ubi soft";
            case 0x42:
            case 0xEB:
                return "Atlus";
            case 0x44:
            case 0x4D:
                return "Malibu";
            case 0x46:
            case 0xCF:
                return "Angel";
            case 0x47:
                return "Spectrum holoby";
            case 0x49:
                return "Irem";
            case 0x4F:
                return "U.s. gold";
            case 0x50:
                return "Absolute";
            case 0x51:
            case 0xB0:
                return "Acclaim";
            case 0x52:
                return "Activision";
            case 0x53:
                return "American sammy";
            case 0x54:
                return "Gametek";
            case 0x55:
                return "Park place";
            case 0x56:
            case 0xDB:
            case 0xFF:
                return "Ljn";
            case 0x57:
                return "Matchbox";
            case 0x59:
                return "Milton bradley";
            case 0x5A:
                return "Mindscape";
            case 0x5B:
                return "Romstar";
            case 0x5C:
            case 0xD6:
                return "Naxat soft";
            case 0x5D:
                return "Tradewest";
            case 0x60:
                return "Titus";
            case 0x67:
                return "Ocean";
            case 0x6F:
                return "Electro brain";
            case 0x71:
                return "Interplay";
            case 0x72:
            case 0xAA:
                return "Broderbund";
            case 0x73:
                return "Sculptered soft";
            case 0x75:
                return "The sales curve";
            case 0x78:
                return "T*hq";
            case 0x79:
                return "Accolade";
            case 0x7A:
                return "Triffix entertainment";
            case 0x7C:
                return "Microprose";
            case 0x7F:
            case 0xC2:
                return "Kemco";
            case 0x80:
                return "Misawa entertainment";
            case 0x83:
                return "Lozc";
            case 0x86:
            case 0xC4:
                return "Tokuma shoten";
            case 0x8B:
                return "Bullet-proof software";
            case 0x8C:
                return "Vic tokai";
            case 0x8E:
                return "Ape";
            case 0x8F:
                return "I'max";
            case 0x91:
                return "Chun soft";
            case 0x92:
                return "Video system";
            case 0x93:
                return "Tsuburava";
            case 0x95:
            case 0xE3:
                return "Varie";
            case 0x96:
                return "Yonezawa/s'pal";
            case 0x97:
                return "Kaneko";
            case 0x99:
                return "Arc";
            case 0x9A:
                return "Nihon bussan";
            case 0x9B:
                return "Tecmo";
            case 0x9C:
                return "Imagineer";
            case 0x9F:
                return "Nova";
            case 0xA1:
                return "Hori electric";
            case 0xA6:
                return "Kawada";
            case 0xA7:
                return "Takara";
            case 0xA9:
                return "Technos japan";
            case 0xAC:
                return "Toei animation";
            case 0xAD:
                return "Toho";
            case 0xAF:
                return "Namco";
            case 0xB1:
                return "Ascii or nexoft";
            case 0xB4:
                return "Enix";
            case 0xB6:
                return "Hal";
            case 0xB7:
                return "Snk";
            case 0xB9:
            case 0xCE:
                return "Pony canyon";
            case 0xBA:
                return "Culture brain (?)";
            case 0xBB:
                return "Sunsoft";
            case 0xBD:
                return "Sony imagesoft";
            case 0xBF:
                return "Sammy";
            case 0xC0:
            case 0xD0:
                return "Taito";
            case 0xC3:
                return "Squaresoft";
            case 0xC5:
                return "Data east";
            case 0xC6:
                return "Tonkin house";
            case 0xC8:
                return "Koei";
            case 0xC9:
                return "Ufl";
            case 0xCA:
                return "Ultra";
            case 0xCB:
                return "Vap";
            case 0xCC:
                return "Use";
            case 0xCD:
                return "Meldac";
            case 0xD1:
                return "Sofel";
            case 0xD2:
                return "Quest";
            case 0xD3:
                return "Sigma enterprises";
            case 0xD4:
                return "Ask kodansha";
            case 0xD7:
                return "Copya systems";
            case 0xDA:
                return "Tomy";
            case 0xDD:
                return "Ncs";
            case 0xDE:
                return "Human";
            case 0xDF:
                return "Altron";
            case 0xE1:
                return "Towachiki";
            case 0xE2:
                return "Uutaka";
            case 0xE5:
                return "Epoch";
            case 0xE7:
                return "Athena";
            case 0xE8:
                return "Asmik";
            case 0xE9:
                return "Natsume";
            case 0xEA:
                return "King records";
            case 0xEC:
                return "Epic/sony records";
            case 0xEE:
                return "Igs";
            case 0xF0:
                return "A wave";
            case 0xF3:
                return "Extreme entertainment";
            default:
                return "Unknown";
        }
    }
} // namespace hydra::Gameboy
