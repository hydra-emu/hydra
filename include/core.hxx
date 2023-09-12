#pragma once

#include <cstdint>
#include <future>
#include <vector>

namespace hydra
{

    enum class VideoFormat
    {
        RGBA8888,
        RGBA5551,
    };

    struct VideoInfo
    {
        std::vector<uint8_t> data{};
        uint32_t width = 0, height = 0;
        VideoFormat format;
    };

    struct AudioInfo
    {
        std::vector<int16_t> data{};
    };

    class Core
    {
    public:
        Core() = default;
        virtual ~Core() = default;
        Core(Core&&) = default;
        Core& operator=(Core&&) = default;

        bool LoadFile(const std::string& type, const std::string& path);
        std::future<VideoInfo> RenderFrameAsync();
        std::future<AudioInfo> RenderAudioAsync();
        std::future<void> RunFrameAsync();
        void Reset();

    private:
        virtual bool load_file(const std::string& type, const std::string& path) = 0;
        virtual VideoInfo render_frame() = 0;
        virtual AudioInfo render_audio() = 0;
        virtual void run_frame() = 0;
        virtual void reset() = 0;

        Core(const Core&) = delete;
        Core& operator=(const Core&) = delete;
    };

} // namespace hydra