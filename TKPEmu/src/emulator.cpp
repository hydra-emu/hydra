#include "../include/emulator.h"
namespace TKPEmu {
    constexpr int Emulator::GetPCHexCharSize() { 
		return 1; 
	}
    void Emulator::HandleKeyDown(SDL_Keycode keycode) { 
        std::cout << "Warning: Key " << SDL_GetKeyName(keycode) << " was pressed but\n"
        "emulator.HandleKeyDown was not implemented" << std::endl;
    }
    void Emulator::HandleKeyUp(SDL_Keycode keycode) { 
			std::cout << "Warning: Key " << SDL_GetKeyName(keycode) << " was released but\n"
			"emulator.HandleKeyUp was not implemented" << std::endl;
    }
    void Emulator::Screenshot(std::string filename) { 
        throw("Screenshot was not implemented for this emulator"); 
    };
    float* Emulator::GetScreenData() { 
        throw("GetScreenData was not implemented for this emulator");
    };
    std::string Emulator::print() const { 
        return "emulator.print() not implemented for this emulator";
    }
    std::ostream& operator<<(std::ostream& os, const Emulator& obj) {
    	return os << obj.print();
	}
}