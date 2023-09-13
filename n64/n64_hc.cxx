#include <n64/n64_hc.hxx>

namespace hydra
{

    HydraCore_N64::HydraCore_N64() {}

    HydraCore_N64::~HydraCore_N64() {}

    bool HydraCore_N64::LoadFile(const std::string& type, const std::string& path)
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

    void HydraCore_N64::Reset()
    {
        impl_.Reset();
    }

    void HydraCore_N64::SetVideoCallback(std::function<void(const VideoInfo&)> callback)
    {
        video_callback_ = callback;
    }

    void HydraCore_N64::run_frame()
    {
        impl_.RunFrame();

        hydra::VideoInfo vi;
        vi.width = impl_.GetWidth();
        vi.height = impl_.GetHeight();
        vi.format = hydra::VideoFormat::RGBA8888;
        impl_.RenderVideo(vi.data);
        video_callback_(vi);
    }

    void HydraCore_N64::SetAudioCallback(std::function<void(const AudioInfo&)> callback)
    {
        audio_callback_ = callback;
        impl_.SetAudioCallback(std::bind(&HydraCore_N64::audio_callback_wrapper, this,
                                         std::placeholders::_1, std::placeholders::_2));
    }

    void HydraCore_N64::audio_callback_wrapper(const std::vector<int16_t>& in, int frequency_in)
    {
        hydra::AudioInfo ai;
        ma_resampler_config config = ma_resampler_config_init(
            ma_format_s16, 2, frequency_in, host_sample_rate_, ma_resample_algorithm_linear);
        ma_resampler resampler;

        struct Deleter
        {
            Deleter(ma_resampler& resampler) : resampler_(resampler){};

            ~Deleter()
            {
                ma_resampler_uninit(&resampler_, nullptr);
            };

        private:
            ma_resampler& resampler_;
        } deleter(resampler);

        if (ma_result res = ma_resampler_init(&config, nullptr, &resampler))
        {
            Logger::Fatal("Failed to create resampler: {}", static_cast<int>(res));
        }

        ma_uint64 frames_in = in.size() >> 1;
        ma_uint64 frames_out = (host_sample_rate_ / 60) * 2;
        std::vector<int16_t> temp;
        temp.resize(frames_out * 2);
        ma_result result = ma_resampler_process_pcm_frames(&resampler, in.data(), &frames_in,
                                                           temp.data(), &frames_out);
        if (result != MA_SUCCESS)
        {
            Logger::Fatal("Failed to resample: {}", static_cast<int>(result));
        }

        ai.data.resize(frames_out * 2);
        std::copy(temp.begin(), temp.begin() + frames_out * 2, ai.data.begin());
        audio_callback_(ai);
    }

    void HydraCore_N64::SetPollInputCallback(std::function<void()> callback)
    {
        impl_.SetPollInputCallback(callback);
    }

    void HydraCore_N64::SetReadInputCallback(std::function<int8_t(const InputInfo&)> callback)
    {
        read_input_callback_ = callback;
        impl_.SetReadInputCallback(std::bind(&HydraCore_N64::read_input_callback_wrapper, this,
                                             std::placeholders::_1, std::placeholders::_2,
                                             std::placeholders::_3));
    }

    int16_t HydraCore_N64::read_input_callback_wrapper(int player, int device, int button)
    {
        hydra::InputInfo ii;
        ii.player = player;

        switch (static_cast<N64::ControllerType>(device))
        {
            case N64::ControllerType::Joypad:
            {
                ii.device = hydra::InputDevice::Joypad;
                switch (button)
                {
                    case N64::Keys::A:
                    {
                        ii.button = hydra::InputButton::A;
                        break;
                    }
                    case N64::Keys::B:
                    {
                        ii.button = hydra::InputButton::B;
                        break;
                    }
                    case N64::Keys::Z:
                    {
                        ii.button = hydra::InputButton::Z;
                        break;
                    }
                    case N64::Keys::Start:
                    {
                        ii.button = hydra::InputButton::Start;
                        break;
                    }
                    case N64::Keys::CUp:
                    {
                        ii.button = hydra::InputButton::DPadUp_1;
                        break;
                    }
                    case N64::Keys::CDown:
                    {
                        ii.button = hydra::InputButton::DPadDown_1;
                        break;
                    }
                    case N64::Keys::CLeft:
                    {
                        ii.button = hydra::InputButton::DPadLeft_1;
                        break;
                    }
                    case N64::Keys::CRight:
                    {
                        ii.button = hydra::InputButton::DPadRight_1;
                        break;
                    }
                    case N64::Keys::L:
                    {
                        ii.button = hydra::InputButton::L1;
                        break;
                    }
                    case N64::Keys::R:
                    {
                        ii.button = hydra::InputButton::R1;
                        break;
                    }
                    case N64::Keys::KeypadUp:
                    {
                        ii.button = hydra::InputButton::DPadUp_0;
                        break;
                    }
                    case N64::Keys::KeypadDown:
                    {
                        ii.button = hydra::InputButton::DPadDown_0;
                        break;
                    }
                    case N64::Keys::KeypadLeft:
                    {
                        ii.button = hydra::InputButton::DPadLeft_0;
                        break;
                    }
                    case N64::Keys::KeypadRight:
                    {
                        ii.button = hydra::InputButton::DPadRight_0;
                        break;
                    }
                    case N64::Keys::AnalogHorizontal:
                    {
                        ii.button = hydra::InputButton::AnalogHorizontal_0;
                        break;
                    }
                    case N64::Keys::AnalogVertical:
                    {
                        ii.button = hydra::InputButton::AnalogVertical_0;
                        break;
                    }
                }
                break;
            }
            case N64::ControllerType::Mouse:
            {
                ii.device = hydra::InputDevice::Mouse;
                break;
            }
        }

        return read_input_callback_(ii);
    }

} // namespace hydra