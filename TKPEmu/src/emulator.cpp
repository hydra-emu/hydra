#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <filesystem>
#include <iostream>
#include <fstream>
#include <include/emulator.h>
#include <lib/stb_image_write.h>
#include <lib/md5.h>
#include <GL/glew.h>
#include <include/settings_manager.h>
#include <include/error_factory.hxx>

namespace {
	std::ifstream::pos_type filesize(const char* filename) {
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        return in.tellg(); 
    }
}

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
    void Emulator::Screenshot(std::string filename, std::string directory) { 
		std::lock_guard<std::mutex> lg(DrawMutex);
		// std::string scrnshot_dir = TKPEmu::Tools::SettingsManager::GetSavePath() + "/screenshots/";
		std::string scrnshot_dir;
		if (directory.empty()) {
			scrnshot_dir = std::filesystem::current_path().string();
		} else {
			scrnshot_dir = directory;
		}
		if (!std::filesystem::is_directory(scrnshot_dir)) {
			std::filesystem::create_directories(scrnshot_dir);
		}
		int index = 0;
		std::string filename_final = scrnshot_dir + "/" + filename;
		std::vector<uint8_t> data;
		if (start_options == EmuStartOptions::Console && false /* TODO: weird needed for .gb server, otherwise blank image. investigate. false for now */) {
			data = std::vector<uint8_t>((float*)GetScreenData(), (float*)GetScreenData() + EmulatorImage.width * EmulatorImage.height * 4);
		} else {
			data.resize(EmulatorImage.width * EmulatorImage.height * 4);
			float* fl_ptr = (float*)GetScreenData();
			for (int i = 0; i < (EmulatorImage.width * EmulatorImage.height * 4); i++) {
				if ((i & 0b11) != 0b11)
					data[i] = static_cast<uint8_t>(fl_ptr[i] * 255.0f);
				else
					data[i] = 0xFF; // stbi doesnt take float as input so we have to multiply with 255 to convert back to
					// uint8_t. But alpha needs no conversion because its already 255 and if its multiplied by 255 again
					// we get 0 cus of overflow
			}
		}
		try {
			stbi_write_bmp(filename_final.c_str(), EmulatorImage.width, EmulatorImage.height, 4, data.data());
		} catch (const std::exception& e) {
			throw ErrorFactory::generate_exception(__func__, __LINE__, e.what());
		}
    }
	std::string Emulator::GetScreenshotHash() {
		return "Warning: GetScreenshotHash not implemented, hash not printed";
	}
    void* Emulator::GetScreenData() { 
        throw ErrorFactory::generate_exception(__func__, __LINE__, "GetScreenData was not implemented for this emulator");
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
	bool Emulator::LoadFromFile(std::string path) {
		CurrentFilename = std::filesystem::path(path).filename().stem();
		CurrentDirectory = std::filesystem::path(path).parent_path();
		std::ifstream t(path, std::ios::in | std::ios::binary);
		std::stringstream buffer;
		buffer << t.rdbuf();
		rom_data_.clear();
		rom_size_ = filesize(path.c_str());
		rom_data_.resize(rom_size_);
		t.seekg(std::ios_base::beg);
		t.read((char*)rom_data_.data(), rom_size_);
		return load_file(path);
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
		v_extra_close();
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
	std::vector<std::string> Emulator::Disassemble(std::string instr) {
		throw ErrorFactory::generate_exception(__func__, __LINE__, "Disassemble was not implemented for this emulator");
	}
	std::string Emulator::GetEmulatorName() {
		throw ErrorFactory::generate_exception(__func__, __LINE__, "GetEmulatorName was not implemented for this emulator");
	}
	void Emulator::v_log_state() {
		throw ErrorFactory::generate_exception(__func__, __LINE__, "log_state was not implemented for this emulator");
	}
    void Emulator::start_normal() {
        throw ErrorFactory::generate_exception(__func__, __LINE__, "start_normal was not implemented for this emulator");
    }
    void Emulator::start_debug() {
        throw ErrorFactory::generate_exception(__func__, __LINE__, "start_debug was not implemented for this emulator");
    }
	void Emulator::start_console() {
		throw ErrorFactory::generate_exception(__func__, __LINE__, "start_console was not implemented for this emulator");
	}
	void Emulator::reset_normal() {
		throw ErrorFactory::generate_exception(__func__, __LINE__, "reset_normal was not implemented for this emulator");
	}
	void Emulator::reset_skip() {
		throw ErrorFactory::generate_exception(__func__, __LINE__, "reset_skip was not implemented for this emulator");
	}
    void Emulator::update() {
        throw ErrorFactory::generate_exception(__func__, __LINE__, "update was not implemented for this emulator");
    }
	bool Emulator::load_file(std::string) {
		throw ErrorFactory::generate_exception(__func__, __LINE__, "load_file was not implemented for this emulator");
	}
	void Emulator::save_state(std::ofstream&) {
		throw ErrorFactory::generate_exception(__func__, __LINE__, "save_state was not implemented for this emulator");
	}
	void Emulator::load_state(std::ifstream&) {
		throw ErrorFactory::generate_exception(__func__, __LINE__, "load_state was not implemented for this emulator");
	}
    std::string Emulator::print() const { 
		throw ErrorFactory::generate_exception(__func__, __LINE__, "print was not implemented for this emulator");
    }
	void Emulator::WS_SetActionPtr(int* action_ptr) {
		action_ptr_ = action_ptr;
	}
    std::ostream& operator<<(std::ostream& os, const Emulator& obj) {
    	return os << obj.print();
	}
}