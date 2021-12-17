#include "../include/settings_manager.h"
#include <filesystem>
#include <iostream>
namespace TKPEmu::Tools {
	SettingsManager::SettingsManager(SettingsMap& settings_, std::string config_file) :
		config_file_(config_file),
		settings_(settings_)
	{
		try {
			// ini_parser::read_ini throws if file doesn't exist, so we create it
			std::string directory;
			directory = std::filesystem::current_path();
			directory += "/Resources/Data/";
			if (!std::filesystem::exists(directory)) {
				std::cout << "Creating " << directory << " directories..." << std::endl;
				std::filesystem::create_directories(directory);
			}
			config_file_ = directory + config_file_;
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
			std::cout << "Error opening user .ini file" << std::endl;
			throw ex;
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
