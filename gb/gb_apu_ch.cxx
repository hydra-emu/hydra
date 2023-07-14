#include <gb/gb_apu_ch.hxx>
#include <iostream>

namespace hydra::Gameboy
{
    void APUChannel::StepWaveGeneration(int cycles)
    {
        FrequencyTimer -= cycles;
        if (FrequencyTimer <= 0)
        {
            FrequencyTimer += (2048 - Frequency) * 4;
            // WaveDutyPosition stays in range 0-7
            WaveDutyPosition = (WaveDutyPosition + 1) & 0b111;
            DACOutput = ((DACInput / 7.5f) - 1.0f) * DACEnabled;
        }
    }

    void APUChannel::StepWaveGenerationCh4(int cycles)
    {
        FrequencyTimer -= cycles;
        if (FrequencyTimer <= 0)
        {
            FrequencyTimer = Divisor << DivisorShift;
            auto xor_res = (LFSR & 0b01) ^ ((LFSR & 0b10) >> 1);
            LFSR = (LFSR >> 1) | (xor_res << 14);
            if (WidthMode)
            {
                LFSR &= !(1 << 6);
                LFSR |= xor_res << 6;
            }
            DACOutput = DACEnabled;
        }
    }

    bool APUChannel::GetAmplitude()
    {
        return (Waveforms[WaveDutyPattern] >> WaveDutyPosition) & 0b1;
    }

    void APUChannel::StepFrameSequencer()
    {
        FrameSequencer = (FrameSequencer + 1) & 0b111;
        if (FrameSequencer % 2 == 0)
        {
            ClockLengthCtr();
            if (FrameSequencer == 2 || FrameSequencer == 6)
            {
                if (HasSweep)
                {
                    ClockSweep();
                }
            }
        }
        else if (FrameSequencer == 7)
        {
            ClockVolEnv();
        }
    }

    void APUChannel::ClockLengthCtr()
    {
        if (LengthTimer > 0 && LengthDecOne)
        {
            --LengthTimer;
        }
        if (LengthTimer == 0)
        {
            LengthCtrEnabled = false;
        }
    }

    void APUChannel::ClockVolEnv()
    {
        if (EnvelopePeriod != 0)
        {
            if (PeriodTimer > 0)
            {
                --PeriodTimer;
                if (PeriodTimer == 0)
                {
                    PeriodTimer = EnvelopePeriod;
                    if (EnvelopeCurrentVolume > 0 && !EnvelopeIncrease)
                    {
                        --EnvelopeCurrentVolume;
                    }
                    else if (EnvelopeCurrentVolume < 0xF && EnvelopeIncrease)
                    {
                        ++EnvelopeCurrentVolume;
                    }
                    DACInput = -EnvelopeCurrentVolume; // uhh idk. it sounds reversed if you dont
                                                       // add the '-' at the start
                }
            }
        }
    }

    void APUChannel::ClockSweep()
    {
        if (SweepTimer > 0)
        {
            --SweepTimer;
        }
        if (SweepTimer == 0)
        {
            if (SweepPeriod > 0)
            {
                SweepTimer = SweepPeriod;
            }
            else
            {
                SweepTimer = 8;
            }
            SweepEnabled = SweepPeriod != 0 || SweepShift != 0;
            if (SweepEnabled && SweepPeriod > 0)
            {
                CalculateSweepFreq();
                if (new_frequency <= 2047 && SweepShift > 0)
                {
                    Frequency = new_frequency;
                    ShadowFrequency = new_frequency;
                    CalculateSweepFreq();
                }
            }
        }
    }

    void APUChannel::CalculateSweepFreq()
    {
        new_frequency = ShadowFrequency >> SweepShift;
        if (!SweepIncrease)
        {
            new_frequency = ShadowFrequency - new_frequency;
        }
        else
        {
            new_frequency = ShadowFrequency + new_frequency;
        }
        if (new_frequency > 2047)
        {
            DisableChannelFlag = true;
        }
    }
} // namespace hydra::Gameboy