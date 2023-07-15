#include <atomic>
#include <chrono>
#include <emulator_settings.hxx>
#include <emulator_user_data.hxx>
#include <filesystem>
#include <gb/gb_tkpwrapper.hxx>
#include <iostream>

namespace hydra::Gameboy
{
    Gameboy_TKPWrapper::Gameboy_TKPWrapper()
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
                SkipBoot = !bus_.BiosEnabled;
            }
            else
            {
                SkipBoot = true;
            }
        }
        else
        {
            SkipBoot = true;
        }
        apu_.UseSound = true;
        apu_.InitSound();
        instrs_per_frame_ = 70224;
    }

    Gameboy_TKPWrapper::~Gameboy_TKPWrapper()
    {
        Stopped.store(true);
    }

    void Gameboy_TKPWrapper::reset()
    {
        bus_.SoftReset();
        cpu_.Reset(SkipBoot);
        timer_.Reset();
        ppu_.Reset();
    }

    void Gameboy_TKPWrapper::update()
    {
        update_audio_sync();
    }

    void Gameboy_TKPWrapper::update_audio_sync()
    {
        if ((apu_.IsQueueEmpty()) || FastMode)
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
        else
        {
            // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void Gameboy_TKPWrapper::HandleKeyDown(uint32_t key)
    {
        if (auto it_dir = std::find(direction_keys_.begin(), direction_keys_.end(), key);
            it_dir != direction_keys_.end())
        {
            int index = it_dir - direction_keys_.begin();
            bus_.DirectionKeys &= (~(1UL << index));
            interrupt_flag_ |= IFInterrupt::JOYPAD;
        }
        if (auto it_dir = std::find(action_keys_.begin(), action_keys_.end(), key);
            it_dir != action_keys_.end())
        {
            int index = it_dir - action_keys_.begin();
            bus_.ActionKeys &= (~(1UL << index));
            interrupt_flag_ |= IFInterrupt::JOYPAD;
        }
    }

    void Gameboy_TKPWrapper::HandleKeyUp(uint32_t key)
    {
        if (auto it_dir = std::find(direction_keys_.begin(), direction_keys_.end(), key);
            it_dir != direction_keys_.end())
        {
            int index = it_dir - direction_keys_.begin();
            bus_.DirectionKeys |= (1UL << index);
        }
        if (auto it_dir = std::find(action_keys_.begin(), action_keys_.end(), key);
            it_dir != action_keys_.end())
        {
            int index = it_dir - action_keys_.begin();
            bus_.ActionKeys |= (1UL << index);
        }
    }

    bool Gameboy_TKPWrapper::load_file(const std::string& path)
    {
        auto loaded = bus_.LoadCartridge(path);
        ppu_.UseCGB = bus_.UseCGB;
        return loaded;
    }

    void* Gameboy_TKPWrapper::GetScreenData()
    {
        return ppu_.GetScreenData();
    }
} // namespace hydra::Gameboy
