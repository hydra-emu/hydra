#pragma once

#include <core.hxx>
#include <n64/core/n64_impl.hxx>

namespace hydra
{
    class HydraCore_N64 : public Core
    {
    public:
        HydraCore_N64();
        ~HydraCore_N64();

    private:
        bool load_file(const std::string& type, const std::string& path) override;
        VideoInfo render_frame() override;
        AudioInfo render_audio() override;
        void run_frame() override;
        void reset() override;

        bool ipl_loaded_ = false;
        N64::N64 impl_;
    };
} // namespace hydra