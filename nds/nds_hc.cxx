#include <nds/nds_hc.hxx>

using nds_emulator_t = void*;

extern "C" {
void run_frame(nds_emulator_t);
}

namespace hydra
{
    bool HydraCore_NDS::LoadFile(const std::string& type, const std::string& path)
    {
        return true;
    }

    void HydraCore_NDS::Reset() {}

    void HydraCore_NDS::SetVideoCallback(std::function<void(const VideoInfo&)> callback) {}

    void HydraCore_NDS::SetAudioCallback(std::function<void(const AudioInfo&)> callback) {}

    void HydraCore_NDS::SetPollInputCallback(std::function<void()> callback) {}

    void HydraCore_NDS::SetReadInputCallback(std::function<int8_t(const InputInfo&)> callback) {}

    void HydraCore_NDS::run_frame()
    {
        ::run_frame(nullptr);
    }
} // namespace hydra
