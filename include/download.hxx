#pragma once

#include "go/downloader.h"
#include <memory>
#include <string>

namespace hydra
{
    struct HydraBuffer
    {
        HydraBuffer() = default;

        HydraBuffer(const hydra_buffer_t& buffer) : buffer_(buffer) {}

        ~HydraBuffer()
        {
            if (buffer_.data)
                free(buffer_.data);
        }

        void* data() const
        {
            return buffer_.data;
        }

        size_t size() const
        {
            return buffer_.size;
        }

    private:
        hydra_buffer_t buffer_;
    };

    using HydraBufferWrapper = std::unique_ptr<HydraBuffer>;

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
                return std::make_unique<HydraBuffer>(buffer);
            } catch (...)
            {
                printf("Failed while trying to download %s using Go\n", url.c_str());
                return {};
            }
        }
    };
} // namespace hydra