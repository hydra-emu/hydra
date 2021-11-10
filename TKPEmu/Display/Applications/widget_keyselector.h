#pragma once
#ifndef TKP_WIDGET_KEYSELECTOR_H
#define TKP_WIDGET_KEYSELECTOR_H
#include <SDL2/SDL.h>
#include <string>
#include <memory>
#include "../ImGui/imgui.h"
struct KeySelector {
    void Draw(std::string&& key_name, SDL_Keycode& key_code, SDL_Keycode* new_key_press = nullptr) {
        static std::string label = "##" + key_name;
        static std::string buffer = SDL_GetKeyName(key_code);
        static bool buffer_changed = false;
        static bool selected = false;
        static bool was_selected = false;
        if (!key_name.empty()) {
            ImGui::TextUnformatted(key_name.c_str());
            ImGui::SameLine();
        }
        ImGui::InputText(label.c_str(), buffer.data(), buffer.size());
        if (ImGui::IsItemActive()) {
            selected = true;
            was_selected = true;
        } else {
            selected = false;
            if (was_selected) {
                buffer = SDL_GetKeyName(key_code);
                was_selected = false;
            }
        }
        if (selected && !buffer_changed) {
            buffer = "Press a new key...";
            if (new_key_press != nullptr) {
                key_code = *new_key_press;
                buffer_changed = true;
            }
        }
    }
};
#endif