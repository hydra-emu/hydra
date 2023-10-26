#pragma once

#include "json.hpp"
#include "settings.hxx"
#include <download.hxx>
#include <miniz/miniz.h>
#include <mutex>
#include <sstream>
#include <thread>

namespace hydra
{
    struct Updater
    {
        enum UpdateStatus
        {
            Error,
            UpToDate,
            UpdateAvailable,
            NoConnection,
        };

        static std::mutex& GetMutex()
        {
            static std::mutex mutex;
            return mutex;
        }

        static UpdateStatus NeedsDatabaseUpdate()
        {
            std::string old_date = Settings::Get("database_date");
            std::string new_date = get_database_time();
            if (new_date.empty())
                return NoConnection;
            if (old_date.empty())
                return UpdateAvailable;

            return is_newer_date(old_date, new_date) ? UpdateAvailable : UpToDate;
        }

        static UpdateStatus NeedsCoreUpdate(const std::string& core_name)
        {
            if (Settings::Get(core_name + "_date").empty())
                return UpdateAvailable;

            std::mutex& mutex = GetMutex();
            std::lock_guard<std::mutex> lock(mutex);

            std::filesystem::path database_path =
                Settings::GetSavePath() / "database" / (core_name + ".json");
            if (!std::filesystem::exists(database_path))
                return Error;

            nlohmann::json database;
            std::ifstream file(database_path);
            file >> database;

            std::string versioning = database["Versioning"];
            std::string versioning_url = database["VersioningURL"];
            printf("Versioning: %s\n", versioning_url.c_str());

            if (versioning == "Github")
            {
                HydraBufferWrapper buffer = Downloader::Download(versioning_url);
                if (buffer.empty())
                    return Error;

                nlohmann::json versioning_json = nlohmann::json::parse((char*)buffer.data());
                std::string last_date = versioning_json["commit"]["commit"]["committer"]["date"];
                std::string old_date = Settings::Get(core_name + "_date");

                return is_newer_date(old_date, last_date) ? UpdateAvailable : UpToDate;
            }
            else
            {
                log_fatal("Unknown versioning type");
            }

            return Error;
        }

        static void UpdateDatabase(std::function<void()> callback)
        {
            std::thread t([callback]() {
                std::mutex& mutex = GetMutex();
                std::lock_guard<std::mutex> lock(mutex);
                HydraBufferWrapper buffer = Downloader::Download(
                    "https://github.com/hydra-emu/database/archive/refs/heads/master.zip");

                if (buffer.empty())
                {
                    printf("Failed to download database. No internet connection?\n");
                    return;
                }

                mz_zip_archive zip_archive;
                memset(&zip_archive, 0, sizeof(zip_archive));

                if (!mz_zip_reader_init_mem(&zip_archive, buffer.data(), buffer.size(), 0))
                    log_fatal("Failed to read database zip");

                if (!std::filesystem::create_directories(Settings::GetSavePath() / "database"))
                {
                    if (!std::filesystem::exists(Settings::GetSavePath() / "database"))
                        log_fatal("Failed to create database directory");
                }

                for (size_t i = 0; i < mz_zip_reader_get_num_files(&zip_archive); i++)
                {
                    mz_zip_archive_file_stat file_stat;
                    if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
                        log_fatal("Failed to stat file in zip");

                    std::filesystem::path path = file_stat.m_filename;
                    if (path.extension() == ".json")
                    {
                        std::string data;
                        data.resize(file_stat.m_uncomp_size);
                        if (!mz_zip_reader_extract_to_mem(&zip_archive, i, data.data(), data.size(),
                                                          0))
                            log_fatal("Failed to extract file from zip");

                        std::ofstream file(Settings::GetSavePath() / "database" / path.filename());
                        file << data;
                    }
                }
                Settings::Set("database_date", get_database_time());
                callback();
            });
            t.detach();
        }

    private:
        static std::string get_database_time()
        {
            HydraBufferWrapper result = Downloader::Download(
                "https://api.github.com/repos/hydra-emu/database/commits/master");
            if (result.empty())
                return std::string();

            std::string data = std::string((char*)result.data(), result.size());
            auto json = nlohmann::json::parse(data);
            std::string date = json["commit"]["committer"]["date"];
            return date;
        }

        static bool is_newer_date(const std::string& date_old, const std::string& date_new)
        {
            std::istringstream ss_old(date_old);
            std::istringstream ss_new(date_new);

            std::tm timeinfo_old = {};
            std::tm timeinfo_new = {};

            ss_old >> std::get_time(&timeinfo_old, "%Y-%m-%dT%H:%M:%SZ");
            ss_new >> std::get_time(&timeinfo_new, "%Y-%m-%dT%H:%M:%SZ");

            std::time_t time_old = std::mktime(&timeinfo_old);
            std::time_t time_new = std::mktime(&timeinfo_new);

            return time_new > time_old;
        }
    };

} // namespace hydra