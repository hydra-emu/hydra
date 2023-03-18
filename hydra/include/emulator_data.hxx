#pragma once
#ifndef TKP_EMULATOR_DATA_H
#define TKP_EMULATOR_DATA_H
#include <string>
#include <array>
#include <filesystem>
#include <memory>
#include <fstream>
#include <map>
#include <include/emulator_user_data.hxx>
#include "emulator_types.hxx"
#include "error_factory.hxx"
#include "json.hpp"
using json = nlohmann::json;
using KeyMappings = std::map<std::string, std::string>;
struct EmulatorData {
    std::string Name;
    std::string SettingsFile;
    std::vector<std::string> Extensions;
    int DefaultWidth;
    int DefaultHeight;
    bool HasDebugger;
    bool HasTracelogger;
    std::vector<std::string> LoggingOptions;
    KeyMappings Mappings;
    EmulatorUserData UserData;
};
#endif