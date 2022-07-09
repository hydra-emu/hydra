#pragma once
#ifndef TKP_USER_DATA_H
#define TKP_USER_DATA_H
#include <map>
#include <string>
#include <mutex>
#include <memory>
// Essentially a wrapper around a std::map<std::string, std::string> that locks a mutex
// upon access and has a save function that saves as a json to file
class EmulatorUserData {
public:
    EmulatorUserData() = default;
    EmulatorUserData(std::string path, std::map<std::string, std::string> map);
    std::string Get(const std::string& key) const;
    void Set(const std::string& key, const std::string& value);
    void Save();
private:
    std::map<std::string, std::string> map_;
    std::string save_path_;
    std::unique_ptr<std::mutex> mutex_;
};
#endif