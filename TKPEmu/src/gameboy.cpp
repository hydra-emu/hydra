#include "../include/gameboy.h"
#include <iostream>
#include <atomic>
#include "../glad/glad/glad.h"
namespace TKPEmu::Gameboy {
	void Gameboy::limit_fps() {
		// TODO: speedhack that sleeps for less time
		a = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> work_time = a - b;
		if (work_time.count() < sleep_time_) {
			std::chrono::duration<double, std::milli> delta_ms(sleep_time_ - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}
		b = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = b - a;
	}
	Gameboy::Gameboy() : cpu_(&bus_), ppu_(&bus_, &DrawMutex), cartridge_() {
		GLuint image_texture;
		glGenTextures(1, &image_texture);
		glBindTexture(GL_TEXTURE_2D, image_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, image_texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			160,
			144,
			0,
			GL_RGBA,
			GL_FLOAT,
			NULL
		);
		glBindTexture(GL_TEXTURE_2D, 0);
		EmulatorImage.texture = image_texture;
		EmulatorImage.width = 160;
		EmulatorImage.height = 144;
	}
	Gameboy::~Gameboy() {
		Stopped.store(true);
		glDeleteTextures(1, &EmulatorImage.texture);
	}
	void Gameboy::Start() {
		Reset();
		auto func = [this]() {
			while (!Stopped.load()) {
				if (!Paused.load()) {
					Update();
				}
			}
		};
		UpdateThread = std::thread(func);
		UpdateThread.detach();
	}

	void Gameboy::StartDebug() {
		auto func = [this]() {
			std::lock_guard<std::mutex> lguard(ThreadStartedMutex);
			Reset();
			Paused = true;
			Stopped = false;
			while (!Stopped.load()) {
				if (!Paused.load()) {
					bool broken = false;
					for (const auto& bp : Breakpoints) {
						bool brk = bp.Check();
						if (brk) {
							Paused.store(true);
							broken = true;
						}
					}
					if (!broken)
						Update();
				}
				else {
					Step.wait(false);
					Step.store(false);
					Update();
					InstructionBreak.store(cpu_.PC);
				}
			}
			return;
		};
		UpdateThread = std::thread(func);
		UpdateThread.detach();
	}

	// Starts the emulator in log mode
	void Gameboy::StartLog() {
		auto func = [this]() {
			std::lock_guard<std::mutex> lguard(ThreadStartedMutex);
			Reset();
			Paused = true;
			Stopped = false;
			while (!Stopped.load()) {
				if (!Paused.load()) {
					bool broken = false;
					for (const auto& bp : Breakpoints) {
						bool brk = bp.Check();
						if (brk) {
							Paused.store(true);
							broken = true;
						}
					}
					if (!broken)
						Update();
				} else {
					Step.wait(false);
					Step.store(false);
					Update();
					InstructionBreak.store(cpu_.PC);
					LogReady.store(true);
				}
			}
			return;
		};
		UpdateThread = std::thread(func);
		UpdateThread.detach();
	}

	void Gameboy::Reset() {
		bus_.SoftReset();
		cpu_.Reset();
		ppu_.Reset();
	}
	void Gameboy::Update() {
		//while (cpu_.tClock < cpu_.MaxCycles) {
		int clk = cpu_.Update();
		ppu_.Update(clk);
		//}
		if (cpu_.tClock >= cpu_.MaxCycles) {
			cpu_.tClock = 0;
			cpu_.TimerCounter = cpu_.ClockSpeed / 0x400;
			limit_fps();
		}
	}
	void Gameboy::HandleKey(SDL_Keycode key) {
		// TODO: implement gameboy::handlekey
	}
	void Gameboy::LoadFromFile(std::string&& path) {
		bus_.LoadCartridge(std::forward<std::string>(path));
	}
	// TODO: LoadInstrToVec only works for rom_only for now
	void Gameboy::LoadInstrToVec(std::vector<DisInstr>& vec) {
		// TODO: make this std::async
		auto func = [this](std::vector<DisInstr>& vec) {
			std::vector<DisInstr> ret;
			ret.reserve(0xFFFF);
			for (uint16_t i = 0; i < 0xFFFF;) {
				uint8_t ins = bus_.Read(i);
				auto x = cpu_.Instructions[ins];
				auto d = DisInstr(i, ins, x.skip);
				uint8_t p1, p2;
				if (x.skip == 1) {
					p1 = bus_.Read(i + 1);
					d.Params[0] = p1;
					ret.push_back(std::move(d));
				}
				else if (x.skip == 2) {
					p1 = bus_.Read(i + 1);
					p2 = bus_.Read(i + 2);
					d.Params[0] = p1;
					d.Params[1] = p2;
					ret.push_back(std::move(d));
				}
				else {
					ret.push_back(std::move(d));
				}
				i += 1 + x.skip;
			}
			vec = std::move(ret);
		};
		std::thread t1(func, std::ref(vec));
		t1.detach();
	}
	void Gameboy::AddBreakpoint(GameboyBreakpoint bp) {
		using RegCheckVector = std::vector<std::function<bool()>>;
		RegCheckVector register_checks;
		// We calculate which of these checks we need, and add them all to a vector to save execution time
		// Before being copied to the lambda, the values are decremented, as to keep them (1-256) -> (0-255)
		// because we used the value of 0 to check whether this is used or not.
		if (bp.A_using) { register_checks.push_back([this, gbbp = bp.A_value]() { return cpu_.A == gbbp; }); }
		if (bp.B_using) { register_checks.push_back([this, gbbp = bp.B_value]() { return cpu_.B == gbbp; }); }
		if (bp.C_using) { register_checks.push_back([this, gbbp = bp.C_value]() { return cpu_.C == gbbp; }); }
		if (bp.D_using) { register_checks.push_back([this, gbbp = bp.D_value]() { return cpu_.D == gbbp; }); }
		if (bp.E_using) { register_checks.push_back([this, gbbp = bp.E_value]() { return cpu_.E == gbbp; }); }
		if (bp.F_using) { register_checks.push_back([this, gbbp = bp.F_value]() { return cpu_.F == gbbp; }); }
		if (bp.H_using) { register_checks.push_back([this, gbbp = bp.H_value]() { return cpu_.H == gbbp; }); }
		if (bp.L_using) { register_checks.push_back([this, gbbp = bp.L_value]() { return cpu_.L == gbbp; }); }
		if (bp.PC_using) { register_checks.push_back([this, gbbp = bp.PC_value]() { return cpu_.PC == gbbp; }); }
		if (bp.SP_using) { register_checks.push_back([this, gbbp = bp.SP_value]() { return cpu_.SP == gbbp; }); }
		if (bp.SP_using) { register_checks.push_back([this, gbbp = bp.SP_value]() { return cpu_.SP == gbbp; }); }
		if (bp.Ins_using) { register_checks.push_back([this, gbbp = bp.Ins_value]() { return (bus_.Read(cpu_.PC)) == gbbp; }); }
		auto lamb = [rc = std::move(register_checks)]() {
			for (auto& check : rc) {
				if (!check()) {
					// If any of the checks fails, that means the breakpoint shouldn't trigger
					return false;
				}
			}
			// Every check is passed, trigger breakpoint
			return true;
		};
		bp.SetChecks(std::move(lamb));
		Breakpoints.push_back(bp);
	}
	void Gameboy::RemoveBreakpoint(int index) {
		Breakpoints.erase(Breakpoints.begin() + index);
	}
	float* Gameboy::GetScreenData()
	{
		return ppu_.GetScreenData();
	}
	const auto& Gameboy::GetOpcodeDescription(uint8_t opc) {
		return cpu_.Instructions[opc].name;
	}

	std::array<std::array<float, 3>, 4>& Gameboy::GetPalette() {
		return bus_.Palette;
	}
}
