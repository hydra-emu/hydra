#define SDL_MAIN_HANDLED
#include <iostream>
#include "include/version.h"
#include "include/display.h"
#include "include/console_colors.h"
#include "include/start_parameters.h"
#include "include/emulator_factory.h"
#include "include/emulator_results.h"
#include "lib/str_hash.h"
// TODO: implement online version checking and updating
// TODO: last open files folder becomes the browser default folder
void print_help() noexcept;

enum class ParameterType {
	RomFile,
	ScreenshotDir,
	ScreenshotTime,
};

int main(int argc, char *argv[]) {
	// Whenever we get an argument that needs parameters, this bool is set to true
	bool expects_parameter = false;
	bool display_mode = false;
	ParameterType next_parameter_type;
	TKPEmu::StartParameters parameters;
	if (argc == 1) {
		std::cout << color_warning "Warning: " color_reset "No parameters specified. Try -h or --help.\nRunning gui as a default..." << std::endl;
		display_mode = true;
	}
	for (int i = 1; i < argc; i++) {
		if (!expects_parameter) {
			switch (str_hash(argv[i])) {
				case str_hash("--display"): {
					display_mode = true;
					goto after_args;
				}
				case str_hash("--help"): {
					print_help();
					return 0;
				}
				case str_hash("--version"): {	
					std::cout << "TKPEmu by OFFTKP. Version: " color_success << TKPEmu_VERSION_MAJOR << "." << TKPEmu_VERSION_MINOR << "." << TKPEmu_VERSION_PATCH << color_reset << std::endl;
					return 0;
				}
				case str_hash("-t"): {
					expects_parameter = true;
					next_parameter_type = ParameterType::RomFile;
					break;
				}
				default: {
					std::cerr << color_error "Error: " color_reset "Invalid parameter.\nUse -h or --help to get the parameter list." << std::endl;
					return 1;
				}
			}
		} else {
			expects_parameter = false;
			switch(next_parameter_type) {
				case ParameterType::RomFile: {
					bool file_exists = false;
					try {
						file_exists = std::filesystem::exists(argv[i]);
						if (std::filesystem::is_directory(argv[i])) {
							std::cerr << color_error "Error: " color_reset "Expected file, got directory " << argv[i] << std::endl;
							return 1;
						}
					} catch (const std::exception& e) {
						std::cerr << color_error "Error: " color_reset << e.what() << std::endl;
						return 1;
					}
					if (file_exists) {
						parameters.RomFile = argv[i];
					} else {
						std::cerr << color_error "Error: " color_reset "File does not exist " << argv[i] << std::endl;
						return 1;
					}
					break;
				}
				case ParameterType::ScreenshotDir: {
					if (std::filesystem::is_directory(argv[i])) {
						parameters.ScreenshotDir = argv[i];
					} else {
						std::cerr << color_error "Error: " color_reset "Specified directory does not exist " << argv[i] << std::endl;
						return 1;
					}
					break;
				}
				case ParameterType::ScreenshotTime: {
					try {
						parameters.ScreenshotTime = std::stoull(argv[i]);
					} catch (const std::exception& e) {
						std::cerr << color_error "Error: " color_reset "Could not parse " << argv[i] << " to ull" << std::endl;
						return 1;
					}
					break;
				}
				default: {
					std::cerr << "Error: Unknown ParameterType" << std::endl;
					return 1;
				}
			}
		}
	}
	after_args:
	if (display_mode) {
		// TODO: remake display constructor, takes runparameters class
		std::cout << "Opening GUI..." << std::endl;
		TKPEmu::Graphics::Display dis;
		dis.EnterMainLoop();
	} else {
		// TODO: implement console mode. Both display and console (make console class) should take a RunParameters class
		// that explains fast_mode, log_mode and other settings. Display constructor takes this as param.
		if (!parameters.RomFile.empty()) {
			auto type = TKPEmu::EmulatorFactory::GetEmulatorType(parameters.RomFile); 
			std::unique_ptr<TKPEmu::Emulator> emu = TKPEmu::EmulatorFactory::Create(type);
			emu->SkipBoot = true;
			emu->FastMode = true;
			emu->LoadFromFile(parameters.RomFile);
			if (!PassedTestMap.contains(emu->RomHash)) {
				std::cerr << color_error "Error: " color_reset "This rom does not have a hash in emulator_results" << std::endl;
				return 1;
			}
			auto result = PassedTestMap.at(emu->RomHash);
			auto options = TKPEmu::EmuStartOptions::Console;
			emu->ScreenshotClocks = result.Clocks;
			emu->ScreenshotHash = result.ExpectedHash;
			emu->Start(options);
		} else {
			std::cerr << color_error "Error: " color_reset "No rom file specified. Use -h or --help for more help on the commands" << std::endl;
		}
	}
	return 0;
}

void print_help() noexcept {
	std::cout << "Commands:\n" 
		"--help: Shows this dialog\n"
		"--display: Starts in GUI mode\n"
		"--version: Print version\n"
		"-t (filename): Test a rom and print the result\n"
		"-tm (files seperated by space): Test multiple roms\n"
	<< std::endl;
}
