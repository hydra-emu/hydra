#pragma once

namespace hydra::NES
{
    enum class Button {
        Right = 0b10000000,
        Left = 0b01000000,
        Down = 0b00100000,
        Up = 0b00010000,
        Start = 0b00001000,
        Select = 0b00000100,
        B = 0b00000010,
        A = 0b00000001,
    };

    struct Joypad
    {
        Joypad() : state_(0) {}

        void set_strobe(bool b)
        {
            strobe_ = b;
            state_ptr_ = 0;
        }

        uint8_t ReadCurrent()
        {
            uint8_t temp = (state_ >> state_ptr_) & 1;
            if (!strobe_)
            {
                state_ptr_++;
            }
            return temp;
        }

        void Press(Button button)
        {
            state_ |= static_cast<uint8_t>(button);
        }

        void Release(Button button)
        {
            state_ &= ~static_cast<uint8_t>(button);
        }

      private:
        uint8_t state_;
        bool strobe_ = false;
        uint8_t state_ptr_ = 0;
    };
} // namespace hydra::NES