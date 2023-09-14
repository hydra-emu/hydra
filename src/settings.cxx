#include <settings.hxx>

std::map<std::string, std::string> Settings::map_;
std::string Settings::save_path_;
bool Settings::initialized_ = false;