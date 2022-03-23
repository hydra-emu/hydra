#include <filesystem>
#include <iostream>
#include <fstream>
#include "../include/emulator.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"
#include "../lib/md5.h"
#include <GL/glew.h>
#include "../include/settings_manager.h"
namespace TKPEmu {
    void Emulator::HandleKeyDown(SDL_Keycode keycode) { 
        std::cout << "Warning: Key " << SDL_GetKeyName(keycode) << " was pressed but\n"
        	"emulator.HandleKeyDown was not implemented" << std::endl;
    }
    void Emulator::HandleKeyUp(SDL_Keycode keycode) { 
        std::cout << "Warning: Key " << SDL_GetKeyName(keycode) << " was released but\n"
        	"emulator.HandleKeyUp was not implemented" << std::endl;
    }
	void Emulator::SaveState(std::string filename) {
		std::ofstream of(filename, std::ios::binary);
		save_state(of);
	}
	void Emulator::LoadState(std::string filename) {
		if (std::filesystem::exists(filename)) {
			std::ifstream ifs(filename, std::ios::in | std::ios::binary);
			load_state(ifs);
		}
	}
    void Emulator::Screenshot(std::string filename) { 
		std::lock_guard<std::mutex> lg(DrawMutex);
		// std::string scrnshot_dir = TKPEmu::Tools::SettingsManager::GetSavePath() + "/screenshots/";
		std::string scrnshot_dir = std::filesystem::current_path().string();
		if (!std::filesystem::is_directory(scrnshot_dir)) {
			std::filesystem::create_directories(scrnshot_dir);
		}
		int index = 0;
		std::string filename_final = scrnshot_dir + "/" + filename;
		std::vector<uint8_t> data(GetScreenData(), GetScreenData() + EmulatorImage.width * EmulatorImage.height * 4);
		try {
			stbi_write_bmp(filename_final.c_str(), EmulatorImage.width, EmulatorImage.height, 4, data.data());
		} catch (const std::exception& e) {
			std::cerr << "Error writing screenshot: " << e.what() << std::endl;
		}
    }
	std::string Emulator::GetScreenshotHash() {
		return "Warning: GetScreenshotHash not implemented, hash not printed";
	}
    float* Emulator::GetScreenData() { 
        std::cerr << "GetScreenData was not implemented for this emulator" << std::endl;
		exit(1);
    }
    void Emulator::Start(EmuStartOptions start_mode) { 
		start_options = start_mode;
        switch (start_mode) {
			case EmuStartOptions::Normal: {
				start_normal();
				break;
			}
			case EmuStartOptions::Console: {
				start_console();
				break;
			}
			case EmuStartOptions::Debug: {
				start_debug();
				break;
			}
		}
    }
	void Emulator::LoadFromFile(std::string path) {
		CurrentFilename = std::filesystem::path(path).filename();
		std::ifstream t(path);
		std::stringstream buffer;
		buffer << t.rdbuf();
		RomHash = md5(buffer.str());
		load_file(path);
	}
    void Emulator::Reset() {
        if (SkipBoot) {
			reset_skip();
		} else {
			reset_normal();
		}
    }
	void Emulator::StartLogging(std::string filename) {
		log_filename_ = std::move(filename);
		logging_ = true;
		log_changed_ = true;
	}
	void Emulator::StopLogging() {
		logging_ = false;
	}
	void Emulator::CloseAndWait() {
		Step.store(true);
        Paused.store(false);
        Stopped.store(true);
        Step.notify_all();
        std::lock_guard<std::mutex> lguard(ThreadStartedMutex);
	}
	void Emulator::log_state() {
		if (logging_ || (log_changed_ && logging_)) {
			// We initialize it here to avoid race conditions
			if (ofstream_ptr_ == nullptr) {
				ofstream_ptr_ = std::make_unique<std::ofstream>(log_filename_.c_str(), std::ofstream::out | std::ofstream::trunc);
				std::cout << "Logging at " << log_filename_.c_str() << std::endl;
			}
			v_log_state();
		} else {
			if (ofstream_ptr_ != nullptr)
				ofstream_ptr_.reset(nullptr);
		}
	}
	std::string Emulator::GetEmulatorName() {
		return "Unknown emulator";
	}
	void Emulator::v_log_state() {
		std::cerr << "log_state was not implemented for this emulator" << std::endl;
		exit(1);
	}
    void Emulator::start_normal() {
        std::cerr << "start_normal was not implemented for this emulator" << std::endl;
		exit(1);
    }
    void Emulator::start_debug() {
        std::cerr << "start_debug was not implemented for this emulator" << std::endl;
		exit(1);
    }
	void Emulator::start_console() {
		std::cerr << "start_console was not implemented for this emulator" << std::endl;
		exit(1);
	}
	void Emulator::reset_normal() {
		std::cerr << "reset_normal was not implemented for this emulator" << std::endl;
		exit(1);
	}
	void Emulator::reset_skip() {
		std::cerr << "reset_skip was not implemented for this emulator" << std::endl;
		exit(1);
	}
    void Emulator::update() {
        std::cerr << "update was not implemented for this emulator" << std::endl;
		exit(1);
    }
	void Emulator::load_file(std::string) {
		std::cerr << "load_file was not implemented for this emulator" << std::endl;
		exit(1);
	}
	void Emulator::save_state(std::ofstream&) {
		std::cerr << "save_state was not implemented for this emulator" << std::endl;
		exit(1);
	}
	void Emulator::load_state(std::ifstream&) {
		std::cerr << "load_state was not implemented for this emulator" << std::endl;
		exit(1);
	}
    std::string Emulator::print() const { 
        return "Error: Override print function for this emulator";
    }
	void Emulator::WS_SetActionPtr(int* action_ptr) {
		action_ptr_ = action_ptr;
	}
    std::ostream& operator<<(std::ostream& os, const Emulator& obj) {
    	return os << obj.print();
	}
}