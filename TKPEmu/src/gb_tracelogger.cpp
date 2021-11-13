#include "../include/gb_tracelogger.h"
namespace TKPEmu::Applications {
	void GameboyTracelogger::v_draw() noexcept {

	}
	void GameboyTracelogger::SetEmulator(Emulator* emulator) noexcept {
		emulator_ = dynamic_cast<Gameboy*>(emulator);
	}
	void GameboyTracelogger::SetFile(std::string file) {
		buffer_ptr_ = std::make_unique<std::ofstream>(file, std::ofstream::out);
	}
	// Logs the current emulator state
	// Example usage: log_emu_state(LOG_PC | LOG_SP | LOG_A) will log "PC:**** SP:**** A:**"
	// TODO: (!) overwrite warning 
	void GameboyTracelogger::log_emu_state(LogType type) {
		bool first = true;
		auto& emu_cpu = emulator_->GetCPU();
		for (int i = 1; i <= LogTypeSize; i++) {
			LogType cur_type = 1 << i;
			if (type && cur_type) {
				if (!first) {
					*buffer_ptr_ << " ";
				}
				else {
					first = false;
				}
				switch (cur_type) {
					case LOG_PC:{
						*buffer_ptr_ << "PC:" << std::hex << std::setw(4) << emu_cpu.PC;
						break;
					}
					case LOG_SP: {
						*buffer_ptr_ << "SP:" << std::hex << std::setw(4) << emu_cpu.SP;
						break;
					}
					case LOG_A: {
						*buffer_ptr_ << "A:" << std::hex << std::setw(2) << emu_cpu.A;
						break;
					}
					case LOG_B: {
						*buffer_ptr_ << "B:" << std::hex << std::setw(2) << emu_cpu.B;
						break;
					}
					case LOG_C: {
						*buffer_ptr_ << "C:" << std::hex << std::setw(2) << emu_cpu.C;
						break;
					}
					case LOG_D: {
						*buffer_ptr_ << "D:" << std::hex << std::setw(2) << emu_cpu.D;
						break;
					}
					case LOG_E: {
						*buffer_ptr_ << "E:" << std::hex << std::setw(2) << emu_cpu.E;
						break;
					}
					case LOG_F: {
						*buffer_ptr_ << "F:" << std::hex << std::setw(2) << emu_cpu.F;
						break;
					}
					case LOG_H: {
						*buffer_ptr_ << "H:" << std::hex << std::setw(2) << emu_cpu.H;
						break;
					}
					case LOG_L: {
						*buffer_ptr_ << "L:" << std::hex << std::setw(2) << emu_cpu.L;
						break;
					}
					case LOG_LY: {
						*buffer_ptr_ << "LY:" << std::hex << std::setw(2) << emu_cpu.LY;
						break;
					}
					case LOG_TMA: {
						*buffer_ptr_ << "TMA:" << std::hex << std::setw(2) << emu_cpu.TMA;
						break;
					}
					case LOG_TIMA: {
						*buffer_ptr_ << "TIMA:" << std::hex << std::setw(2) << emu_cpu.TIMA;
						break;
					}
					case LOG_DIV: {
						*buffer_ptr_ << "DIV:" << std::hex << std::setw(2) << emu_cpu.Oscillator;
						break;
					}
					case LOG_TAC: {
						*buffer_ptr_ << "TAC:" << std::hex << std::setw(2) << emu_cpu.TAC;
						break;
					}
					case LOG_CY: {
						*buffer_ptr_ << "CY:" << std::dec << emu_cpu.TotalClocks;
						break;
					}
				}
			}
		}
		*buffer_ptr_ << "\n";
		lines_logged_++;
	}
}
