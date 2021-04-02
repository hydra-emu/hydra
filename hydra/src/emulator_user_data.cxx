#include <include/emulator_user_data.hxx>
#include <include/error_factory.hxx>
#include <fstream>
#include <include/json.hpp>
using json = nlohmann::json;

EmulatorUserData::EmulatorUserData(std::string path, std::map<std::string, std::string> map) :
save_path_(path),
map_(std::move(map)),
mutex_(std::make_unique<std::mutex>())
{}

void EmulatorUserData::Save() {
    std::lock_guard lg(*mutex_);
    std::ofstream ofs(save_path_, std::ios::trunc);
    json j_map(map_);
    ofs << j_map << std::endl;
    ofs.close();
}

std::string EmulatorUserData::Get(const std::string& key) const {
    std::lock_guard lg(*mutex_);
    try {
        return map_.at(key);
    } catch (std::exception& ex) {
        throw ErrorFactory::generate_exception(__func__, __LINE__, "Failed to get: " + key);
    }
}

bool EmulatorUserData::IsEmpty() const {
    return map_.empty();
}

void EmulatorUserData::Set(const std::string& key, const std::string& value) {
    std::lock_guard lg(*mutex_);
    map_[key] = value;
}