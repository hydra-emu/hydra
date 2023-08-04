#include <algorithm>
#include <bitset>
#include <cstring>
#include <filesystem>
#include <gb/gb_addresses.hxx>
#include <gb/gb_bus.hxx>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace hydra::Gameboy
{
    using RamBank = std::array<uint8_t, 0x2000>;

    Bus::Bus(ChannelArrayPtr channel_array_ptr) : channel_array_ptr_(channel_array_ptr)
    {
        if (channel_array_ptr)
        {
            (*channel_array_ptr_)[2].LengthInit = 256;
        }
    }

    Bus::~Bus()
    {
        battery_save();
    }

    void Bus::handle_mbc(uint16_t address, uint8_t data)
    {
        auto type = cartridge_.GetCartridgeType();
        switch (type)
        {
            case CartridgeType::MBC1:
            case CartridgeType::MBC1_RAM:
            case CartridgeType::MBC1_RAM_BATTERY:
            {
                if (address <= 0x1FFF)
                {
                    if ((data & 0b1111) == 0b1010)
                    {
                        ram_enabled_ = true;
                    }
                    else
                    {
                        ram_enabled_ = false;
                    }
                }
                else if (address <= 0x3FFF)
                {
                    // BANK register 1 (TODO: this doesnt happen on mbc0?)
                    selected_rom_bank_ &= 0b1100000;
                    selected_rom_bank_ |= data & 0b11111;
                    selected_rom_bank_ %= rom_banks_size_;
                    refill_fast_map_rom();
                }
                else if (address <= 0x5FFF)
                {
                    // BANK register 2
                    selected_rom_bank_ &= 0b11111;
                    selected_rom_bank_ |= ((data & 0b11) << 5);
                    selected_rom_bank_ %= rom_banks_size_;
                    selected_ram_bank_ = data & 0b11;
                    refill_fast_map_rom();
                }
                else
                {
                    // MODE register
                    banking_mode_ = data & 0b1;
                    refill_fast_map_rom();
                }
                break;
            }
            case CartridgeType::MBC2:
            case CartridgeType::MBC2_BATTERY:
            {
                if (address <= 0x3FFF)
                {
                    // Different behavior when bit 8 of address is set
                    bool ram_wr = (address >> 8) & 0b1;
                    if (ram_wr)
                    {
                        selected_rom_bank_ = data;
                        refill_fast_map_rom();
                    }
                    else
                    {
                        if ((data & 0b1111) == 0b1010)
                        {
                            ram_enabled_ = true;
                        }
                        else
                        {
                            ram_enabled_ = false;
                        }
                    }
                }
                break;
            }
            case CartridgeType::MBC3:
            case CartridgeType::MBC3_RAM:
            case CartridgeType::MBC3_RAM_BATTERY:
            case CartridgeType::MBC3_TIMER_BATTERY:
            case CartridgeType::MBC3_TIMER_RAM_BATTERY:
            {
                if (address <= 0x1FFF)
                {
                    if ((data & 0b1111) == 0b1010)
                    {
                        ram_enabled_ = true;
                        // TODO: enable writing to RTC mbc3 registers
                    }
                    else
                    {
                        ram_enabled_ = false;
                    }
                }
                else if (address <= 0x3FFF)
                {
                    selected_rom_bank_ = data & 0b0111'1111;
                    if (selected_rom_bank_ == 0)
                    {
                        selected_rom_bank_ = 1;
                    }
                    refill_fast_map_rom();
                }
                else if (address <= 0x5FFF)
                {
                    if (data <= 0b11)
                    {
                        selected_ram_bank_ = data;
                    }
                    else
                    {
                        // TODO: mbc3 rtc
                    }
                }
                else
                {
                    // MODE register
                    banking_mode_ = data & 0b1;
                    refill_fast_map_rom();
                }
                break;
            }
            case CartridgeType::MBC5:
            case CartridgeType::MBC5_RAM:
            case CartridgeType::MBC5_RAM_BATTERY:
            case CartridgeType::MBC5_RUMBLE:
            case CartridgeType::MBC5_RUMBLE_RAM:
            case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
            {
                if (address <= 0x1FFF)
                {
                    if ((data & 0b1111) == 0b1010)
                    {
                        ram_enabled_ = true;
                    }
                    else
                    {
                        ram_enabled_ = false;
                    }
                }
                else if (address <= 0x2FFF)
                {
                    selected_rom_bank_ = data;
                    refill_fast_map_rom();
                }
                else if (address <= 0x3FFF)
                {
                    selected_rom_bank_high_ = data & 0b1;
                    refill_fast_map_rom();
                }
                else if (address <= 0x5FFF)
                {
                    if (data <= 0xF)
                    {
                        selected_ram_bank_ = data;
                    }
                }
                break;
            }
            default:
            {
                return;
            }
        }
    }

    void Bus::fill_fast_map()
    {
        for (int i = 0x0; i < 0x40; i++)
        {
            auto address = (i << 8) & 0x3FFF;
            fast_map_[i] = &((rom_banks_[0])[address]);
        }
        for (int i = 0x40; i < 0x80; i++)
        {
            auto address = (i << 8) & 0x3FFF;
            fast_map_[i] = &((rom_banks_[1])[address]);
        }
        for (int i = 0x80; i < 0xA0; i++)
        {
            auto address = (i << 8) & 0x1FFF;
            fast_map_[i] = &((vram_banks_[vram_sel_bank_])[address]);
        }
        for (int i = 0xC0; i < 0xD0; i++)
        {
            auto address = (i << 8) & 0xFFF;
            fast_map_[i] = &(wram_banks_[0][address]);
        }
        for (int i = 0xD0; i < 0xE0; i++)
        {
            auto address = (i << 8) & 0xFFF;
            fast_map_[i] = &(wram_banks_[1][address]);
        }
        for (int i = 0xE0; i < 0xF0; i++)
        {
            auto address = (i << 8) & 0xFFF;
            fast_map_[i] = &(wram_banks_[0][address]);
        }
        if (BiosEnabled)
        {
            // use slow redirecting
            fast_map_[0x00] = nullptr;
            for (int i = 0x02; i < 0x09; i += 0x1)
            {
                fast_map_[i] = nullptr;
            }
        }
    }

    void Bus::refill_fast_map_rom()
    {
        auto ct = cartridge_.GetCartridgeType();
        switch (ct)
        {
            case CartridgeType::MBC1:
            case CartridgeType::MBC1_RAM:
            case CartridgeType::MBC1_RAM_BATTERY:
            {
                auto sel =
                    (banking_mode_ ? selected_rom_bank_ & 0b1100000 : 0) % cartridge_.GetRomSize();
                for (int i = 0x0; i < 0x40; i++)
                {
                    auto address = (i << 8) % 0x4000;
                    fast_map_[i] = &((rom_banks_[sel])[address]);
                }
                auto sel_h = selected_rom_bank_ % cartridge_.GetRomSize();
                if ((sel_h & 0b11111) == 0)
                {
                    // In 4000-7FFF, automatically maps to next addr if addr chosen is 00/20/40/60
                    // TODO: fix multicart roms
                    sel_h += 1;
                }
                for (int i = 0x40; i < 0x80; i++)
                {
                    auto address = (i << 8) % 0x4000;
                    fast_map_[i] = &((rom_banks_[sel_h])[address]);
                }
                break;
            }
            case CartridgeType::MBC2:
            case CartridgeType::MBC2_BATTERY:
            {
                if ((selected_rom_bank_ & 0b1111) == 0)
                {
                    selected_rom_bank_ |= 0b1;
                }
                auto sel = selected_rom_bank_ % cartridge_.GetRomSize();
                for (int i = 0x40; i < 0x80; i++)
                {
                    auto address = (i << 8) % 0x4000;
                    fast_map_[i] = &((rom_banks_[sel])[address]);
                }
                break;
            }
            case CartridgeType::MBC3:
            case CartridgeType::MBC3_RAM:
            case CartridgeType::MBC3_RAM_BATTERY:
            case CartridgeType::MBC3_TIMER_BATTERY:
            case CartridgeType::MBC3_TIMER_RAM_BATTERY:
            {
                auto sel = selected_rom_bank_ % cartridge_.GetRomSize();
                for (int i = 0x40; i < 0x80; i++)
                {
                    auto address = (i << 8) % 0x4000;
                    fast_map_[i] = &((rom_banks_[sel])[address]);
                }
                break;
            }
            case CartridgeType::MBC5:
            case CartridgeType::MBC5_RAM:
            case CartridgeType::MBC5_RAM_BATTERY:
            case CartridgeType::MBC5_RUMBLE:
            case CartridgeType::MBC5_RUMBLE_RAM:
            case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
            {
                uint16_t sel = selected_rom_bank_ % cartridge_.GetRomSize();
                sel = sel | (selected_rom_bank_high_ << 8);
                for (int i = 0x40; i < 0x80; i++)
                {
                    auto address = (i << 8) % 0x4000;
                    fast_map_[i] = &((rom_banks_[sel])[address]);
                }
                break;
            }
        }
        if (BiosEnabled)
        {
            // use slow redirecting
            fast_map_[0x0000] = nullptr;
            for (int i = 0x02; i < 0x09; i += 0x01)
            {
                fast_map_[i] = nullptr;
            }
        }
    }

    void Bus::refill_fast_map_vram()
    {
        for (int i = 0x80; i < 0xA0; i++)
        {
            auto address = (i << 8) % 0x2000;
            fast_map_[i] = &((vram_banks_[vram_sel_bank_])[address]);
        }
    }

    void Bus::refill_fast_map_wram()
    {
        for (int i = 0xD0; i < 0xE0; i++)
        {
            auto address = (i << 8) % 0x1000;
            fast_map_[i] = &(wram_banks_[wram_sel_bank_][address]);
        }
        for (int i = 0xF0; i < 0xFE; i++)
        {
            auto address = (i << 8) % 0x1000;
            fast_map_[i] = &(wram_banks_[wram_sel_bank_][address]);
        }
    }

    // TODO: add ram to fast map
    uint8_t& Bus::fast_redirect_address(uint16_t address)
    {
        uint8_t* paddr = fast_map_[address >> 8];
        if (paddr)
        {
            return *(paddr + (address & 0xFF));
        }
        else
        {
            return redirect_address(address);
        }
    }

    uint8_t& Bus::redirect_address(uint16_t address)
    {
        unused_mem_area_ = 0;
        WriteToVram = false;
        // Return address from ROM banks
        // TODO: create better exceptions
        switch (address & 0xF000)
        {
            case 0x0000:
            {
                if (BiosEnabled)
                {
                    if (!UseCGB && dmg_bios_loaded_)
                    {
                        if (address < 0x100)
                        {
                            return dmg_bios_[address];
                        }
                    }
                    else if (UseCGB && cgb_bios_loaded_)
                    {
                        if (address < 0x100)
                        {
                            return cgb_bios_[address];
                        }
                        else if (address >= 0x200 && address < 0x900)
                        {
                            return cgb_bios_[address];
                        }
                    }
                }
                [[fallthrough]]; // This avoids a compiler warning. Fallthrough is intentional
            }
            case 0x1000:
            case 0x2000:
            case 0x3000:
            case 0x4000:
            case 0x5000:
            case 0x6000:
            case 0x7000:
            {
                auto ct = cartridge_.GetCartridgeType();
                switch (ct)
                {
                    case CartridgeType::ROM_ONLY:
                    {
                        int index = address / 0x4000;
                        return (rom_banks_[index])[address % 0x4000];
                    }
                    case CartridgeType::MBC1:
                    case CartridgeType::MBC1_RAM:
                    case CartridgeType::MBC1_RAM_BATTERY:
                    {
                        if (address <= 0x3FFF)
                        {
                            auto sel = (banking_mode_ ? selected_rom_bank_ & 0b1100000 : 0) %
                                       cartridge_.GetRomSize();
                            return (rom_banks_[sel])[address % 0x4000];
                        }
                        else
                        {
                            auto sel = selected_rom_bank_ % cartridge_.GetRomSize();
                            if ((sel & 0b11111) == 0)
                            {
                                // In 4000-7FFF, automatically maps to next addr if addr chosen is
                                // 00/20/40/60
                                // TODO: fix multicart roms
                                sel += 1;
                            }
                            return (rom_banks_[sel])[address % 0x4000];
                        }
                        break;
                    }
                    case CartridgeType::MBC2:
                    case CartridgeType::MBC2_BATTERY:
                    {
                        if (address <= 0x3FFF)
                        {
                            return (rom_banks_[0])[address % 0x4000];
                        }
                        else
                        {
                            if ((selected_rom_bank_ & 0b1111) == 0)
                            {
                                selected_rom_bank_ |= 0b1;
                            }
                            auto sel = selected_rom_bank_ % cartridge_.GetRomSize();
                            return (rom_banks_[sel])[address % 0x4000];
                        }
                        break;
                    }
                    case CartridgeType::MBC3:
                    case CartridgeType::MBC3_RAM:
                    case CartridgeType::MBC3_RAM_BATTERY:
                    case CartridgeType::MBC3_TIMER_BATTERY:
                    case CartridgeType::MBC3_TIMER_RAM_BATTERY:
                    {
                        if (address <= 0x3FFF)
                        {
                            return (rom_banks_[0])[address % 0x4000];
                        }
                        else
                        {
                            auto sel = selected_rom_bank_ % cartridge_.GetRomSize();
                            return (rom_banks_[sel])[address % 0x4000];
                        }
                        break;
                    }
                    case CartridgeType::MBC5:
                    case CartridgeType::MBC5_RAM:
                    case CartridgeType::MBC5_RAM_BATTERY:
                    case CartridgeType::MBC5_RUMBLE:
                    case CartridgeType::MBC5_RUMBLE_RAM:
                    case CartridgeType::MBC5_RUMBLE_RAM_BATTERY:
                    {
                        if (address <= 0x3FFF)
                        {
                            auto sel = (banking_mode_ ? selected_rom_bank_ & 0b1100000 : 0) %
                                       cartridge_.GetRomSize();
                            return (rom_banks_[sel])[address % 0x4000];
                        }
                        else
                        {
                            auto sel = selected_rom_bank_ % cartridge_.GetRomSize();
                            sel = sel | (selected_rom_bank_high_ << 8);
                            return (rom_banks_[sel])[address % 0x4000];
                        }
                        break;
                    }
                    default:
                    {
                        // Unhandled cartridge type.
                        // TODO: stop emulator instead
                        return unused_mem_area_;
                    }
                }
                break;
            }
            case 0x8000:
            case 0x9000:
            {
                WriteToVram = true;
                if (UseCGB)
                {
                    return vram_banks_[vram_sel_bank_][address % 0x2000];
                }
                else
                {
                    return vram_banks_[0][address % 0x2000];
                }
            }
            case 0xA000:
            case 0xB000:
            {
                auto ct = cartridge_.GetCartridgeType();
                switch (ct)
                {
                    case CartridgeType::MBC2:
                    case CartridgeType::MBC2_BATTERY:
                    {
                        if (ram_enabled_)
                        {
                            auto sel =
                                (banking_mode_ ? selected_ram_bank_ : 0) % cartridge_.GetRamSize();
                            (ram_banks_[sel])[address % 0x200] |= 0b1111'0000;
                            return (ram_banks_[sel])[address % 0x200];
                        }
                        else
                        {
                            unused_mem_area_ = 0xFF;
                            return unused_mem_area_;
                        }
                        break;
                    }
                    default:
                    {
                        if (ram_enabled_)
                        {
                            if (cartridge_.GetRamSize() == 0)
                            {
                                return eram_default_[address % 0x2000];
                            }
                            auto sel =
                                (banking_mode_ ? selected_ram_bank_ : 0) % cartridge_.GetRamSize();
                            return (ram_banks_[sel])[address % 0x2000];
                        }
                        else
                        {
                            unused_mem_area_ = 0xFF;
                            return unused_mem_area_;
                        }
                        break;
                    }
                }
            }
            case 0xC000:
            {
                return wram_banks_[0][address % 0x1000];
            }
            case 0xD000:
            {
                return wram_banks_[wram_sel_bank_][address % 0x1000];
            }
            case 0xE000:
            {
                return redirect_address(address - 0x2000);
            }
            case 0xF000:
            {
                if (address <= 0xFDFF)
                {
                    return redirect_address(address - 0x2000);
                }
                else if (address <= 0xFE9F)
                {
                    // OAM
                    if (dma_transfer_ && dma_index_ == 0 && dma_fresh_bug_)
                    {
                        return oam_[address & 0xFF];
                    }
                    else if (dma_transfer_ || !OAMAccessible)
                    {
                        unused_mem_area_ = 0xFF;
                        return unused_mem_area_;
                    }
                    return oam_[address & 0xFF];
                }
                else if (address <= 0xFEFF)
                {
                    return unused_mem_area_;
                }
                else
                {
                    switch (address)
                    {
                        case addr_bcpd:
                        {
                            return bg_cram_[bg_palette_index_];
                        }
                        case addr_ocpd:
                        {
                            return obj_cram_[obj_palette_index_];
                        }
                        case addr_hdma5:
                        {
                            return hdma_remaining_;
                        }
                        default:
                            [[likely]]
                            {
                                return hram_[address % 0xFF00];
                            }
                    }
                }
            }
        }
        return unused_mem_area_;
    }

    uint8_t Bus::Read(uint16_t address)
    {
        switch (address)
        {
            case addr_joy:
            {
                uint8_t res = ~(ActionKeys & DirectionKeys) & 0b00001111;
                if (!res)
                {
                    // No key is currently pressed, return 0xCF
                    return 0b1100'1111;
                }
                return action_key_mode_ ? ActionKeys : DirectionKeys;
            }
        }
        unused_mem_area_ = 0xFF;
        uint8_t read = fast_redirect_address(address);
        return read;
    }

    uint16_t Bus::ReadL(uint16_t address)
    {
        return Read(address) + (Read(address + 1) << 8);
    }

    uint8_t& Bus::GetReference(uint16_t address)
    {
        return redirect_address(address);
    }

    std::vector<RamBank>& Bus::GetRamBanks()
    {
        return ram_banks_;
    }

    std::string Bus::GetVramDump()
    {
        std::stringstream s;
        for (const auto& m : vram_banks_[0])
        {
            s << std::hex << std::setfill('0') << std::setw(2) << m;
        }
        for (size_t i = 0; i < oam_.size(); i += 4)
        {
            s << std::hex << std::setfill('0') << std::setw(2) << oam_[i + 3];
            s << std::hex << std::setfill('0') << std::setw(2) << oam_[i + 2];
            s << std::hex << std::setfill('0') << std::setw(2) << oam_[i + 1];
            s << std::hex << std::setfill('0') << std::setw(2) << oam_[i];
        }
        return std::move(s.str());
    }

    void Bus::Write(uint16_t address, uint8_t data)
    {
        if (address <= 0x7FFF)
        {
            handle_mbc(address, data);
        }
        else
        {
            TIMAChanged = false;
            TMAChanged = false;
            if (!SoundEnabled)
            {
                if (address >= addr_NR10 && address <= addr_NR51)
                {
                    // When sound is disabled, ignore writes
                    return;
                }
            }
            switch (address)
            {
                case addr_std:
                {
                    // TODO: implement serial
                    break;
                }
                case addr_bgp:
                {
                    Change& ch = ScanlineChanges[CurScanlineX];
                    if (UseCGB)
                    {
                    }
                    else
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            BGPalettes[0][i] = (data >> (i * 2)) & 0b11;
                        }
                        ch.new_bg_pal = BGPalettes[0];
                    }
                    break;
                }
                case addr_ob0:
                {
                    if (UseCGB)
                    {
                        // this is free ram in this mode
                        // TODO: they are actually registers in cgb-dmg mode
                    }
                    else
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            OBJPalettes[0][i] = (data >> (i * 2)) & 0b11;
                        }
                    }
                    break;
                }
                case addr_ob1:
                {
                    if (UseCGB)
                    {
                        // this is free ram in this mode
                        // TODO: they are actually registers in cgb-dmg mode
                    }
                    else
                    {
                        for (int i = 0; i < 4; i++)
                        {
                            OBJPalettes[1][i] = (data >> (i * 2)) & 0b11;
                        }
                    }
                    break;
                }
                case addr_dma:
                {
                    if (!dma_transfer_)
                    {
                        dma_fresh_bug_ = true;
                    }
                    else
                    {
                        dma_fresh_bug_ = false;
                    }
                    dma_setup_ = true;
                    dma_transfer_ = false;
                    dma_new_offset_ = data << 8;
                    break;
                }
                case addr_lcd:
                {
                    bool enabled = data & LCDCFlag::LCD_ENABLE;
                    if (!enabled)
                    {
                        OAMAccessible = true;
                    }
                    bool bg_en = data & LCDCFlag::BG_ENABLE;
                    Change& ch = ScanlineChanges[CurScanlineX];
                    ch.new_bg_en = bg_en;
                    break;
                }
                case addr_vbk:
                {
                    if (UseCGB)
                    {
                        vram_sel_bank_ = data & 0b1;
                        data |= 0b1111'1110;
                        refill_fast_map_vram();
                    }
                    break;
                }
                case addr_svbk:
                {
                    if (UseCGB)
                    {
                        wram_sel_bank_ = data & 0b111;
                        if (wram_sel_bank_ == 0)
                        {
                            wram_sel_bank_ = 1;
                        }
                        data |= 0b1111'1000;
                        refill_fast_map_wram();
                    }
                    break;
                }
                case addr_bcps:
                {
                    if (UseCGB)
                    {
                        bg_palette_auto_increment_ = data & 0b1000'0000;
                        bg_palette_index_ = data & 0b11'1111;
                    }
                    else
                    {
                        data |= 0b1111'1111;
                    }
                    break;
                }
                case addr_bcpd:
                {
                    if (UseCGB)
                    {
                        auto pal_index = bg_palette_index_ >> 3;
                        auto color_index = (bg_palette_index_ >> 1) & 0b11;
                        auto byte_index = bg_palette_index_ & 0b1;
                        if (byte_index == 0)
                        {
                            BGPalettes[pal_index][color_index] &= 0xFF00;
                            BGPalettes[pal_index][color_index] |= data;
                        }
                        else
                        {
                            BGPalettes[pal_index][color_index] &= 0x00FF;
                            BGPalettes[pal_index][color_index] |= data << 8;
                        }
                        if (bg_palette_auto_increment_)
                        {
                            ++bg_palette_index_;
                            if (bg_palette_index_ == 0x40)
                            {
                                bg_palette_index_ = 0;
                            }
                        }
                    }
                    else
                    {
                        data |= 0b1111'1111;
                    }
                    break;
                }
                case addr_ocps:
                {
                    if (UseCGB)
                    {
                        obj_palette_auto_increment_ = data & 0b1000'0000;
                        obj_palette_index_ = data & 0b11'1111;
                    }
                    else
                    {
                        data |= 0b1111'1111;
                    }
                    break;
                }
                case addr_ocpd:
                {
                    if (UseCGB)
                    {
                        auto pal_index = obj_palette_index_ >> 3;
                        auto color_index = (obj_palette_index_ >> 1) & 0b11;
                        auto byte_index = obj_palette_index_ & 0b1;
                        if (byte_index == 0)
                        {
                            OBJPalettes[pal_index][color_index] &= 0xFF00;
                            OBJPalettes[pal_index][color_index] |= data;
                        }
                        else
                        {
                            OBJPalettes[pal_index][color_index] &= 0x00FF;
                            OBJPalettes[pal_index][color_index] |= data << 8;
                        }
                        if (obj_palette_auto_increment_)
                        {
                            ++obj_palette_index_;
                            if (obj_palette_index_ == 0x40)
                            {
                                obj_palette_index_ = 0;
                            }
                        }
                    }
                    else
                    {
                        data |= 0b1111'1111;
                    }
                    break;
                }
                case addr_bank:
                {
                    BiosEnabled = false;
                    refill_fast_map_rom();
                    data |= 0b1111'1111;
                    break;
                }
                case addr_hdma1:
                {
                    hdma_source_ &= 0xFF;
                    hdma_source_ |= data << 8;
                    data |= 0b1111'1111;
                    break;
                }
                case addr_hdma2:
                {
                    hdma_source_ &= 0xFF00;
                    hdma_source_ |= data & 0xF0;
                    data |= 0b1111'1111;
                    break;
                }
                case addr_hdma3:
                {
                    hdma_dest_ &= 0xFF;
                    hdma_dest_ |= (data & 0b0001'1111) << 8;
                    data |= 0b1111'1111;
                    break;
                }
                case addr_hdma4:
                {
                    hdma_dest_ &= 0xFF00;
                    hdma_dest_ |= data & 0xF0;
                    data |= 0b1111'1111;
                    break;
                }
                case addr_hdma5:
                {
                    if (!UseCGB)
                    {
                        data |= 0b1111'1111;
                    }
                    else
                    {
                        if (data != 0)
                        {
                            use_gdma_ = data & 0b1000'0000;
                            hdma_size_ = ((data & 0b0111'1111) + 1);
                            hdma_transfer_ = true;
                            hdma_index_ = 0;
                            if (use_gdma_)
                            {
                                for (int i = 0; i < hdma_size_; i++)
                                {
                                    TransferHDMA();
                                }
                            }
                        }
                        else
                        {
                            hdma_transfer_ = false;
                        }
                    }
                    break;
                }
                case addr_div:
                {
                    DIVReset = true;
                    break;
                }
                case addr_tac:
                {
                    data |= 0b1111'1000;
                    break;
                }
                case addr_tim:
                {
                    TIMAChanged = true;
                    break;
                }
                case addr_tma:
                {
                    TMAChanged = true;
                    break;
                }
                case addr_joy:
                {
                    action_key_mode_ = (data == 0x10);
                    return;
                }
                case addr_stc:
                {
                    data |= 0b0111'1110;
                    break;
                }
                case addr_ifl:
                {
                    data |= 0b1110'0000;
                    break;
                }
                case addr_sta:
                {
                    data |= 0b1000'0000;
                    data |= Read(addr_sta) & 0b11;
                    break;
                }
                case addr_NR10:
                {
                    (*channel_array_ptr_)[0].SweepPeriod = (data >> 4) & 0b111;
                    (*channel_array_ptr_)[0].SweepIncrease = !(data & 0b1000);
                    (*channel_array_ptr_)[0].SweepShift = data & 0b111;
                    data |= 0b1000'0000;
                    break;
                }
                case addr_NR11:
                {
                    handle_nrx1(1, data);
                    data |= 0b0011'1111;
                    break;
                }
                case addr_NR12:
                {
                    handle_nrx2(1, data);
                    break;
                }
                case addr_NR13:
                {
                    (*channel_array_ptr_)[0].Frequency &= 0b0111'0000'0000;
                    (*channel_array_ptr_)[0].Frequency |= data;
                    (*channel_array_ptr_)[0].ShadowFrequency = (*channel_array_ptr_)[0].Frequency;
                    data |= 0b1111'1111;
                    break;
                }
                case addr_NR14:
                {
                    handle_nrx4(1, data);
                    auto& chan = (*channel_array_ptr_)[0];
                    if (chan.SweepShift > 0)
                    {
                        chan.CalculateSweepFreq();
                        if (chan.DisableChannelFlag)
                        {
                            ClearNR52Bit(0);
                            chan.DACEnabled = false;
                            chan.DisableChannelFlag = false;
                        }
                    }
                    break;
                }
                case addr_NR20:
                {
                    data |= 0b1111'1111;
                    break;
                }
                case addr_NR21:
                {
                    handle_nrx1(2, data);
                    data |= 0b0011'1111;
                    break;
                }
                case addr_NR22:
                {
                    handle_nrx2(2, data);
                    break;
                }
                case addr_NR23:
                {
                    (*channel_array_ptr_)[1].Frequency &= 0b0111'0000'0000;
                    (*channel_array_ptr_)[1].Frequency |= data;
                    data |= 0b1111'1111;
                    break;
                }
                case addr_NR24:
                {
                    handle_nrx4(2, data);
                    break;
                }
                case addr_NR30:
                {
                    if (!(data & 0b1000'0000))
                    {
                        disable_dac(2);
                    }
                    else
                    {
                        (*channel_array_ptr_)[2].DACEnabled = true;
                    }
                    data |= 0b0111'1111;
                    break;
                }
                case addr_NR31:
                {
                    handle_nrx1(3, data);
                    data |= 0b1111'1111;
                    break;
                }
                case addr_NR32:
                {
                    data |= 0b1001'1111;
                    break;
                }
                case addr_NR33:
                {
                    (*channel_array_ptr_)[2].Frequency &= 0b0111'0000'0000;
                    (*channel_array_ptr_)[2].Frequency |= data;
                    data |= 0b1111'1111;
                    break;
                }
                case addr_NR34:
                {
                    handle_nrx4(3, data);
                    break;
                }
                case addr_NR40:
                {
                    data |= 0b1111'1111;
                    break;
                }
                case addr_NR41:
                {
                    auto& chan = (*channel_array_ptr_)[3];
                    chan.LFSR = 0xFFFF;
                    handle_nrx1(4, data);
                    data |= 0b1111'1111;
                    break;
                }
                case addr_NR42:
                {
                    handle_nrx2(4, data);
                    break;
                }
                case addr_NR43:
                {
                    auto& chan = (*channel_array_ptr_)[3];
                    auto shift_amount = data >> 4;
                    auto divisor_code = data & 0b111;
                    auto divisor = divisor_code * 16;
                    if (divisor == 0)
                    {
                        divisor = 8;
                    }
                    chan.FrequencyTimer = divisor << shift_amount;
                    chan.Divisor = divisor;
                    chan.DivisorShift = shift_amount;
                    chan.WidthMode = data & 0b1000;
                    break;
                }
                case addr_NR44:
                {
                    handle_nrx4(4, data);
                    break;
                }
                case addr_NR50:
                {
                    for (int i = 0; i < 4; i++)
                    {
                        auto& ch = (*channel_array_ptr_)[i];
                        ch.RightVolume = data & 0b111;
                        ch.LeftVolume = (data >> 4) & 0b111;
                    }
                    break;
                }
                case addr_NR51:
                {
                    for (int i = 0; i < 4; i++)
                    {
                        auto& ch = (*channel_array_ptr_)[i];
                        ch.RightEnabled = (data >> i) & 0b1;
                        ch.LeftEnabled = (data >> (i + 4)) & 0b1;
                    }
                    break;
                }
                case addr_NR52:
                {
                    data &= 0b1111'0000;
                    bool enabled = data & 0b1000'0000;
                    if (!enabled)
                    {
                        // When sound is disabled, clear all registers
                        for (int i = addr_NR10; i <= addr_NR51; i++)
                        {
                            Write(i, 0);
                        }
                        data = 0;
                    }
                    SoundEnabled = enabled;
                    data |= 0b0111'0000;
                    break;
                }
                // Unused HWIO registers
                // Writing to these sets all the bits
                case 0xFF03:
                case 0xFF08:
                case 0xFF09:
                case 0xFF0A:
                case 0xFF0B:
                case 0xFF0C:
                case 0xFF0D:
                case 0xFF0E:
                case 0xFF27:
                case 0xFF28:
                case 0xFF29:
                case 0xFF2A:
                case 0xFF2B:
                case 0xFF2C:
                case 0xFF2D:
                case 0xFF2E:
                case 0xFF2F:
                case 0xFF4C:
                case 0xFF4D:
                case 0xFF4E:
                case 0xFF56:
                case 0xFF57:
                case 0xFF58:
                case 0xFF59:
                case 0xFF5A:
                case 0xFF5B:
                case 0xFF5C:
                case 0xFF5D:
                case 0xFF5E:
                case 0xFF5F:
                case 0xFF60:
                case 0xFF61:
                case 0xFF62:
                case 0xFF63:
                case 0xFF64:
                case 0xFF65:
                case 0xFF66:
                case 0xFF67:
                case 0xFF6C:
                case 0xFF6D:
                case 0xFF6E:
                case 0xFF6F:
                case 0xFF71:
                case 0xFF72:
                case 0xFF73:
                case 0xFF74:
                case 0xFF75:
                case 0xFF76:
                case 0xFF77:
                case 0xFF78:
                case 0xFF79:
                case 0xFF7A:
                case 0xFF7B:
                case 0xFF7C:
                case 0xFF7D:
                case 0xFF7E:
                case 0xFF7F:
                {
                    data |= 0b1111'1111;
                    break;
                }
            }
            fast_redirect_address(address) = data;
        }
    }

    void Bus::WriteL(uint16_t address, uint16_t data)
    {
        Write(address, data & 0xFF);
        Write(address + 1, data >> 8);
    }

    void Bus::ClearNR52Bit(uint8_t bit)
    {
        redirect_address(addr_NR52) &= ~(1 << bit);
    }

    void Bus::Reset()
    {
        SoftReset();
        for (auto& rom : rom_banks_)
        {
            rom.fill(0xFF);
        }
    }

    void Bus::SoftReset()
    {
        hram_.fill(0);
        SoundEnabled = true;
        // for (int i = 0xFF10; i < 0xFF25; i++) {
        //     Write(i, 0);
        // }
        // SoundEnabled = false;
        oam_.fill(0);
        vram_banks_[0].fill(0);
        vram_banks_[1].fill(0);
        DirectionKeys = 0b1110'1111;
        ActionKeys = 0b1101'1111;
        selected_rom_bank_ = 1;
        selected_ram_bank_ = 0;
        wram_sel_bank_ = 1;
        vram_sel_bank_ = UseCGB;
        BiosEnabled = true;
    }

    Cartridge& Bus::GetCartridge()
    {
        return cartridge_;
    }

    bool Bus::LoadCartridge(const std::string& filename)
    {
        Reset();
        bool ret = cartridge_.Load(filename, rom_banks_, ram_banks_);
        if (!ret)
        {
            return false;
        }
        rom_banks_size_ = cartridge_.GetRomSize();
        if (cartridge_.UsingBattery())
        {
            auto path = static_cast<std::filesystem::path>(filename);
            std::string path_save = path.parent_path().string();
            path_save += "/";
            path_save += path.stem().string();
            path_save += ".sav";
            curr_save_file_ = path_save;
            if (std::filesystem::exists(path_save))
            {
                std::ifstream is;
                is.open(path_save, std::ios::binary);
                if (is.is_open() && ram_banks_.size() > 0)
                {
                    for (size_t i = 0; i < ram_banks_.size(); ++i)
                    {
                        is.read(reinterpret_cast<char*>(&ram_banks_[i]), sizeof(uint8_t) * 0x2000);
                    }
                }
                is.close();
            }
        }
        BiosEnabled = true;
        UseCGB = cartridge_.UseCGB;
        SoftReset();
        fill_fast_map();
        return true;
    }

    bool Bus::LoadCartridge(uint8_t* data)
    {
        Reset();
        Header header = *reinterpret_cast<Header*>(data + 0x100);
        cartridge_.header_ = header;
        rom_banks_.resize(cartridge_.GetRomSize());
        ram_banks_.resize(cartridge_.GetRamSize());
        for (size_t i = 0; i < rom_banks_.size(); i++)
        {
            std::memcpy(rom_banks_[i].data(), data + (i * 0x4000), sizeof(uint8_t) * 0x4000);
        }
        rom_banks_size_ = cartridge_.GetRomSize();
        BiosEnabled = false;
        UseCGB = cartridge_.header_.gameboyColor & 0x80;
        SoftReset();
        fill_fast_map();
        return true;
    }

    void Bus::TransferDMA(uint8_t clk)
    {
        if (dma_transfer_)
        {
            int times = clk / 4;
            for (int i = 0; i < times; ++i)
            {
                auto index = dma_index_ + i;
                if (index < oam_.size())
                {
                    uint16_t source = dma_offset_ | index;
                    bool old = OAMAccessible;
                    OAMAccessible = true;
                    oam_[index] = Read(source);
                    OAMAccessible = old;
                }
                else
                {
                    dma_transfer_ = false;
                    dma_fresh_bug_ = false;
                    return;
                }
            }
            dma_index_ += times;
        }
        if (dma_setup_)
        {
            dma_transfer_ = true;
            dma_setup_ = false;
            dma_index_ = 0;
            dma_offset_ = dma_new_offset_;
        }
    }

    void Bus::TransferHDMA()
    {
        if (hdma_transfer_)
        {
            if (hdma_index_ < hdma_size_)
            {
#pragma GCC unroll 10
                for (int i = 0; i < 16; i++)
                {
                    Write(hdma_dest_++, Read(hdma_source_++));
                }
                hdma_index_++;
                hdma_remaining_--;
            }
            else
            {
                hdma_transfer_ = false;
            }
        }
    }

    void Bus::battery_save()
    {
        if (cartridge_.UsingBattery())
        {
            std::ofstream of(curr_save_file_, std::ios::binary);
            if (cartridge_.GetRamSize() != 0)
            {
                for (int i = 0; i < cartridge_.GetRamSize(); ++i)
                {
                    of.write(reinterpret_cast<char*>(&ram_banks_[i]), sizeof(uint8_t) * 0x2000);
                }
            }
            else
            {
                of.write(reinterpret_cast<char*>(&ram_banks_[0]), sizeof(uint8_t) * 0x2000);
            }
        }
    }

    void Bus::handle_nrx4(int channel_no, uint8_t& data)
    {
        --channel_no;
        auto& chan = (*channel_array_ptr_)[channel_no];
        bool old = chan.LengthDecOne;
        chan.LengthDecOne = data & 0b0100'0000;
        if (!old && chan.LengthDecOne)
        {
            // From 03-trigger blargg test:
            // Enabling in first half of length period should clock length
            // Because length clocks every 2 frame sequencer steps, first half is every time its
            // even
            if ((chan.FrameSequencer % 2 == 0))
            {
                chan.ClockLengthCtr();
            }
            // If clock makes length zero, should disable chan
            if (chan.LengthTimer == 0)
            {
                ClearNR52Bit(channel_no);
            }
        }
        if (channel_no < 2)
        {
            chan.Frequency &= 0b0000'1111'1111;
            chan.Frequency |= (data & 0b111) << 8;
            chan.ShadowFrequency = chan.Frequency;
        }
        if ((data & 0b1000'0000))
        {
            chan.LengthCtrEnabled = true;
            if (chan.LengthTimer == 0)
            {
                chan.LengthTimer = chan.LengthInit;
                if (data & 0b0100'0000 && (chan.FrameSequencer % 2 == 0))
                {
                    // Trigger that un-freezes enabled length should clock it
                    --chan.LengthTimer;
                }
            }
            if (chan.DACEnabled)
            {
                redirect_address(addr_NR52) |= 1 << channel_no;
            }
        }
        data |= 0b1011'1111;
    }

    void Bus::handle_nrx2(int channel_no, uint8_t& data)
    {
        --channel_no;
        auto& chan = (*channel_array_ptr_)[channel_no];
        chan.EnvelopeCurrentVolume = data >> 4;
        chan.EnvelopeIncrease = data & 0b0000'1000;
        int sweep = data & 0b0000'0111;
        chan.EnvelopePeriod = sweep;
        chan.PeriodTimer = chan.EnvelopePeriod;
        chan.VolEnvEnabled = sweep != 0;
        if ((data >> 3) == 0)
        {
            disable_dac(channel_no);
        }
        else
        {
            chan.DACEnabled = true;
        }
    }

    void Bus::handle_nrx1(int channel_no, uint8_t& data)
    {
        --channel_no;
        auto& chan = (*channel_array_ptr_)[channel_no];
        chan.LengthData = data & (chan.LengthInit - 1); // and with highest value
        chan.LengthTimer = chan.LengthInit - chan.LengthData;
        chan.LengthHalf = chan.LengthTimer / 2;
        chan.WaveDutyPattern = data >> 6;
    }

    void Bus::disable_dac(int channel_no)
    {
        auto& chan = (*channel_array_ptr_)[channel_no];
        ClearNR52Bit(channel_no);
        chan.DACEnabled = false;
        chan.LengthTimer = 0;
    }
} // namespace hydra::Gameboy
