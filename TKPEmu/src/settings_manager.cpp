#include <include/settings_manager.h>
#include <filesystem>
#include <iostream>
#include <include/error_factory.hxx>

namespace TKPEmu::Tools {
	SettingsManager::SettingsManager(SettingsMap& settings, std::string config_file) :
		config_file_(config_file),
		settings_(settings)
	{
		try {
			std::string save_dir_ = GetSavePath();
			if (!std::filesystem::exists(save_dir_)) {
				// TODO: test that this actually works
				std::cout << "Creating " << save_dir_ << " directories..." << std::endl;
				std::filesystem::create_directories(save_dir_);
			}
			config_file_ = save_dir_ + config_file_;
			if (!std::filesystem::exists(config_file_)) {
				std::cout << "User settings not found. Loading default settings..." << std::endl;
				std::fstream temp;
				temp.open(config_file_, std::fstream::in | std::fstream::out | std::fstream::trunc);
				temp.close();
			} else {
				boost::property_tree::ini_parser::read_ini(config_file_, ptree_);
			}
		}
		catch (std::exception& ex) {
			// Ini file doesn't exist or is inaccessible
			std::cout << "Error opening user .ini file\n" << ex.what() << std::endl;
			return;
		}
		for (auto& item : settings_) {
			item.second = ptree_.get(item.first, item.second);
		}
	}
	SettingsManager::~SettingsManager() {
		save_settings();
	}
	std::string SettingsManager::GetSavePath() {
		static std::string dir;
		if (dir.empty()) {
			#if defined(__linux__)
			dir = getenv("HOME") + std::string("/.config/tkpemu");
			#elif defined(_WIN32)
			dir = getenv("APPDATA") + std::string("/tkpemu");
			#endif
			if (dir.empty()) {
				throw ErrorFactory::generate_exception(__func__, __LINE__, "GetSavePath was not defined for this environment");
			}
		}
		return dir;
	}
	void SettingsManager::save_settings() noexcept {
		for (auto& item : settings_) {
			ptree_.put(item.first, item.second);
		}
		boost::property_tree::ini_parser::write_ini(config_file_, ptree_);
	}
}
