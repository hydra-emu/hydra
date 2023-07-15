#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>

// Essentially a wrapper around a std::map<std::string, std::string> that locks a mutex
// upon access and has a save function that saves as a json to file
class EmulatorUserData
{
public:
    EmulatorUserData(){};
    EmulatorUserData(std::string save_path);
    std::string Get(const std::string& key) const;
    bool Has(const std::string& key) const;
    void Set(const std::string& key, const std::string& value);
    bool IsEmpty() const;

private:
    std::map<std::string, std::string> map_;
    std::string save_path_;
    std::shared_ptr<std::mutex> mutex_;

    bool initialized_ = false;
};
