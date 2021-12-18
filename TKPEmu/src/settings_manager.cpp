#include "../include/settings_manager.h"
#include <filesystem>
#include <iostream>
namespace TKPEmu::Tools {
	SettingsManager::SettingsManager(SettingsMap& settings, std::string config_file) :
		config_file_(config_file),
		settings_(settings)
	{
		try {
			#if defined(__linux__)
			SaveDataDir = getenv("HOME") + std::string("/.config/tkpemu/");
			#elif defined(_WIN32)
			SaveDataDir = getenv("APPDATA") + std::string("/TKPEmu/");
			#endif
			if (SaveDataDir.empty()) {
				std::cerr << "SaveDataDir was not defined for this environment - settings_manager.cpp" << std::endl;
				exit(1);
			}
			if (!std::filesystem::exists(SaveDataDir)) {
				std::cout << "Creating " << SaveDataDir << " directories..." << std::endl;
				std::filesystem::create_directories(SaveDataDir);
			}
			config_file_ = SaveDataDir + config_file_;
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

	void SettingsManager::save_settings() noexcept {
		for (auto& item : settings_) {
			ptree_.put(item.first, item.second);
		}
		boost::property_tree::ini_parser::write_ini(config_file_, ptree_);
	}
}
