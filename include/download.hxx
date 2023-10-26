#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <memory>
#include <string>

namespace hydra
{
    using HydraBufferWrapper = std::vector<uint8_t>;

    inline std::pair<std::string, std::string> split_url(const std::string& url)
    {
        std::regex path_regex("https://(.*?)(/.*)");
        std::string host = "https://", query;
        std::smatch match;
        if (std::regex_search(url, match, path_regex) && match.size() == 3)
        {
            host += match[1];
            query = match[2];
        }
        else
        {
            printf("[rcheevos]: failed to parse URL: %s\n", url.c_str());
        }
        return std::make_pair(host, query);
    }

    struct Downloader
    {
        static HydraBufferWrapper Download(const std::string& url)
        {
            try
            {
                auto [host, query] = split_url(url);
                httplib::Client client(host);
                client.set_follow_location(true);
                auto response = client.Get(query);

                if (!response)
                {
                    printf("Null response when downloading %s\n", url.c_str());
                }
                else if (response->status != 200)
                {
                    printf("Failed to download %s: %d\n", url.c_str(), response->status);
                }

                return {response->body.begin(), response->body.end()};
            } catch (...)
            {
                printf("Failed while trying to download %s\n", url.c_str());
                return {};
            }
        }
    };
} // namespace hydra