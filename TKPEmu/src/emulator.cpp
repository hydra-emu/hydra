#include <filesystem>
#include <iostream>
#include <fstream>
#include "../include/emulator.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"
#include "../lib/md5.h"
#include "../glad/glad/glad.h"
namespace TKPEmu {
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
    }
	std::string Emulator::GetScreenshotHash() {
		return "Warning: GetScreenshotHash not implemented, hash not printed";
	}
	void Emulator::limit_fps() const {
		if (!FastMode) {
			a = std::chrono::system_clock::now();
			std::chrono::duration<double, std::milli> work_time = a - b;
			if (work_time.count() < sleep_time_) {
				std::chrono::duration<double, std::milli> delta_ms(sleep_time_ - work_time.count());
				auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
				std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
			}
			b = std::chrono::system_clock::now();
		}
	}
    float* Emulator::GetScreenData() { 
        std::cerr << "GetScreenData was not implemented for this emulator" << std::endl;
		exit(1);
    }
    void Emulator::Start(EmuStartOptions start_mode) { 
        switch (start_mode) {
			case EmuStartOptions::Normal: {
				start_normal();
				break;
			}
			case EmuStartOptions::Console: {
				FastMode = true;
				[[fallthrough]];
			}
			case EmuStartOptions::Debug: {
				start_debug();
				break;
			}
		}
    }
	void Emulator::LoadFromFile(std::string path) {
		std::ifstream t(path);
		std::stringstream buffer;
		buffer << t.rdbuf();
		rom_hash_ = md5(buffer.str());
		std::cout << "Loading " << path << " with hash " << rom_hash_ << std::endl;
		load_file(path);
	}
    void Emulator::Reset() {
        if (SkipBoot) {
			reset_skip();
		} else {
			reset_normal();
		}
    }
	void Emulator::ResetState() {
		Step.store(true);
        Paused.store(false);
        Stopped.store(true);
        Step.notify_all();
	}
	void Emulator::StartLogging(std::string filename) {
		log_filename_ = std::move(filename);
		logging_ = true;
	}
	void Emulator::StopLogging() {
		logging_ = false;
	}
	void Emulator::CloseAndWait() {
		Stopped = true;
        Paused = false;
        Step = true;
        Step.notify_all();
        std::lock_guard<std::mutex> lguard(ThreadStartedMutex);
	}
	void Emulator::log_state() {
		if (logging_) {
			// We initialize it here to avoid race conditions
			if (ofstream_ptr_ == nullptr)
				ofstream_ptr_ = std::make_unique<std::ofstream>(log_filename_.c_str(), std::ofstream::out | std::ofstream::trunc);
			v_log_state();
		} else {
			if (ofstream_ptr_ != nullptr)
				ofstream_ptr_.reset(nullptr);
		}
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
	void Emulator::load_file(std::string path) {
		std::cerr << "load_file was not implemented for this emulator" << std::endl;
		exit(1);
	}
    std::string Emulator::print() const { 
        return "Error: Override print function for this emulator";
    }
    std::ostream& operator<<(std::ostream& os, const Emulator& obj) {
    	return os << obj.print();
	}
}