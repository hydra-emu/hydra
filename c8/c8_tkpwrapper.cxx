#include <c8/c8_tkpwrapper.hxx>
#include <iostream>

namespace hydra::c8
{
    Chip8_TKPWrapper::Chip8_TKPWrapper()
    {
        width_ = 64;
        height_ = 32;

        instrs_per_frame_ = 10;
    }

    // Chip8_TKPWrapper::c8(std::unique_ptr<OptionsBase> args) : c8() {
    //     // key_mappings_ = std::any_cast<c8Keys>(args);
    // }
    Chip8_TKPWrapper::~Chip8_TKPWrapper()
    {
        Stopped.store(true);
    }

    bool Chip8_TKPWrapper::load_file(const std::string& path)
    {
        inter_.load_file(path);
        Loaded = true;
        return true;
    }

    void Chip8_TKPWrapper::update()
    {
        inter_.Update();
    }

    void Chip8_TKPWrapper::reset()
    {
        inter_.reset();
    }

    void Chip8_TKPWrapper::HandleKeyDown(uint32_t key)
    {
        for (int i = 0; i < 16; i++)
        {
            if (key_mappings_[i] == key)
            {
                inter_.key_pressed_[i] = true;
                break;
            }
        }
    }

    void Chip8_TKPWrapper::HandleKeyUp(uint32_t key)
    {
        for (int i = 0; i < 16; i++)
        {
            if (key_mappings_[i] == key)
            {
                inter_.key_pressed_[i] = false;
                break;
            }
        }
    }

    void* Chip8_TKPWrapper::GetScreenData()
    {
        return inter_.GetScreenData();
    }
} // namespace hydra::c8