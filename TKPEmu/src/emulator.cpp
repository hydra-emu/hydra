#include <filesystem>
#include <iostream>
#include "../include/emulator.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"
#include "../glad/glad/glad.h"
namespace TKPEmu {
    constexpr int Emulator::GetPCHexCharSize() { 
		return 1; 
	}
    void Emulator::HandleKeyDown(SDL_Keycode keycode) { 
        std::cout << "Warning: Key " << SDL_GetKeyName(keycode) << " was pressed but\n"
        "emulator.HandleKeyDown was not implemented" << std::endl;
    }
    void Emulator::HandleKeyUp(SDL_Keycode keycode) { 
        std::cout << "Warning: Key " << SDL_GetKeyName(keycode) << " was released but\n"
        "emulator.HandleKeyUp was not implemented" << std::endl;
    }
    void Emulator::Screenshot(std::string filename) { 
		std::lock_guard<std::mutex> lg(DrawMutex);
		static const std::string scrnshot_dir = std::string(std::filesystem::current_path()) + "/Resources/Screenshots/";
		if (!std::filesystem::is_directory(scrnshot_dir)) {
			std::filesystem::create_directories(scrnshot_dir);
		}
		int index = 0;
		std::string filename_final;
		do {
			// Find available screenshot name
			filename_final = scrnshot_dir + filename + "_" + std::to_string(++index) + ".bmp";
			if (!std::filesystem::exists(filename_final)) {
				break;
			}
			if (index > 200){
				std::cerr << "Failed to take screenshot: more than 200 screenshots detected. Delete or move some from " << scrnshot_dir << std::endl;
				return;
			}
		} while (true);
		auto pixels = std::make_unique<uint8_t[]>(EmulatorImage.width * EmulatorImage.height * 4);
		glBindTexture(GL_TEXTURE_2D, EmulatorImage.texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
		glBindTexture(GL_TEXTURE_2D, 0);
		try {
			stbi_write_bmp(filename_final.c_str(), EmulatorImage.width, EmulatorImage.height, 3, pixels.get());
		} catch (const std::exception& e) {
			std::cerr << "Error writing screenshot: " << e.what() << std::endl;
		}
    };
    float* Emulator::GetScreenData() { 
        throw("GetScreenData was not implemented for this emulator");
    };
    void Emulator::Start(EmuStartOptions start_mode) { 
        switch (start_mode) {
			case EmuStartOptions::Normal: {
				start_normal();
				break;
			}
			case EmuStartOptions::Debug: {
				start_debug();
				break;
			}
		}
    }
    void Emulator::Reset() {
        std::cout << "Warning: Reset was not implemented for this emulator" << std::endl;
    }
    void Emulator::start_normal() {
        throw("start_normal was not implemented for this emulator");
    }
    void Emulator::start_debug() {
        throw("start_debug was not implemented for this emulator");
    }
    void Emulator::update() {
        throw("update was not implemented for this emulator");
    }
    std::string Emulator::print() const { 
        return "Error: Override print function for this emulator";
    }
    std::ostream& operator<<(std::ostream& os, const Emulator& obj) {
    	return os << obj.print();
	}
}