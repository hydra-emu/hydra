#include <settings.hxx>

std::vector<core_info> Settings::CoreInfo;
std::map<std::string, std::string> Settings::map_;
std::string Settings::save_path_;
bool Settings::initialized_ = false;
bool Settings::core_info_initialized_ = false;