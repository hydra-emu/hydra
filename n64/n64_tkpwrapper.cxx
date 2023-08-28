#include "core/n64_keys.hxx"
#include <emulator_factory.hxx>
#include <emulator_settings.hxx>
#include <fmt/format.h>
#include <iostream>
#include <n64/n64_tkpwrapper.hxx>

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
    {
        ++it;
    }
    return !s.empty() && it == s.end();
}

uint32_t get_key_number(std::string key_s)
{
    switch (str_hash(key_s.c_str()))
    {
        case str_hash("A"):
            return hydra::N64::Keys::A;
        case str_hash("B"):
            return hydra::N64::Keys::B;
        case str_hash("Z"):
            return hydra::N64::Keys::Z;
        case str_hash("Start"):
            return hydra::N64::Keys::Start;
        case str_hash("Up"):
            return hydra::N64::Keys::Up;
        case str_hash("Down"):
            return hydra::N64::Keys::Down;
        case str_hash("Left"):
            return hydra::N64::Keys::Left;
        case str_hash("Right"):
            return hydra::N64::Keys::Right;
        case str_hash("L"):
            return hydra::N64::Keys::L;
        case str_hash("R"):
            return hydra::N64::Keys::R;
        case str_hash("CUp"):
            return hydra::N64::Keys::CUp;
        case str_hash("CDown"):
            return hydra::N64::Keys::CDown;
        case str_hash("CLeft"):
            return hydra::N64::Keys::CLeft;
        case str_hash("CRight"):
            return hydra::N64::Keys::CRight;
        case str_hash("KeypadDown"):
            return hydra::N64::Keys::KeypadDown;
        case str_hash("KeypadUp"):
            return hydra::N64::Keys::KeypadUp;
        case str_hash("KeypadLeft"):
            return hydra::N64::Keys::KeypadLeft;
        case str_hash("KeypadRight"):
            return hydra::N64::Keys::KeypadRight;
    }

    return hydra::N64::Keys::ErrorKey;
}

namespace hydra::N64
{
    bool N64_TKPWrapper::ipl_loaded_ = false;

    N64_TKPWrapper::N64_TKPWrapper() : n64_impl_(should_draw_)
    {
        using namespace hydra::N64::Keys;
        instrs_per_frame_ = 1;
        KeyMappings& mappings = EmulatorSettings::GetEmulatorData(EmuType::N64).Mappings;
        if (mappings.size() == 0)
        {
            Logger::Warn("No key mappings found for N64!");
        }
        else
        {
            for (auto& mapping : mappings)
            {
                std::string key = mapping.first;
                std::string value = mapping.second;
                if (!is_number(value))
                {
                    Logger::Fatal("Invalid key mapping: {} -> {} (not a number)", key, value);
                }
                uint32_t key_n = get_key_number(key);
                uint32_t value_n = std::stoi(value);
                if (key_n == ErrorKey)
                {
                    Logger::Fatal("Invalid key mapping: {} -> {}", key_n, value_n);
                }
                key_mappings_[value_n] = key_n;
            }
        }

        width_ = 640;
        height_ = 480;
    }

    N64_TKPWrapper::~N64_TKPWrapper() {}

    bool N64_TKPWrapper::load_file(const std::string& path)
    {
        bool ipl_loaded = ipl_loaded_;
        if (!ipl_loaded)
        {
            auto ipl_path = EmulatorSettings::GetEmulatorData(EmuType::N64).UserData.Get("IPLPath");
            if (!std::filesystem::exists(ipl_path))
            {
                throw std::runtime_error("Missing IPL path!");
            }
            bool ipl_status = n64_impl_.LoadIPL(ipl_path);
            ipl_loaded = ipl_status;
        }
        bool opened = n64_impl_.LoadCartridge(path);
        Loaded = opened && ipl_loaded;
        return Loaded;
    }

    void N64_TKPWrapper::reset()
    {
        n64_impl_.Reset();
    }

    void N64_TKPWrapper::update()
    {
        try
        {
            n64_impl_.Update();
        } catch (std::exception& ex)
        {
            fmt::print("{}\n", ex.what());
            fmt::print("Current pc: {:#x}\n", n64_impl_.cpu_.pc_);
            stop();
        }
    }

    void N64_TKPWrapper::HandleKeyDown(uint32_t key)
    {
        if (key_mappings_.find(key) != key_mappings_.end())
        {
            n64_impl_.SetKeyState(key_mappings_[key], true);
        }
    }

    void N64_TKPWrapper::HandleKeyUp(uint32_t key)
    {
        if (key_mappings_.find(key) != key_mappings_.end())
        {
            n64_impl_.SetKeyState(key_mappings_[key], false);
        }
    }

    void N64_TKPWrapper::HandleMouseMove(int32_t x, int32_t y)
    {
        n64_impl_.SetMousePos(x, y);
    }

    void* N64_TKPWrapper::GetScreenData()
    {
        return n64_impl_.GetColorData();
    }
} // namespace hydra::N64