#include <filesystem>
#include <input.hxx>
#include <SDL_keyboard.h>
#include <settings.hxx>

namespace hydra
{

    const uint8_t* keyboardState = nullptr;
    int keyboardStateSize = 0;
    std::unordered_map<std::string, KeyMappings> mappings;
    std::vector<KeyMappings*> playerMappings;
    std::vector<std::string> mappingNames;
    bool mappingNamesDirty = true;

    void Input::Init()
    {
        std::filesystem::path path = Settings::GetSavePath() / "mappings";

        std::filesystem::directory_iterator end;

        for (std::filesystem::directory_iterator it(path); it != end; ++it)
        {
            if (it->is_regular_file())
            {
                std::filesystem::path file = it->path();
                if (file.extension() != ".json")
                {
                    continue;
                }

                std::string data = hydra::fread(file);
                nlohmann::json json_mappings;
                try
                {
                    json_mappings = nlohmann::json::parse(data);
                } catch (std::exception& e)
                {
                    hydra::log("Failed to parse mappings file: {}", e.what());
                    ::remove(file.c_str());
                    continue;
                }

                std::string name = json_mappings["name"];
                KeyMappings mappings;

                for (int i = 0; i < (int)hydra::ButtonType::InputCount; ++i)
                {
                    mappings.mappings[i] = json_mappings["mappings"][i];
                }

                Add(name, mappings);
            }
        }

        KeyMappings replacement_mappings;
#define map(button, key) replacement_mappings.mappings[(int)hydra::ButtonType::button] = key
        map(Keypad1Up, SDL_SCANCODE_UP);
        map(Keypad1Down, SDL_SCANCODE_DOWN);
        map(Keypad1Left, SDL_SCANCODE_LEFT);
        map(Keypad1Right, SDL_SCANCODE_RIGHT);
        map(Keypad2Up, SDL_SCANCODE_KP_8);
        map(Keypad2Down, SDL_SCANCODE_KP_2);
        map(Keypad2Left, SDL_SCANCODE_KP_4);
        map(Keypad2Right, SDL_SCANCODE_KP_6);
        map(A, SDL_SCANCODE_Z);
        map(B, SDL_SCANCODE_X);
        map(Z, SDL_SCANCODE_C);
        map(Start, SDL_SCANCODE_RETURN);
        map(Select, SDL_SCANCODE_SPACE);
        map(Analog1Left, SDL_SCANCODE_A);
        map(Analog1Right, SDL_SCANCODE_D);
        map(Analog1Up, SDL_SCANCODE_W);
        map(Analog1Down, SDL_SCANCODE_S);
        map(Analog2Left, SDL_SCANCODE_J);
        map(Analog2Right, SDL_SCANCODE_L);
        map(Analog2Up, SDL_SCANCODE_I);
        map(Analog2Down, SDL_SCANCODE_K);
        map(X, SDL_SCANCODE_Q);
        map(Y, SDL_SCANCODE_E);
        map(L1, SDL_SCANCODE_1);
        map(R1, SDL_SCANCODE_2);
        map(L2, SDL_SCANCODE_3);
        map(R2, SDL_SCANCODE_4);
        map(L3, SDL_SCANCODE_5);
        map(R3, SDL_SCANCODE_6);
#undef map
        Add("Default mappings", replacement_mappings);
    }

    void Input::Poll()
    {
        if (!keyboardState)
            keyboardState = SDL_GetKeyboardState(&keyboardStateSize);
    }

    int32_t Input::Check(uint32_t player, hydra::ButtonType button)
    {
        if (playerMappings.size() <= player || playerMappings[player] == nullptr)
        {
            printf("Returning 0 %d %p\n", player, playerMappings[player]);
            return 0;
        }

        return keyboardState[playerMappings[player]->mappings[(int)button]];
    }

    bool Input::Add(const std::string& name, const KeyMappings& mapping)
    {
        if (mappings.find(name) != mappings.end())
        {
            return false;
        }

        mappings[name] = mapping;
        return true;
    }

    bool Input::Remove(const std::string& name)
    {
        if (mappings.find(name) == mappings.end())
        {
            return false;
        }

        mappings.erase(name);
        return true;
    }

    bool Input::IsAssociated(const std::string& name)
    {
        return mappings.find(name) != mappings.end();
    }

    bool Input::Associate(uint32_t player, const std::string& name)
    {
        if (mappings.find(name) == mappings.end())
        {
            return false;
        }

        if (playerMappings.size() <= player)
        {
            playerMappings.resize(player + 1);
        }

        playerMappings[player] = &mappings[name];
        return true;
    }

    void Input::Unassociate()
    {
        playerMappings.clear();
    }

    const std::vector<std::string>& Input::GetMappingNames()
    {
        if (mappingNamesDirty)
        {
            mappingNamesDirty = false;
            mappingNames.clear();
            mappingNames.reserve(mappings.size());

            for (auto& [name, _] : mappings)
            {
                mappingNames.push_back(name);
            }
        }

        return mappingNames;
    }

    KeyMappings& Input::GetMapping(const std::string& name)
    {
        return mappings[name];
    }

} // namespace hydra