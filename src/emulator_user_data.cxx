#include <emulator_factory.hxx>
#include <emulator_user_data.hxx>
#include <error_factory.hxx>
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

EmulatorUserData::EmulatorUserData(std::string save_path)
    : save_path_(save_path), map_(), mutex_(std::make_shared<std::mutex>())
{
    std::ifstream ifs(save_path_);
    if (ifs.good())
    {
        json j_map;
        ifs >> j_map;
        map_ = j_map.get<std::map<std::string, std::string>>();
    }
    initialized_ = true;
}

std::string EmulatorUserData::Get(const std::string& key) const
{
    if (!initialized_)
    {
        throw ErrorFactory::generate_exception(
            __func__, __LINE__, "EmulatorUserData not initialized for path " + save_path_);
    }

    std::lock_guard lg(*mutex_);
    try
    {
        return map_.at(key);
    } catch (std::exception& ex)
    {
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to get: " + key);
    }
}

bool EmulatorUserData::Has(const std::string& key) const
{
    if (!initialized_)
    {
        throw ErrorFactory::generate_exception(
            __func__, __LINE__, "EmulatorUserData not initialized for path " + save_path_);
    }

    std::lock_guard lg(*mutex_);
    return map_.find(key) != map_.end();
}

bool EmulatorUserData::IsEmpty() const
{
    if (!initialized_)
    {
        throw ErrorFactory::generate_exception(
            __func__, __LINE__, "EmulatorUserData not initialized for path " + save_path_);
    }

    std::lock_guard lg(*mutex_);
    return map_.empty();
}

void EmulatorUserData::Set(const std::string& key, const std::string& value)
{
    if (!initialized_)
    {
        throw ErrorFactory::generate_exception(
            __func__, __LINE__, "EmulatorUserData not initialized for path " + save_path_);
    }

    std::lock_guard lg(*mutex_);
    map_[key] = value;
    std::ofstream ofs(save_path_, std::ios::trunc);
    json j_map(map_);
    ofs << j_map << std::endl;
}