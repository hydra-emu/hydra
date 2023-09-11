#include <atomic>
#include <chrono>
#include <emulator_settings.hxx>
#include <emulator_user_data.hxx>
#include <filesystem>
#include <gb/gb_hc.hxx>
#include <iostream>

namespace hydra
{
    HydraCore_Gameboy::HydraCore_Gameboy()
        : channel_array_ptr_(std::make_shared<ChannelArray>()), bus_(channel_array_ptr_),
          apu_(channel_array_ptr_, bus_.GetReference(addr_NR52)), ppu_(bus_),
          timer_(channel_array_ptr_, bus_), cpu_(bus_, ppu_, apu_, timer_),
          joypad_(bus_.GetReference(addr_joy)), interrupt_flag_(bus_.GetReference(addr_if))
    {
        (*channel_array_ptr_.get())[0].HasSweep = true;
        const EmulatorUserData& user_data =
            EmulatorSettings::GetEmulatorData(EmuType::Gameboy).UserData;
        KeyMappings& mappings = EmulatorSettings::GetEmulatorData(EmuType::Gameboy).Mappings;
        if (!mappings.empty())
        {
            for (int i = 0; i < 4; i++)
            {
                auto color = std::stoi(user_data.Get(std::string("dmg_c") + std::to_string(i)));
                bus_.Palette[i][0] = color & 0xFF;
                bus_.Palette[i][1] = (color >> 8) & 0xFF;
                bus_.Palette[i][2] = color >> 16;
            }
            direction_keys_[0] = std::stoi(mappings["Right"]);
            direction_keys_[1] = std::stoi(mappings["Left"]);
            direction_keys_[2] = std::stoi(mappings["Up"]);
            direction_keys_[3] = std::stoi(mappings["Down"]);
            action_keys_[0] = std::stoi(mappings["A"]);
            action_keys_[1] = std::stoi(mappings["B"]);
            action_keys_[2] = std::stoi(mappings["Start"]);
            action_keys_[3] = std::stoi(mappings["Select"]);
        }
        if (!user_data.IsEmpty())
        {
            if (user_data.Get("skip_bios") == "false")
            {
                auto dmg_path = user_data.Get("dmg_path");
                auto cgb_path = user_data.Get("cgb_path");
                if (std::filesystem::exists(dmg_path))
                {
                    std::ifstream ifs(dmg_path, std::ios::binary);
                    if (ifs.is_open())
                    {
                        ifs.read(reinterpret_cast<char*>(&bus_.dmg_bios_[0]),
                                 sizeof(bus_.dmg_bios_));
                        ifs.close();
                        bus_.dmg_bios_loaded_ = true;
                        bus_.BiosEnabled = true;
                    }
                }
                if (std::filesystem::exists(cgb_path))
                {
                    std::ifstream ifs(cgb_path, std::ios::binary);
                    if (ifs.is_open())
                    {
                        ifs.read(reinterpret_cast<char*>(&bus_.cgb_bios_[0]),
                                 sizeof(bus_.cgb_bios_));
                        ifs.close();
                        bus_.cgb_bios_loaded_ = true;
                        bus_.BiosEnabled = true;
                    }
                }
            }
        }
        apu_.UseSound = true;
        apu_.InitSound();
    }

    void HydraCore_Gameboy::reset()
    {
        bus_.SoftReset();
        cpu_.Reset(true);
        timer_.Reset();
        ppu_.Reset();
    }

    void HydraCore_Gameboy::run_frame()
    {
        for (int i = 0; i < 70224; i++)
        {
            uint8_t old_if = interrupt_flag_;
            int clk = 0;
            if (!cpu_.skip_next_)
            {
                clk = cpu_.Update();
            }
            cpu_.skip_next_ = false;
            if (timer_.Update(clk, old_if))
            {
                if (cpu_.halt_)
                {
                    cpu_.halt_ = false;
                    cpu_.skip_next_ = true;
                }
            }
            ppu_.Update(clk);
            apu_.Update(clk);
        }
    }

    bool HydraCore_Gameboy::load_file(const std::string& type, const std::string& path)
    {
        if (type == "rom")
        {
            auto loaded = bus_.LoadCartridge(path);
            ppu_.UseCGB = bus_.UseCGB;
            return loaded;
        }
        return false;
    }

    VideoInfo HydraCore_Gameboy::render_frame()
    {
        VideoInfo info;
        info.width = 160;
        info.height = 144;
        info.data.resize(info.width * info.height * 4);
        memcpy(info.data.data(), ppu_.GetScreenData(), info.data.size());
        info.format = VideoFormat::RGBA8888;
        return info;
    }

    AudioInfo HydraCore_Gameboy::render_audio()
    {
        AudioInfo info;
        return info;
    }

} // namespace hydra
