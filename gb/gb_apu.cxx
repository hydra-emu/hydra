#include <gb/gb_addresses.hxx>
#include <gb/gb_apu.hxx>
#include <iostream>
constexpr int SAMPLE_RATE = 48000;
constexpr int AMPLITUDE = 8000;
constexpr int RESAMPLED_RATE = (4194304 / SAMPLE_RATE);

namespace hydra::Gameboy
{
    APU::APU(ChannelArrayPtr channel_array_ptr, uint8_t& NR52)
        : channel_array_ptr_(channel_array_ptr), NR52_(NR52)
    {
    }

    APU::~APU()
    {
        if (UseSound)
        {
            // SDL_CloseAudioDevice(device_id_);
        }
    }

    void APU::InitSound()
    {
        if (init_)
        {
            // audio_sink_.reset();
            std::fill(samples_.begin(), samples_.end(), 0);
            sample_index_ = 0;
            return;
        }
        if (UseSound)
        {
            // SDL_AudioSpec want;
            // SDL_zero(want);
            // want.freq = SAMPLE_RATE;
            // want.format = AUDIO_S16SYS;
            // want.channels = 1;
            // want.samples = 512;
            // SDL_AudioSpec have;
            // device_id_ = SDL_OpenAudioDevice(0, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE
            // | SDL_AUDIO_ALLOW_SAMPLES_CHANGE); if (want.format != have.format) {
            //     SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to get the desired AudioSpec");
            // }
            // float i = 0;
            // std::fill(samples_.begin(), samples_.end(), 0);
            // SDL_PauseAudioDevice(device_id_, 0);
            // init_ = true;
        }
    }

    void APU::Update(int clk)
    {
        if (UseSound)
        {
            static int inner_clk = 0;
            auto& chan1 = (*channel_array_ptr_)[0];
            auto& chan2 = (*channel_array_ptr_)[1];
            auto& chan4 = (*channel_array_ptr_)[3];
            inner_clk += clk;
            chan1.StepWaveGeneration(clk);
            chan2.StepWaveGeneration(clk);
            chan4.StepWaveGenerationCh4(clk);
            double chan1out = (chan1.GetAmplitude() == 0.0 ? 1.0 : -1.0) * chan1.DACOutput *
                              chan1.GlobalVolume() * !!chan1.EnvelopeCurrentVolume;
            double chan2out = (chan2.GetAmplitude() == 0.0 ? 1.0 : -1.0) * chan2.DACOutput *
                              chan2.GlobalVolume() * !!chan2.EnvelopeCurrentVolume;
            double chan4out = (~chan4.LFSR & 0x01) * chan4.DACOutput * chan4.GlobalVolume() *
                              !!chan4.EnvelopeCurrentVolume;
            if (inner_clk >= RESAMPLED_RATE)
            {
                auto sample = (chan1out + chan2out + chan4out) / 3;
                samples_[sample_index_++] = sample * AMPLITUDE;
                // in case it's bigger
                inner_clk = inner_clk - RESAMPLED_RATE;
            }
            if (sample_index_ == samples_.size())
            {
                sample_index_ = 0;
                QueueSamples();
            }
        }
    }
} // namespace hydra::Gameboy