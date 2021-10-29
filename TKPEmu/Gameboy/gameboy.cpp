#include "gameboy.h"
#include <iostream>
namespace TKPEmu::Gameboy {
	Gameboy::Gameboy() : cpu_(&bus_), ppu_(&bus_), cartridge_() {

	}
	Gameboy::~Gameboy() {
		Stopped.store(true);
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
		// TODO: StartDebug starts paused
		Paused.store(true);
		Reset();
		auto func = [this]() {
			Stopped = true;
			std::lock_guard<std::mutex> lguard(ThreadStartedMutex);
			Stopped = false;
			while (!Stopped.load(std::memory_order_seq_cst)) {
				Paused.wait(true);
				if (!Paused.load()) {
					for (const auto& bp : Breakpoints) {
					bool brk = bp();
						if (brk) {
							Paused.store(true);
							Break.store(true);
							InstructionBreak.store(cpu_.PC);
						}
					}
				}
				else {
					if (Step.load()) {
						Update();
						Step.store(false);
					}
				}
				if (!Paused.load()) {
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
		cpu_.Reset();
		ppu_.Reset();
	}
	void Gameboy::Update() {
		cpu_.Update();
		//ppu_.Update();
		//if (ppu_.NeedsDraw()) {
		//	ScreenDataMutex.lock();
		//	ppu_.Draw(EmulatorImage);
		//	ScreenDataMutex.unlock();
		//}
	}
	void Gameboy::LoadFromFile(const std::string& path) {
		cartridge_.Load(path, bus_.mem);
	}
	// TODO: LoadInstrToVec only works for rom_only for now
	// TODO: if you win + D while in disassembler, game crashes
	void Gameboy::LoadInstrToVec(std::vector<DisInstr>& vec, bool& finished) {
		// TODO: make this std::async
		auto func = [this, &finished](std::vector<DisInstr>& vec) {
			for (uint16_t i = 0; i < 0x8000;) {
				uint8_t ins = cpu_.bus_->Read(i);
				auto x = cpu_.instructions[ins];
				auto d = DisInstr(i, ins);
				uint8_t p1, p2;
				if (x.skip == 1) {
					p1 = cpu_.bus_->Read(i + 1);
					d.Params[0] = p1;
					std::stringstream ss;
					ss << x.name << " " << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int)p1;
					d.InstructionFull = ss.str();
					std::stringstream ss2;
					ss2 << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int)ins << " " << std::setfill('0') << std::setw(2) << (int)p1;
					d.InstructionHex = ss2.str();
					vec.push_back(std::move(d));
				}
				else if (x.skip == 2) {
					p1 = cpu_.bus_->Read(i + 1);
					p2 = cpu_.bus_->Read(i + 2);
					d.Params[0] = p1;
					d.Params[1] = p2;
					std::stringstream ss;
					ss << x.name << " " << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int)p2 << std::setfill('0') << std::setw(2) << (int)p1;
					d.InstructionFull = ss.str();
					std::stringstream ss2;
					ss2 << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (int)ins << " " << std::setfill('0') << std::setw(2) << (int)p1 << " " << std::setfill('0') << std::setw(2) << (int)p2;
					d.InstructionHex = ss2.str();
					vec.push_back(std::move(d));
				}
				else {
					std::stringstream ss;
					ss << x.name;
					d.InstructionFull = ss.str();
					vec.push_back(std::move(d));
				}
				i += 1 + x.skip;
			}
			finished = true;
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
		if (!bp.A_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.A_value - 1]() { return cpu_->A == gbbp; }); }
		if (!bp.B_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.B_value - 1]() { return cpu_->B == gbbp; }); }
		if (!bp.C_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.C_value - 1]() { return cpu_->C == gbbp; }); }
		if (!bp.D_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.D_value - 1]() { return cpu_->D == gbbp; }); }
		if (!bp.E_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.E_value - 1]() { return cpu_->E == gbbp; }); }
		if (!bp.F_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.F_value - 1]() { return cpu_->F == gbbp; }); }
		if (!bp.H_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.H_value - 1]() { return cpu_->H == gbbp; }); }
		if (!bp.L_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.L_value - 1]() { return cpu_->L == gbbp; }); }
		if (!bp.PC_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.PC_value - 1]() { return cpu_->PC == gbbp; }); }
		if (!bp.SP_using) { register_checks.push_back([cpu_ = &cpu_, gbbp = bp.SP_value - 1]() { return cpu_->SP == gbbp; }); }
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
		Breakpoints.push_back(lamb);
	}
	void Gameboy::RemoveBreakpoint(int index) {
		Breakpoints.erase(Breakpoints.begin() + index);
	}
}