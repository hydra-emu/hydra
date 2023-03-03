#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <filesystem>
#include <iostream>
#include <fstream>
#include <include/emulator.h>
#include <lib/md5.h>
#include <include/error_factory.hxx>
#include <lib/str_hash.h>
#include <valgrind/callgrind.h>

#ifndef CALLGRIND_START_INSTRUMENTATION
#define NO_PROFILING
#define CALLGRIND_START_INSTRUMENTATION
#endif
#ifndef CALLGRIND_STOP_INSTRUMENTATION
#define CALLGRIND_STOP_INSTRUMENTATION
#endif

namespace TKPEmu {
    Emulator::~Emulator() {}
    void Emulator::HandleKeyDown(uint32_t keycode) { 
        std::cout << "Warning: Key " << keycode << " was pressed but\n"
        	"emulator.HandleKeyDown was not implemented" << std::endl;
    }
    void Emulator::HandleKeyUp(uint32_t keycode) { 
        std::cout << "Warning: Key " << keycode << " was released but\n"
        	"emulator.HandleKeyUp was not implemented" << std::endl;
    }
    void* Emulator::GetScreenData() { 
        throw ErrorFactory::generate_exception(__func__, __LINE__, "GetScreenData was not implemented for this emulator");
    }
    void Emulator::Start() { 
		std::lock_guard<std::mutex> lguard(ThreadStartedMutex);
        if (instrs_per_frame_ == 0) {
            throw ErrorFactory::generate_exception(__func__, __LINE__, "instrs_per_frame_ is 0");
        }
		Loaded = true;
		Loaded.notify_all();
		Paused = false; // TODO: get from settings
		Stopped = false;
		Step = false;
		Reset();
		if (Paused) {
			goto paused;
		}
		begin:
		CALLGRIND_START_INSTRUMENTATION;
		do {
            std::unique_lock lock(DataMutex);
			frame_start_ = std::chrono::system_clock::now();
			for (; cur_instr_ < instrs_per_frame_; cur_instr_++)
				update();
			auto end = std::chrono::system_clock::now();
			auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - frame_start_).count();
			last_frame_time_ms_ = dur;
			if (Stopped.load()) {
				return;
			}
			if (Paused.load()) {
				break;
			}
			should_draw_ = true;
            cur_instr_ = 0;
		} while(true);
		CALLGRIND_STOP_INSTRUMENTATION;
		paused:
		Step.wait(false);
		Step.store(false);
		update();
		goto begin;
    }
	void Emulator::reset() { 
		throw ErrorFactory::generate_exception(__func__, __LINE__, "reset was not implemented for this emulator");
    }
	bool Emulator::load_file(std::string) { 
		throw ErrorFactory::generate_exception(__func__, __LINE__, "load_file was not implemented for this emulator");
    }
	bool Emulator::LoadFromFile(std::string path) {
		return load_file(path);
	}
    void Emulator::Reset() {
        reset();
    }
	void Emulator::CloseAndWait() {
		Step.store(true);
        Paused.store(false);
        stop();
        Step.notify_all();
		std::lock_guard<std::mutex> lguard(ThreadStartedMutex);
	}
    void Emulator::stop() {
        Stopped.store(true);
        cur_instr_ = instrs_per_frame_;
    }
}