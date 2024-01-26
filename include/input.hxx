#pragma once

#include <array>
#include <cstdint>
#include <hydra/core.hxx>
#include <SDL2/SDL_keyboard.h>
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

    private:
        static const uint8_t* keyboardState;
        static int keyboardStateSize;
        static std::unordered_map<std::string, KeyMappings> mappings;
        static std::vector<KeyMappings*> playerMappings;
    };
} // namespace hydra