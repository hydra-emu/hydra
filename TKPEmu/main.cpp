#define SDL_MAIN_HANDLED
#define TKPEMU_VERSION_MAJOR 
#define TKPEMU_VERSION_MINOR 
#define TKPEMU_VERSION_PATCH 
#include "Display/display.h"
#include <iostream>

// Function for hashing a string in compile time in order to be used in a switch statement
// https://stackoverflow.com/a/46711735
// If there's a collision between two strings, we will know
// at compile time since the cases can't use the same number twice
constexpr uint32_t hash(const char* data) noexcept {
    uint32_t hash = 5381;
	const size_t size = strlen(data);
    for(const char *c = data; c < data + size; ++c)
        hash = ((hash << 5) + hash) + (unsigned char) *c;

    return hash;
}
void print_help() noexcept;

enum class ParameterType {
	File,
	Log
};

int main(int argc, char *argv[]) {
	std::string rom_file;
	// Bools that depend on the arguments passed
	bool display_mode = false;
	bool pre_open = false;
	bool fast_mode = false;
	bool log_mode = false;

	// The first argument is expected to be an option (eg. -d or -o)
	// Whenever we get an argument that needs parameters, this bool is set to true
	bool expects_parameter = false;
	ParameterType next_parameter_type;
	if (argc == 1) {
		std::cout << "No parameters specified. Try -h or --help." << std::endl;
		return 0;
	}
	for (int i = 1; i < argc; i++) {
		if (!expects_parameter) {
			switch (hash(argv[i])) {
				case hash("--display"):
				case hash("-d"): {
					display_mode = true;
					break;
				}
				case hash("--help"):
				case hash("-h"): {
					print_help();
					return 0;
				}
				case hash("--version"):
				case hash("-v"): {
					// Print version	
					//std::cout << "TKPEmu by OFFTKP. Version:" << TKPEMU_VERSION << std::endl;
					return 0;
				}
				case hash("--open"): 
				case hash("-o"): {
					pre_open = true;
					expects_parameter = true;
					next_parameter_type = ParameterType::File;
					break;
				}
				case hash("--fast-mode"):
				case hash("-f"): {
					fast_mode = true;
					break;
				}
				default: {
					std::cerr << "Error: Invalid parameter.\nUse -h or --help to get the parameter list." << std::endl;
					exit(127);
				}
			}
		} else {
			switch(next_parameter_type) {
				case ParameterType::File: {
					rom_file = argv[i];
					expects_parameter = false;
					break;
				}
			}
		}
	}
	if (display_mode) {
		std::cout << "Opening GUI..." << std::endl;
		TKPEmu::Graphics::Display dis;
		dis.EnterMainLoop();
	} else {
		// TODO: implement console mode. Both display and console (make console class) should take a RunParameters class
		// that explains fast_mode, log_mode and other settings. Display constructor takes this as param.
		// TODO: console mode must use -o, std::cerr otherwise
	}
	return 0;
}

void print_help() noexcept {
	std::cout << "Commands:\n" 
				"-h or --help: Shows this dialog\n"
				"-d or --display: Activates GUI mode\n"
				"-o or --open (filename): Open and start a rom\n"
				"-f or --fast-mode: Force emulator to run as fast as possible\n" 
		<< std::endl;
}
