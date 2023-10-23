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
        static std::mutex& GetMutex()
        {
            static std::mutex mutex;
            return mutex;
        }

        static bool NeedsDatabaseUpdate()
        {
            std::string old_date = Settings::Get("database_date");
            if (old_date.empty())
                return true;

            std::string new_date = get_database_time();
            return is_newer_date(old_date, new_date);
        }

        static void UpdateDatabase()
        {
            std::thread t([]() {
                std::mutex& mutex = GetMutex();
                std::lock_guard<std::mutex> lock(mutex);
                HydraBufferWrapper buffer = Downloader::Download(
                    "https://github.com/hydra-emu/database/archive/refs/heads/master.zip");

                if (!buffer->data)
                {
                    log_fatal("Failed to download database zip\n");
                }

                mz_zip_archive zip_archive;
                memset(&zip_archive, 0, sizeof(zip_archive));

                if (!mz_zip_reader_init_mem(&zip_archive, buffer->data, buffer->size, 0))
                    log_fatal("Failed to read database zip\n");

                if (!std::filesystem::create_directories(Settings::GetSavePath() / "database"))
                {
                    if (!std::filesystem::exists(Settings::GetSavePath() / "database"))
                        log_fatal("Failed to create database directory\n");
                }

                for (size_t i = 0; i < mz_zip_reader_get_num_files(&zip_archive); i++)
                {
                    mz_zip_archive_file_stat file_stat;
                    if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
                        log_fatal("Failed to stat file in zip\n");

                    std::filesystem::path path = file_stat.m_filename;
                    if (path.extension() == ".json")
                    {
                        std::string data;
                        data.resize(file_stat.m_uncomp_size);
                        if (!mz_zip_reader_extract_to_mem(&zip_archive, i, data.data(), data.size(),
                                                          0))
                            log_fatal("Failed to extract file from zip\n");

                        std::ofstream file(Settings::GetSavePath() / "database" / path.filename());
                        file << data;
                    }
                }
            });
            t.detach();
        }

    private:
        static std::string get_database_time()
        {
            HydraBufferWrapper result = Downloader::Download(
                "https://api.github.com/repos/hydra-emu/database/commits/master");

            if (result->data)
                log_fatal("Failed to download database info\n");

            std::string data = std::string((char*)result->data, result->size);
            auto json = nlohmann::json::parse(data);
            std::string date = json["commit"]["commit"]["commiter"]["date"];

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