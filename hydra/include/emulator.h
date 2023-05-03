#pragma once
#ifndef TKP_EMULATOR_H
#define TKP_EMULATOR_H
#include <iosfwd>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include <any>
#include <fstream>
#include <bitset>
#include "emulator_data.hxx"
#include "emulator_user_data.hxx"

namespace {
	bool always_false_ = false;
}

// Macro that adds the essential functions that every emulator have
#define TKP_EMULATOR(emulator)									\
	public:														\
	emulator();													\
	~emulator() override;										\
	void* GetScreenData() override;								\
	void HandleKeyDown(uint32_t key) override;				 	\
	void HandleKeyUp(uint32_t key) override;					\
	private:													\
	void update() override;										\
	void reset() override;										\
	bool load_file(std::string path) override;					\

namespace TKPEmu {
	class Emulator {
	public:
		Emulator() {};
		virtual ~Emulator();
		Emulator(const Emulator&) = delete;
		Emulator& operator=(const Emulator&) = delete;
		std::atomic_bool Stopped = false;
		std::atomic_bool Paused = false;
		std::atomic_bool Step = false;
		std::atomic_bool Loaded = false;
		bool SkipBoot = false;
		bool FastMode = false;
		void Start();
		void Reset();
		virtual void HandleKeyDown(uint32_t keycode);
		virtual void HandleKeyUp(uint32_t keycode);
		bool LoadFromFile(std::string path);
		void CloseAndWait();
		virtual int GetWidth() { return width_; }
		virtual int GetHeight() { return height_; }
		void SetWidth(int width) { width_ = width; }
		void SetHeight(int height) { height_ = height; }
		virtual void* GetScreenData();
		bool& IsReadyToDraw() { return should_draw_; };
		virtual bool& IsResized() { return always_false_; };
		virtual int GetBitdepth() { return 0x1401; /* GL_UNSIGNED_BYTE */ }
		std::mutex ThreadStartedMutex;
		std::shared_mutex DataMutex;
	protected:
		void stop();
		int instrs_per_frame_ = 0;
		bool should_draw_ = false;
	private:
		virtual void update() = 0;
		virtual void reset();
		virtual bool load_file(std::string);
		int width_, height_;
		int64_t last_frame_time_ms_;
		int cur_instr_ = 0;
		bool reset_flag_ = false;
		std::chrono::system_clock::time_point frame_start_ = std::chrono::system_clock::now();
	};
}
#endif
