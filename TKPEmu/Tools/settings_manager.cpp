#include "settings_manager.h"
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#endif
namespace TKPEmu::Tools {
	SettingsManager::SettingsManager(SettingsMap& settings_, std::string config_file) : settings_(settings_), config_file_(config_file) {
		try {
			// ini_parser::read_ini throws if file doesn't exist, so we create it
			std::string directory;
			#ifdef _WIN32
			wchar_t exe_dir[MAX_PATH];
			GetModuleFileNameW(NULL, exe_dir, MAX_PATH);
			char DefChar = ' ';
			char res[260];
			WideCharToMultiByte(CP_ACP, 0, exe_dir, -1, res, MAX_PATH, &DefChar, NULL);
			directory = res;
			const size_t last_slash_idx = directory.rfind('\\');
			if (std::string::npos != last_slash_idx)
			{
				directory = directory.substr(0, last_slash_idx);
			}
			directory += "/Resources/Data/";
			#endif
			if (!std::filesystem::exists(directory)) {
				std::filesystem::create_directories(directory);
			}
			config_file_ = directory + config_file_;
			if (!std::filesystem::exists(config_file_)) {
				std::fstream temp;
				temp.open(config_file_, std::fstream::in | std::fstream::out | std::fstream::trunc);
				temp.close();
			}
			boost::property_tree::ini_parser::read_ini(config_file_, ptree_);
		}
		catch (std::exception& ex) {
			// Ini file doesn't exist or is inaccessible
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