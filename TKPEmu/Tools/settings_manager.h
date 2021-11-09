#pragma once
#ifndef TKP_TOOLS_SETTINGSMANAGER_H
#define TKP_TOOLS_SETTINGSMANAGER_H
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <fstream>
#include <unordered_map>
namespace TKPEmu::Tools {
	using SettingsMap = std::unordered_map<std::string, std::string>;
	class SettingsManager {
	private:
		std::string config_file_;
		SettingsMap& settings_;
		boost::property_tree::ptree ptree_;
		// Declared noexcept to terminate the program if this throws
		void save_settings() noexcept;
	public:
		SettingsManager(SettingsMap& settings_, std::string config_file);
		~SettingsManager();
	};
}
#endif