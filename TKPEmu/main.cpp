#define SDL_MAIN_HANDLED
#include <execution>
#include <iostream>
#include <syncstream>
#include <mutex>
#include "include/version.h"
#include "include/display.h"
#include "include/console_colors.h"
#include "include/start_parameters.h"
#include "include/emulator_factory.h"
#include "include/emulator_results.h"
#include "lib/str_hash.h"
using TestResult = TKPEmu::Testing::TestResult;
using TestData = TKPEmu::Testing::TestData;
using TestDataVec = std::vector<TestData>;
enum class ParameterType {
	RomFile,
	RomDir,
	TestResultsPath,
};
// TODO: implement online version checking and updating
TKPEmu::StartParameters parameters;
// TODO: remove last_emulator_name -> add in TestData
std::string last_emulator_name = "Unknown emulator";
void print_help() noexcept;
TestData test_rom(std::string path);
void generate_results(TestDataVec& results);
template <typename It, typename ExecPolicy>
TestDataVec test_dir_exec(It begin, It end, ExecPolicy exec_pol) {
	std::mutex push_mutex;
	TestDataVec results;
	std::for_each(exec_pol, begin, end, [&](const auto& file) {
		if (!file.is_directory()) {
			TestData res = test_rom(file.path().string());
			if (res.Result != TestResult::None) {
				std::lock_guard<std::mutex> lg(push_mutex);
				results.push_back(res);
			}
		}
	});
	return results;
}
template<typename It>
void test_dir(It dir_it, bool parallel) {
	std::vector<typename It::value_type> file_vec;
	for (auto& f : dir_it) {
		file_vec.push_back(f);		
	}
	auto begin = file_vec.begin();
	auto end = file_vec.end();
	TestDataVec results;
	if (parallel) {
		results = test_dir_exec(begin, end, std::execution::par_unseq);
	} else {
		results = test_dir_exec(begin, end, std::execution::seq);
	}
	if (parameters.GenerateTestResults) {
		generate_results(results);
	}
}
int main(int argc, char *argv[]) {
	// Whenever we get an argument that needs parameters, this bool is set to true
	bool expects_parameter = false;
	bool display_mode = false;
	ParameterType next_parameter_type;
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
				case str_hash("-g"):
				case str_hash("--generate-markdown"): {
					parameters.GenerateTestResults = true;
					break;
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
				case str_hash("-V"):
				case str_hash("--verbose"): {
					parameters.Verbose = true;
					break;
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
				case str_hash("-G"):
				case str_hash("--generation-path"): {
					next_parameter_type = ParameterType::TestResultsPath;
					expects_parameter = true;
					break;
				}
				default: {
					std::cerr << color_error "Error: " color_reset "Invalid parameter: " << argv[i] << ".\nUse -h or --help to get the parameter list." << std::endl;
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
				case ParameterType::TestResultsPath: {
					if (!std::filesystem::is_directory(argv[i])) {
						parameters.GenerationPath = argv[i];
					} else {
						std::cerr << color_error "Error: " color_reset "Specified path is a directory, expected file " << argv[i] << std::endl;
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
TestData test_rom(std::string path) {
	TestData ret;
	std::osyncstream scout(std::cout);
	auto type = TKPEmu::EmulatorFactory::GetEmulatorType(path); 
	if (type == TKPEmu::EmuType::None) {
		if (parameters.Verbose) {
			scout << "[" color_warning << std::filesystem::path(path).filename() << color_reset "]: No available emulator found for this file" << std::endl;
		}
		ret.Result = TestResult::None;
		return ret;
	}
	std::unique_ptr<TKPEmu::Emulator> emu = TKPEmu::EmulatorFactory::Create(type);
	emu->SkipBoot = true;
	emu->FastMode = true;
	emu->LoadFromFile(path);
	if (!TKPEmu::Testing::PassedTestMap.contains(emu->RomHash)) {
		if (parameters.Verbose) {
			scout << "[" color_error << std::filesystem::path(path).filename() << color_reset "]: This rom does not have a hash in emulator_results" << std::endl;
		}
		ret.Result = TestResult::None;
		return ret;
	}
	auto result = TKPEmu::Testing::PassedTestMap.at(emu->RomHash);
	auto options = TKPEmu::EmuStartOptions::Console;
	emu->ScreenshotClocks = result.Clocks;
	emu->ScreenshotHash = result.ExpectedHash;
	last_emulator_name = emu->GetEmulatorName();
	emu->Start(options);
	// Console mode does not run on a seperate thread so we don't need to wait
	ret.Result = emu->Result;
	ret.RomName = TKPEmu::Testing::PassedTestMap.at(emu->RomHash).TestName;
	return ret;
}
void generate_results(TestDataVec& results) {
	std::sort(results.begin(), results.end(),
		[](auto& lres, auto& rres) -> bool {
			return lres.RomName < rres.RomName; 
		}
	);
	if (parameters.GenerationPath.empty()) {
		parameters.GenerationPath = std::filesystem::current_path().string() + "/results.md";
	}
	std::ofstream file(parameters.GenerationPath, std::ios::app);
	file << "| Test | " << last_emulator_name << " |\n";
	file << "| -- | -- |\n";
	for (const auto& r : results) {
		std::string emoji;
		switch (r.Result) {
			case TestResult::Passed: {
				emoji = ":+1:";
				break;
			}
			case TestResult::Failed: {
				emoji = ":x:";
				break;
			}
			case TestResult::Unknown: {
				emoji = ":question:";
				break;
			}
			case TestResult::None: {
				continue;
			}
		}
		file << "| " << r.RomName << " | " << emoji << " |\n";
	}
}