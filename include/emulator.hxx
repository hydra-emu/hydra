#pragma once

#include "emulator_data.hxx"
#include "emulator_user_data.hxx"
#include <any>
#include <atomic>
#include <bitset>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <iosfwd>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

// Macro that adds the essential functions that every emulator must override
#define TKP_EMULATOR(emulator)                 \
                                               \
public:                                        \
    emulator();                                \
    ~emulator() override;                      \
    void* GetScreenData() override;            \
    void HandleKeyDown(uint32_t key) override; \
    void HandleKeyUp(uint32_t key) override;   \
                                               \
private:                                       \
    void update() override;                    \
    void reset() override;                     \
    bool load_file(const std::string& path) override;

namespace hydra
{
    class Emulator
    {
    public:
        Emulator(){};
        virtual ~Emulator();
        Emulator(const Emulator&) = delete;
        Emulator& operator=(const Emulator&) = delete;
        // TODO: Do all of these need to be atomic?
        std::atomic_bool Stopped{};
        std::atomic_bool Paused{};
        std::atomic_bool Step{};
        std::condition_variable StepCV{};
        std::atomic_bool Loaded{};
        bool SkipBoot = false;
        bool FastMode = false;
        void Start();
        void Reset();
        virtual void HandleKeyDown(uint32_t keycode);
        virtual void HandleKeyUp(uint32_t keycode);
        virtual void HandleMouseMove(int x, int y);
        bool LoadFromFile(std::string path);
        void CloseAndWait();

        virtual int GetWidth()
        {
            return width_;
        }

        virtual int GetHeight()
        {
            return height_;
        }

        virtual void* GetScreenData();

        bool& IsReadyToDraw()
        {
            return should_draw_;
        };

        std::mutex ThreadStartedMutex;
        std::mutex StepMutex;
        std::shared_mutex DataMutex;

    protected:
        void stop();
        int instrs_per_frame_ = 0;
        bool should_draw_ = false;
        int width_, height_;

    private:
        virtual void update() = 0;
        virtual void reset();
        virtual bool load_file(const std::string&);
        int64_t last_frame_time_ms_;
        int cur_instr_ = 0;
        bool reset_flag_ = false;
        std::chrono::system_clock::time_point frame_start_ = std::chrono::system_clock::now();
    };
} // namespace hydra
