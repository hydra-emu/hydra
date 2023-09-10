#include <core.hxx>

namespace hydra
{

    bool Core::LoadFile(const std::string& type, const std::string& path)
    {
        return load_file(type, path);
    }

    std::future<VideoInfo> Core::RenderFrameAsync()
    {
        return std::async(std::launch::async, [this]() { return render_frame(); });
    }

    std::future<AudioInfo> Core::RenderAudioAsync()
    {
        return std::async(std::launch::async, [this]() { return render_audio(); });
    }

    std::future<void> Core::RunFrameAsync()
    {
        return std::async(std::launch::async, [this]() { run_frame(); });
    }

    void Core::Reset()
    {
        reset();
    }

} // namespace hydra