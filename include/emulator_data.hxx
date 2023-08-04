#pragma once

#include "emulator_types.hxx"
#include "error_factory.hxx"
#include "json.hpp"
#include <array>
#include <emulator_user_data.hxx>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
using json = nlohmann::json;
using KeyMappings = std::map<std::string, std::string>;

struct EmulatorData
{
    std::string Name;
    std::string SettingsFile;
    std::vector<std::string> Extensions;
    bool HasDebugger;
    // TODO: remove HasTracelogger, should be active for every emulator
    bool HasTracelogger;
    std::vector<std::string> LoggingOptions;
    KeyMappings Mappings;
    EmulatorUserData UserData;
};
