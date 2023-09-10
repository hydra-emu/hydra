#include <n64/n64_hc.hxx>

namespace hydra
{

    HydraCore_N64::HydraCore_N64() {}

    HydraCore_N64::~HydraCore_N64() {}

    bool HydraCore_N64::load_file(const std::string& type, const std::string& path)
    {
        if (type == "ipl")
        {
            ipl_loaded_ = impl_.LoadIPL(path);
            return ipl_loaded_;
        }
        else if (type == "rom")
        {
            if (!ipl_loaded_)
                return false;
            return impl_.LoadCartridge(path);
        }
        return false;
    }

    VideoInfo HydraCore_N64::render_frame()
    {
        VideoInfo info;
        info.width = impl_.GetWidth();
        info.height = impl_.GetHeight();
        impl_.RenderVideo(info.data);
        info.format = VideoFormat::RGBA8888;
        return info;
    }

    AudioInfo HydraCore_N64::render_audio()
    {
        AudioInfo info;
        impl_.RenderAudio(info.data);
        return info;
    }

    void HydraCore_N64::reset()
    {
        impl_.Reset();
    }

    void HydraCore_N64::run_frame()
    {
        impl_.RunFrame();
    }

} // namespace hydra