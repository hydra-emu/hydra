#include "gameboy.h"
#include <iostream>
namespace TKPEmu::Gameboy {
	Gameboy::Gameboy() : cpu_(&bus_), ppu_(&bus_), cartridge_(), file(str) {

	}
	Gameboy::~Gameboy() {
		Stopped.store(true);
		file.close();
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
		Reset();
		auto func = [this]() {
			std::lock_guard<std::mutex> lguard(ThreadStartedMutex);
			Paused = true;
			Stopped = false;
			while (!Stopped.load(std::memory_order_seq_cst)) {
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
					InstructionBreak.store(cpu_.PC);
					Break.store(true);
					Step.wait(false);
					Step.store(false);
					Update();
				}
			}
			// As the thread closes, set store to false so the next thread enters the loop
			Stopped.store(false, std::memory_order_seq_cst);
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
		std::lock_guard<std::mutex> lg(UpdateMutex);
		cpu_.TimerCounter = cpu_.ClockSpeed / 0x400;
		while (cpu_.tClock < cpu_.MaxCycles) {
			int clk = cpu_.Update();
			ppu_.Update(clk);
		}
		cpu_.tClock = 0;
	}
	void Gameboy::LoadFromFile(std::string&& path) {
		bus_.LoadCartridge(std::forward<std::string>(path));
	}
	// TODO: LoadInstrToVec only works for rom_only for now
	// TODO: if you win + D while in disassembler, game crashes
	void Gameboy::LoadInstrToVec(std::vector<DisInstr>& vec, std::atomic_bool& finished) {
		// TODO: make this std::async
		auto func = [this, &finished](std::vector<DisInstr>& vec) {
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
			finished.store(true);
			finished.notify_all();
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
	void Gameboy::CopyRegToBreakpoint(GameboyBreakpoint& bp) {
		std::lock_guard<std::mutex> lg(UpdateMutex);
		bp.A_value = cpu_.A;
		bp.B_value = cpu_.B;
		bp.C_value = cpu_.C;
		bp.D_value = cpu_.D;
		bp.E_value = cpu_.E;
		bp.F_value = cpu_.F;
		bp.H_value = cpu_.H;
		bp.L_value = cpu_.L;
		bp.SP_value = cpu_.SP;
		bp.PC_value = cpu_.PC;
	}
	const auto& Gameboy::GetOpcodeDescription(uint8_t opc) {
		return cpu_.Instructions[opc].name;
	}
}