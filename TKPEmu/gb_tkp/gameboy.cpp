#include <iostream>
#include <atomic>
#include "../glad/glad/glad.h"
#include "gameboy.h"
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
	Gameboy::Gameboy(GameboyKeys& direction_keys, GameboyKeys& action_keys) :
		bus_(Instructions),
		cpu_(&bus_),
		ppu_(&bus_, &DrawMutex),
		timer_(&bus_),
		direction_keys_(direction_keys),
		action_keys_(action_keys),
		joypad_(bus_.GetReference(addr_joy)),
		interrupt_flag_(bus_.GetReference(addr_if))
	{
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
	void Gameboy::SetLogTypes(std::unique_ptr<std::vector<LogType>> types_ptr) {
		log_types_ptr_ = std::move(types_ptr);
	}
	void Gameboy::v_log_state() {
		*ofstream_ptr_ << std::setfill('0');
		bool first_type = true;
		for (const auto& t : *log_types_ptr_) {
			switch (t) {
				case LogType::PC: {
					*ofstream_ptr_ << "PC:" << std::setw(4) << std::hex <<  cpu_.PC;
					break;
				}
				case LogType::SP: {
					*ofstream_ptr_ << "SP:" << std::setw(4) << std::hex <<  cpu_.SP;
					break;
				}
				case LogType::A: {
					*ofstream_ptr_ << "A:" << std::setw(2) << std::hex << (int)cpu_.A;
					break;
				}
				case LogType::B: {
					*ofstream_ptr_ << "B:" << std::setw(2) << std::hex << (int)cpu_.B;
					break;
				}
				case LogType::C: {
					*ofstream_ptr_ << "C:" << std::setw(2) <<  std::hex << (int)cpu_.C;
					break;
				}
				case LogType::D: {
					*ofstream_ptr_ << "D:" << std::setw(2) << std::hex << (int)cpu_.D;
					break;
				}
				case LogType::E: {
					*ofstream_ptr_ << "E:" << std::setw(2) << std::hex << (int)cpu_.A;
					break;
				}
				case LogType::F: {
					*ofstream_ptr_ << "F:" <<  std::setw(2) << std::hex << (int)cpu_.F;
					break;
				}
				case LogType::H: {
					*ofstream_ptr_ << "H:" << std::setw(2) << std::hex << (int)cpu_.H;
					break;
				}
				case LogType::L: {
					*ofstream_ptr_ << "L:" << std::setw(2) << std::hex << (int)cpu_.L;
					break;
				}
			}
			*ofstream_ptr_ << " ";
		}
		*ofstream_ptr_ << "\n";
	}
	void Gameboy::start_normal() { 
		Reset();
		auto func = [this]() {
			while (!Stopped.load()) {
				if (!Paused.load()) {
					update();
				}
			}
		};
		UpdateThread = std::thread(func);
		UpdateThread.detach();
	}
	void Gameboy::start_debug() {
		auto func = [this]() {
			std::lock_guard<std::mutex> lguard(ThreadStartedMutex);
			Reset();
			Paused = true;
			Stopped = false;
			Step = false;
			// Emulation doesn't break on first instruction
			bool first_instr = true;
			while (!Stopped.load()) {
				if (!Paused.load()) {
					bool broken = false;
					if (!first_instr) {
						for (const auto& bp : Breakpoints) {
							bool brk = bp.Check();
							if (brk) {
								InstructionBreak.store(cpu_.PC);
								Paused.store(true);
								broken = true;
							}
						}
					}
					first_instr = false;
					if (!broken)
						update();
				}
				else {
					Step.wait(false);
					Step.store(false);
					update();
					InstructionBreak.store(cpu_.PC);
				}
			}
			return;
		};
		UpdateThread = std::thread(func);
		UpdateThread.detach();
	}
	void Gameboy::reset_normal() {
		bus_.SoftReset();
		cpu_.Reset(false);
		timer_.Reset();
		ppu_.Reset();
	}
	void Gameboy::reset_skip() {
		bus_.SoftReset();
		cpu_.Reset(true);
		timer_.Reset();
		ppu_.Reset();
	}
	void Gameboy::update() {
		if (cpu_.PC == 0x100) {
			bus_.BiosEnabled = false;
		}
		int clk = cpu_.Update();
		if (timer_.Update(clk)) {
			interrupt_flag_ |= IFInterrupt::TIMER;
		}
		ppu_.Update(clk);
		if (cpu_.TClock >= cpu_.MaxCycles) {
			cpu_.TClock = 0;
			limit_fps();
		}
		log_state();
	}
	std::string Gameboy::print() const { 
		return "GameboyTKP for TKPEmu\n"
		       "Read more: https://github.com/OFFTKP/TKPEmu/tree/master/TKPEmu/gb_tkp";
	}
	void Gameboy::HandleKeyDown(SDL_Keycode key) {
		static const uint8_t joy_direction = 0b1110'1111;
		static const uint8_t joy_action = 0b1101'1111;
		if (auto it_dir = std::find(direction_keys_.begin(), direction_keys_.end(), key); it_dir != direction_keys_.end()) {
			int index = it_dir - direction_keys_.begin();
			bus_.DirectionKeys = (~(1UL << index)) & joy_direction;
			interrupt_flag_ |= IFInterrupt::JOYPAD;
		}
		if (auto it_dir = std::find(action_keys_.begin(), action_keys_.end(), key); it_dir != action_keys_.end()) {
			int index = it_dir - action_keys_.begin();
			bus_.ActionKeys = (~(1UL << index)) & joy_action;
			interrupt_flag_ |= IFInterrupt::JOYPAD;
		}
	}
	void Gameboy::HandleKeyUp(SDL_Keycode key) {
		static const uint8_t joy_direction = 0b1110'1111;
		static const uint8_t joy_action = 0b1101'1111;
		if (auto it_dir = std::find(direction_keys_.begin(), direction_keys_.end(), key); it_dir != direction_keys_.end()) {
			int index = it_dir - direction_keys_.begin();
			bus_.DirectionKeys = (1UL << index) | joy_direction;
		}
		if (auto it_dir = std::find(action_keys_.begin(), action_keys_.end(), key); it_dir != action_keys_.end()) {
			int index = it_dir - action_keys_.begin();
			bus_.ActionKeys = (1UL << index) | joy_action;
		}
	}
	void Gameboy::LoadFromFile(std::string&& path) {
		bus_.LoadCartridge(std::forward<std::string>(path));
	}
	DisInstr Gameboy::GetInstruction(uint16_t address) {
		uint8_t ins = bus_.Read(address);
		auto time = InstrTimes[ins];
		auto instr = DisInstr(address, ins, time);
		uint8_t p1 = 0, p2 = 0;
		if (time == 1) {
			p1 = bus_.Read(address + 1);
			instr.Params[0] = p1;
			return instr;
		}
		else if (time == 2) {
			p1 = bus_.Read(address + 1);
			p2 = bus_.Read(address + 2);
			instr.Params[0] = p1;
			instr.Params[1] = p2;
			return instr;
		}
		else {
			return instr;
		}
	}
	bool Gameboy::AddBreakpoint(GBBPArguments bp) {
		using RegCheckVector = std::vector<std::function<bool()>>;
		RegCheckVector register_checks;
		// TODO: Check if breakpoint already exists before adding it
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
		GameboyBreakpoint gbp;
		gbp.Args = std::move(bp);
		gbp.SetChecks(std::move(lamb));
		std::cout << "Breakpoint added:\n" << gbp.GetName() << std::endl;
		bool ret = false;
		if (gbp.BPFromTable)
			ret = true;
		Breakpoints.push_back(std::move(gbp));
		return ret;
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

	GameboyPalettes& Gameboy::GetPalette() {
		return bus_.Palette;
	}
}
