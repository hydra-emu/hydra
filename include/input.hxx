#pragma once

#include <array>
#include <cstdint>
#include <hydra/core.hxx>
#include <SDL3/SDL_keyboard.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace hydra
{
    struct KeyMappings
    {
        std::array<SDL_Scancode, (int)hydra::ButtonType::InputCount> mappings;
    };

    struct Input
    {
        static void Init();
        static void Poll();
        static int32_t Check(uint32_t player, hydra::ButtonType button);
        static bool Add(const std::string& name, const KeyMappings& mapping);
        static bool Remove(const std::string& name);
        static bool IsAssociated(const std::string& name);
        static bool Associate(uint32_t player, const std::string& name);
        static void Unassociate();
        static const std::vector<std::string>& GetMappingNames();
        static KeyMappings& GetMapping(const std::string& name);
    };

    constexpr const char* serialize(hydra::ButtonType input)
    {
        switch (input)
        {
            case hydra::ButtonType::Keypad1Up:
                return "Left Keypad - Up button";
            case hydra::ButtonType::Keypad1Down:
                return "Left Keypad - Down button";
            case hydra::ButtonType::Keypad1Left:
                return "Left Keypad - Left button";
            case hydra::ButtonType::Keypad1Right:
                return "Left Keypad - Right button";
            case hydra::ButtonType::Keypad2Up:
                return "Right Keypad - Up button";
            case hydra::ButtonType::Keypad2Down:
                return "Right Keypad - Down button";
            case hydra::ButtonType::Keypad2Left:
                return "Right Keypad - Left button";
            case hydra::ButtonType::Keypad2Right:
                return "Right Keypad - Right button";
            case hydra::ButtonType::A:
                return "A button";
            case hydra::ButtonType::B:
                return "B button";
            case hydra::ButtonType::X:
                return "X button";
            case hydra::ButtonType::Y:
                return "Y button";
            case hydra::ButtonType::Z:
                return "Z button";
            case hydra::ButtonType::L1:
                return "L1 button";
            case hydra::ButtonType::R1:
                return "R1 button";
            case hydra::ButtonType::L2:
                return "L2 button";
            case hydra::ButtonType::R2:
                return "R2 button";
            case hydra::ButtonType::L3:
                return "L3 button";
            case hydra::ButtonType::R3:
                return "R3 button";
            case hydra::ButtonType::Start:
                return "Start button";
            case hydra::ButtonType::Select:
                return "Select button";
            case hydra::ButtonType::Touch:
                return "Touch";
            case hydra::ButtonType::Analog1Up:
                return "Left Analog Stick - Up";
            case hydra::ButtonType::Analog1Down:
                return "Left Analog Stick - Down";
            case hydra::ButtonType::Analog1Left:
                return "Left Analog Stick - Left";
            case hydra::ButtonType::Analog1Right:
                return "Left Analog Stick - Right";
            case hydra::ButtonType::Analog2Up:
                return "Right Analog Stick - Up";
            case hydra::ButtonType::Analog2Down:
                return "Right Analog Stick - Down";
            case hydra::ButtonType::Analog2Left:
                return "Right Analog Stick - Left";
            case hydra::ButtonType::Analog2Right:
                return "Right Analog Stick - Right";
            default:
                return "Unknown";
        }
    }
} // namespace hydra