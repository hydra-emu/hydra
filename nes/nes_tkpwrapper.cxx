#include "nes_tkpwrapper.hxx"
#include <emulator_settings.hxx>
#include <log.hxx>

namespace hydra::NES
{
    NES_TKPWrapper::NES_TKPWrapper()
    {
        using hydra::NES::Button;
        instrs_per_frame_ = 1789773 / 60;
        ppu_.SetNMI(std::bind(&CPU::NMI, &cpu_));
        KeyMappings& mappings = EmulatorSettings::GetEmulatorData(EmuType::NES).Mappings;
        std::unordered_map<uint32_t, Button> keymap;
        if (!mappings.empty())
        {
            keymap[std::stoi(mappings["Right"])] = Button::Right;
            keymap[std::stoi(mappings["Left"])] = Button::Left;
            keymap[std::stoi(mappings["Up"])] = Button::Up;
            keymap[std::stoi(mappings["Down"])] = Button::Down;
            keymap[std::stoi(mappings["A"])] = Button::A;
            keymap[std::stoi(mappings["B"])] = Button::B;
            keymap[std::stoi(mappings["Start"])] = Button::Start;
            keymap[std::stoi(mappings["Select"])] = Button::Select;
        } else
        {
            Logger::Warn("No key mappings found for NES");
        }
        cpu_.SetKeys(keymap);
    }

    NES_TKPWrapper::~NES_TKPWrapper() {}

    void NES_TKPWrapper::HandleKeyDown(uint32_t key) { cpu_.HandleKeyDown(key); }

    void NES_TKPWrapper::HandleKeyUp(uint32_t key) { cpu_.HandleKeyUp(key); }

    void* NES_TKPWrapper::GetScreenData() { return ppu_.GetScreenData(); }

    void NES_TKPWrapper::reset()
    {
        cpu_.SoftReset();
        ppu_.Reset();
    }

    void NES_TKPWrapper::update() { cpu_.Tick(); }

    bool NES_TKPWrapper::load_file(std::string path) { return cpu_.bus_.LoadCartridge(path); }
} // namespace hydra::NES