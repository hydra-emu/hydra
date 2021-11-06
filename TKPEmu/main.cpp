#define SDL_MAIN_HANDLED
#include "Display/display.h"

int main() {
	using TKPEmu::Graphics::Display;
	Display d;
	d.EnterMainLoop();
	return 0;
}