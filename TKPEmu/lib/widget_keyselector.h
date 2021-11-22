#pragma once
#ifndef TKP_WIDGET_KEYSELECTOR_H
#define TKP_WIDGET_KEYSELECTOR_H
#include <SDL2/SDL.h>
#include <string>
#include <memory>
#include <iostream>
#include "../lib/imgui.h"
#include "../include/settings_manager.h"
namespace TKPEmu::Tools {
	struct KeySelector {
		public:
		KeySelector(std::string button_name, std::string button_key, SDL_Keycode& key_ref) : 
			button_name_(std::move(button_name)),
			button_key_(std::move(button_key)),
			old_button_text_(button_text_),
			key_ref_(key_ref)
    	{
			button_text_ += "##" + button_key_;
			if (settings_map_ != nullptr) 
				key_ref_ = std::stoi(settings_map_->at(button_key_));
			else
				throw("KeySelector::settings_map_ is not initialized. Run Initialize() first!");
			button_text_ = SDL_GetKeyName(key_ref);
		};
		static void Initialize(SettingsMap* settings_map);
	    void Draw(SDL_Keycode& new_key_press);
		private:
		std::string button_name_;
		std::string button_text_;
		std::string button_key_;
		SDL_Keycode old_keycode_ = 0;
		std::string old_button_text_;
		bool select_key_mode = false;
		SDL_Keycode& key_ref_;
		
		static bool lock_key_mode_;
		static SettingsMap* settings_map_;
	};
}
#endif
