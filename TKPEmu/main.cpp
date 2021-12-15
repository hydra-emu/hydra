#define SDL_MAIN_HANDLED
#include <execution>
#include <iostream>
#include "include/version.h"
#include "include/display.h"
#include "include/console_colors.h"
#include "include/start_parameters.h"
#include "include/emulator_factory.h"
#include "include/emulator_results.h"
#include "lib/str_hash.h"
// TODO: implement online version checking and updating
void print_help() noexcept;
void test_rom(std::string path);
template <typename It, typename ExecPolicy>
void test_dir_exec(It dir_it, ExecPolicy exec_pol) {
	// TODO: parallel does not work because directory iterator cant be used in parallel, find workaround
	auto begin = std::filesystem::begin(dir_it);
	auto end = std::filesystem::end(dir_it);
	std::for_each(std::execution::par_unseq, begin, end, [](const auto& file) {
		if (!file.is_directory()) {
			test_rom(file.path().string());
		}
	});
}
template<typename It>
void test_dir(It dir_it, bool parallel) {
	if (parallel) {
		test_dir_exec(dir_it, std::execution::par_unseq);
	} else {
		test_dir_exec(dir_it, std::execution::seq);
	}
}

enum class ParameterType {
	RomFile,
	RomDir,
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
				case str_hash("-d"):
				case str_hash("--display"): {
					display_mode = true;
					goto after_args;
				}
				case str_hash("-h"):
				case str_hash("--help"): {
					print_help();
					return 0;
				}
				case str_hash("-v"):
				case str_hash("--version"): {	
					std::cout << "TKPEmu by OFFTKP. Version: " color_success << TKPEmu_VERSION_MAJOR << "." << TKPEmu_VERSION_MINOR << "." << TKPEmu_VERSION_PATCH << color_reset << std::endl;
					return 0;
				}
				case str_hash("-t"):
				case str_hash("--test"): {
					expects_parameter = true;
					next_parameter_type = ParameterType::RomFile;
					break;
				}
				case str_hash("-T"):
				case str_hash("--test-directory"): {
					expects_parameter = true;
					next_parameter_type = ParameterType::RomDir;
					break;
				}
				case str_hash("-r"):
				case str_hash("--recursive"): {
					parameters.Recursive = true;
					break;
				}
				case str_hash("-p"):
				case str_hash("--parallel"): {
					parameters.Parallel = true;
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
				case ParameterType::RomDir: {
					if (std::filesystem::is_directory(argv[i])) {
						parameters.RomDir = argv[i];
					} else {
						std::cerr << color_error "Error: " color_reset "Specified directory does not exist " << argv[i] << std::endl;
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
		if (!parameters.RomFile.empty()) {
			test_rom(parameters.RomFile);			
		} else if (!parameters.RomDir.empty()) {
			if (parameters.Recursive) {
				test_dir(std::filesystem::recursive_directory_iterator(parameters.RomDir), parameters.Parallel);
			} else {
				test_dir(std::filesystem::directory_iterator(parameters.RomDir), parameters.Parallel);
			}
		} else {
			std::cerr << color_error "Error: " color_reset "No rom file specified. Use -h or --help for more help on the commands" << std::endl;
		}
	}
	return 0;
}

void print_help() noexcept {
	std::cout << "For debugging/testing check the manual:\n"
		"man tkpemu\n" 
	<< std::endl;
}

void test_rom(std::string path) {
	auto type = TKPEmu::EmulatorFactory::GetEmulatorType(path); 
	if (type == TKPEmu::EmuType::None) {
		std::cerr << "[" color_warning << std::filesystem::path(path).filename() << color_reset "]: No available emulator found for this file" << std::endl;
		return;
	}
	std::unique_ptr<TKPEmu::Emulator> emu = TKPEmu::EmulatorFactory::Create(type);
	emu->SkipBoot = true;
	emu->FastMode = true;
	emu->LoadFromFile(path);
	if (!PassedTestMap.contains(emu->RomHash)) {
		std::cerr << "[" color_error << std::filesystem::path(path).filename() << color_reset "]: This rom does not have a hash in emulator_results" << std::endl;
		return;
	}
	auto result = PassedTestMap.at(emu->RomHash);
	auto options = TKPEmu::EmuStartOptions::Console;
	emu->ScreenshotClocks = result.Clocks;
	emu->ScreenshotHash = result.ExpectedHash;
	emu->Start(options);
}