#pragma once

#include "go/downloader.h"
#include <string>

namespace hydra
{
    struct HydraBufferWrapper
    {
        HydraBufferWrapper() = default;

        HydraBufferWrapper(const hydra_buffer_t& buffer) : buffer_(buffer) {}

        ~HydraBufferWrapper()
        {
            if (buffer_.data)
                free(buffer_.data);
        }

        hydra_buffer_t* operator->()
        {
            return &buffer_;
        }

    private:
        hydra_buffer_t buffer_;
    };

    struct Downloader
    {
        static HydraBufferWrapper Download(const std::string& url)
        {
#ifndef HYDRA_USE_GOLANG
            printf("Tried to download when built without golang support, this shouldn't happen");
            return {};
#endif
            try
            {
                hydra_buffer_t buffer = hydra_download(url.c_str());
                return HydraBufferWrapper(buffer);
            } catch (...)
            {
                printf("Failed while trying to download %s using Go\n", url.c_str());
                return {};
            }
        }
    };
} // namespace hydra