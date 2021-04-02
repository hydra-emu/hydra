#pragma once
#ifndef TKP_EMULATOR_H
#define TKP_EMULATOR_H
#include <SDL2/SDL.h>
#include <iosfwd>
#include <mutex>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>
#include <any>
#include <fstream>
#include <bitset>
#include "emulator_data.hxx"
#include "emulator_user_data.hxx"
#include "../lib/messagequeue.hxx"

namespace {
	bool always_false_ = false;
}

// Macro that adds the essential functions that every emulator have
#define TKP_EMULATOR(emulator)									\
	public:														\
	emulator();													\
	~emulator() override;										\
	void* GetScreenData() override;								\
	bool& IsReadyToDraw() override;								\
	void HandleKeyDown(uint32_t key) override;				 	\
	void HandleKeyUp(uint32_t key) override;					\
	private:													\
	void reset() override;										\
	void start() override;										\
	bool load_file(std::string path) override;					\
	bool poll_uncommon_request(const Request& request) override

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
		void Screenshot(std::string filename, std::string directory = {});
		void CloseAndWait();
		virtual int GetWidth() { return width_; }
		virtual int GetHeight() { return height_; }
		void SetWidth(int width) { width_ = width; }
		void SetHeight(int height) { height_ = height; }
		virtual void* GetScreenData();
		virtual bool& IsReadyToDraw() { return always_false_; };
		virtual bool& IsResized() { return always_false_; };
		virtual int GetBitdepth() { return 0x1401; /* GL_UNSIGNED_BYTE */ }
		std::mutex DrawMutex;
		std::mutex FrameMutex;
		std::mutex ThreadStartedMutex;
		std::shared_ptr<TKPEmu::Tools::MQBase> MessageQueue = std::shared_ptr<TKPEmu::Tools::MQBase>(new TKPEmu::Tools::MQBase);
	protected:
		// Polls common requests across emulators and returns true
		// if such a request is at the front of the queue
		// This function should only be ran if you're sure there's
		// at least 1 request
		bool poll_request(const Request& request);
		std::unique_ptr<std::ofstream> log_file_ptr_;
		std::bitset<64> log_flags_;
		bool logging_ = false;
	private:
		virtual void v_extra_close() {};
		virtual void v_log() {};
		virtual void start();
		virtual void reset();
		virtual bool load_file(std::string);
		virtual bool poll_uncommon_request(const Request& request) = 0;
		int width_, height_;
	};
}
#endif
