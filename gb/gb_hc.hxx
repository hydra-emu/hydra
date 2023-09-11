#pragma once

#include <array>
#include <core.hxx>
#include <gb/gb_addresses.hxx>
#include <gb/gb_apu.hxx>
#include <gb/gb_apu_ch.hxx>
#include <gb/gb_breakpoint.hxx>
#include <gb/gb_bus.hxx>
#include <gb/gb_cpu.hxx>
#include <gb/gb_ppu.hxx>
#include <gb/gb_timer.hxx>

class MmioViewer;

namespace hydra
{
    class HydraCore_Gameboy : public Core
    {
    private:
        using GameboyPalettes = std::array<std::array<float, 3>, 4>;
        using GameboyKeys = std::array<uint32_t, 4>;
        using CPU = hydra::Gameboy::CPU;
        using PPU = hydra::Gameboy::PPU;
        using APU = hydra::Gameboy::APU;
        using ChannelArrayPtr = hydra::Gameboy::ChannelArrayPtr;
        using ChannelArray = hydra::Gameboy::ChannelArray;
        using Bus = hydra::Gameboy::Bus;
        using Timer = hydra::Gameboy::Timer;
        using Cartridge = hydra::Gameboy::Cartridge;
        using GameboyBreakpoint = hydra::Gameboy::Utils::GameboyBreakpoint;

    public:
        HydraCore_Gameboy();

    private:
        ChannelArrayPtr channel_array_ptr_;
        Bus bus_;
        APU apu_;
        PPU ppu_;
        Timer timer_;
        CPU cpu_;
        GameboyKeys direction_keys_;
        GameboyKeys action_keys_;
        uint8_t &joypad_, &interrupt_flag_;
        friend class ::MmioViewer;

        bool load_file(const std::string& type, const std::string& path) override;
        VideoInfo render_frame() override;
        AudioInfo render_audio() override;
        void run_frame() override;
        void reset() override;
    };
} // namespace hydra
