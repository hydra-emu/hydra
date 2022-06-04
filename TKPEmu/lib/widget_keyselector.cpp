#include "widget_keyselector.h"	
namespace TKPEmu::Tools {
    bool KeySelector::lock_key_mode_ = false;
    SettingsMap* KeySelector::settings_map_ = nullptr;
    void KeySelector::Initialize(SettingsMap* settings_map) {
        settings_map_ = settings_map;
    }
    void KeySelector::Draw(SDL_Keycode& new_key_press) {
        ImGui::Text(button_name_.c_str());
        ImGui::SameLine();
        if (ImGui::Button(button_text_.c_str(), ImVec2(50, 0))) {
            if (!select_key_mode) {
                if (!lock_key_mode_) {
                    old_button_text_ = std::move(button_text_);
                    button_text_ = "Press a button...";
                    select_key_mode = true;
                    lock_key_mode_ = true;
                }
            } else {
                key_ref_ = old_keycode_;
                button_text_ = std::move(old_button_text_) + "##" + button_key_;
                select_key_mode = false;
                lock_key_mode_ = false;
            }
        }
        if (select_key_mode) {
            if (new_key_press != 0) {
                key_ref_ = new_key_press;
                new_key_press = 0;
                old_button_text_ = std::move(old_button_text_);
                button_text_ = std::string(SDL_GetKeyName(key_ref_)) + "##" + button_key_;
                settings_map_->at(button_key_) = std::to_string(key_ref_);
                select_key_mode = false;
                lock_key_mode_ = false;
            }
        }
    }
}