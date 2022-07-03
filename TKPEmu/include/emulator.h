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
#include "emulator_results.h"
#include "../lib/messagequeue.hxx"

namespace {
	bool always_false_ = false;
}

// Macro that adds the essential functions that every emulator have
#define TKP_EMULATOR(emulator)									\
	public:														\
	emulator();													\
	emulator(std::any args);									\
	~emulator() override;										\
	void* GetScreenData() override;								\
	bool& IsReadyToDraw() override;								\
	void HandleKeyDown(SDL_Keycode key) override;				\
	void HandleKeyUp(SDL_Keycode key) override;					\
	private:													\
	void reset() override;										\
	void start() override;										\
	bool load_file(std::string path) override 	// missing semicolon to require
											  	// semicolon on virtual classes

namespace TKPEmu {
	class Emulator {
	public:
		Emulator() {};
		Emulator(std::any args) {};
		virtual ~Emulator() = default;
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
		virtual void HandleKeyDown(SDL_Keycode keycode);
		virtual void HandleKeyUp(SDL_Keycode keycode);
		bool LoadFromFile(std::string path);
		void Screenshot(std::string filename, std::string directory = {});
		void CloseAndWait();
		virtual void* GetScreenData();
		virtual bool& IsReadyToDraw() { return always_false_; };
		virtual bool& IsResized() { return always_false_; };
		std::mutex DrawMutex;
		std::mutex FrameMutex;
		std::mutex ThreadStartedMutex;
		TKPEmu::Tools::MQServer MessageQueueServer;
	private:
		virtual void v_extra_close() {};
		virtual void start();
		virtual void reset();
		virtual bool load_file(std::string);
	};
}
#endif
