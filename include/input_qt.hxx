#pragma once

#include "error_factory.hxx"
#include <filesystem>
#include <fstream>
#include <hydra/core.hxx>
#include <json.hpp>
#include <map>
#include <QKeySequence>
#include <string>
#include <unordered_map>

namespace hydra
{

    using KeyMappings = std::array<QKeySequence, (int)hydra::ButtonType::InputCount>;

    class Input
    {
        using json = nlohmann::json;
        Input() = delete;
        ~Input() = delete;

    public:
        static QKeySequence StringToKey(const std::string& str)
        {
            if (str.empty())
                return QKeySequence();
            QKeySequence key(str.c_str());
            if (key.isEmpty())
            {
                throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                       "Invalid key sequence: " + str);
            }
            return key;
        }

        static std::string KeyToString(const QKeySequence& key)
        {
            return key.toString().toStdString();
        }

        static KeyMappings Load(const std::string& data)
        {
            KeyMappings mappings;
            std::map<std::string, std::string> map;
            json j_map = json::parse(data);
            map = j_map.get<std::map<std::string, std::string>>();

            for (auto& [key, value] : map)
            {
                int button_type = 0;

                try
                {
                    button_type = std::stoi(key);
                } catch (std::exception& e)
                {
                    throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                           "Invalid button type");
                }
                if (button_type < 0 || button_type >= (int)hydra::ButtonType::InputCount)
                {
                    throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                           "Invalid button range");
                }

                mappings[button_type] = StringToKey(value);
            }

            return mappings;
        }

        static KeyMappings Open(const std::filesystem::path& path)
        {
            if (!std::filesystem::exists(path))
            {
                throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                       "Input file does not exist");
            }

            std::ifstream ifs(path);
            if (!ifs.good())
            {
                throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                       "Failed to open input file");
            }

            return Load(std::string((std::istreambuf_iterator<char>(ifs)),
                                    std::istreambuf_iterator<char>()));
        }

        static void Save(const std::filesystem::path& path, const KeyMappings& mappings)
        {
            std::map<std::string, std::string> map;

            for (int key = 0; key < (int)hydra::ButtonType::InputCount; ++key)
            {
                map[std::to_string(key)] = KeyToString(mappings[key]);
            }

            if (!std::filesystem::create_directories(path.parent_path()))
            {
                if (!std::filesystem::exists(path.parent_path()))
                {
                    throw ErrorFactory::generate_exception(__func__, __LINE__,
                                                           "Failed to create input directory");
                }
            }

            json j_map = map;
            std::ofstream ofs(path);
            ofs << j_map.dump(4);
        }

        static std::vector<std::filesystem::path> Scan(const std::filesystem::path& root,
                                                       const std::string& ext)
        {
            std::vector<std::filesystem::path> paths;

            if (std::filesystem::exists(root) && std::filesystem::is_directory(root))
            {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
                {
                    if (std::filesystem::is_regular_file(entry) && entry.path().extension() == ext)
                        paths.emplace_back(entry.path());
                }
            }

            return paths;
        }
    };

} // namespace hydra