#define SDL_MAIN_HANDLED

#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <bitset>
#include <chrono>

#include "cpu.h"
#include "cartridge.h"
#include "disassembler.h"
#include "textbox.h"
#include "button.h"

#define BG_MAP0_START 0x9800
#define BG_MAP0_END 0x9C00
#define MAX_FPS 60

struct Timer
{
	int div, tma, tima, tac, cmain, csub, cdiv;
};

CPU c;
Timer tData;

bool pause = false;
bool quit = false;
bool find = false;
bool next = false;
int fps = 0;
int fps_a = 0;
int ftime = 0;
int ptime = 0;
int memstart = 0x9800;
int memend = 0x98FF;
int mouse_x, mouse_y;

Textbox* minAddr = new Textbox(400, 250, 77, 14, c.gpu->GetFont(), 7, "0x9800");
Textbox* maxAddr = new Textbox(493, 250, 77, 14, c.gpu->GetFont(), 7, "0x98FF");

std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
std::chrono::system_clock::time_point b = std::chrono::system_clock::now();

std::vector<Object*> objects;

void dispose();
void frame();
void fpscalc();
void timerinc(int mTemp);
void timerstep();

int main() {
	
	SDL_GL_SetSwapInterval(0);

	Button* addressShow = new Button(400, 270, 14, c.gpu->GetFont(), 7, "Show addresses", 
		[]() {
			std::stringstream ss;
			ss << std::hex << minAddr->GetText();
			ss >> memstart;
			ss.clear();
			ss << std::hex << maxAddr->GetText();
			ss >> memend;
		}
	);

	objects.push_back(minAddr);
	objects.push_back(maxAddr);
	objects.push_back(addressShow);
	c.cpuBus->Write(0xFF00, 0xFF); //TODO: Temporary input solution
	disassembler d;
	c.cpuBus->LoadCartridge("inter.gb");
	c.Reset();
	SDL_Event e;

	bool useLockTexture = false;

	while (true) {

		#pragma region Calculate duration to sleep
		a = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> work_time = a - b;

		if (work_time.count() < 16.75) {
			std::chrono::duration<double, std::milli> delta_ms(16.75 - work_time.count());
			auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
		}

		b = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = b - a;
	#pragma endregion

		fpscalc();

		if (!pause)
		frame();

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN) {
				bool inserted = false;
				for (auto o : objects) {
					if (o->IsSelected()) {
						switch (o->GetType()) {
							case Type::Textbox: {
								// Check if alphanumeric
								if (e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_z || e.key.keysym.sym == 0x8)
									o->Update(e.key.keysym.sym);
								inserted = true;
							}
						}
					}
				}
				if (!inserted) {
					switch (e.key.keysym.sym) {
						case SDLK_r:
							c.Reset();
						break;
						case SDLK_n:
							if (pause)
								next = true;
						break;
						case SDLK_f:
							c.IME = 1;
						break;
						case SDLK_p:
							pause = !pause;
						break;
						case SDLK_RETURN:
							c.cpuBus->WriteInput(e.key.keysym.sym);
						break;
					}
				}
			}
			else if (e.type == SDL_KEYUP) {
				switch (e.key.keysym.sym) {
					case SDLK_RETURN:
						//c.cpuBus->RemoveInput(e.key.keysym.sym);
					break;
				}
			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				SDL_GetMouseState(&mouse_x, &mouse_y);
				for (auto o : objects) {
					if (o->CheckMouse(mouse_x,mouse_y)) {
						o->Click();
					}
					else {
						o->SetSelected(false);
					}
				}
			}
		}
		
		if (pause && next) {
			next = false;
			c.Update();
		}
		
		// Draw block
		{
			c.gpu->PreDraw();

			std::stringstream ss;

			ss << "PC:0x" << std::hex << 0u + c.PC << "\nA:0x" << std::hex << 0u + c.A << "\nB:0x" << std::hex << 0u + c.B << "\nC:0x" << std::hex << 0u + c.C
				<< "\nD:0x" << std::hex << 0u + c.D << "\nE:0x" << std::hex << 0u + c.E << "\nH:0x" << std::hex << 0u + c.H << "\nL:0x" << std::hex << 0u + c.L
				<< "\nSP:0x" << std::hex << 0u + c.SP << "\nIE:" << std::bitset<8>(c.cpuBus->GetIE()) << "\nIF:" << std::bitset<8>(c.cpuBus->GetIF()) << "\nIME:" << c.IME;
			c.gpu->DrawMemoryArea(memstart,memend, 400, 300);
			std::string x(ss.str());
			c.gpu->DrawString(x, 530, 2);
			auto t = std::to_string(fps);
			t = "FPS:" + t;
			c.gpu->DrawString(t, 2, 2);

			if (c.cpuBus->backgroundMapChanged) {
				int i = 0;
				int xx = 0;
				int yy = 0;
				for (int j = BG_MAP0_START; j < BG_MAP0_END; j++) {
					if (i == 32) {
						i = 0;
						yy += 8;
						xx = 0;
					}
					c.gpu->FillTile(c.cpuBus->Read(j), xx, yy);
					xx += 8;
					i++;
				}
				c.cpuBus->backgroundMapChanged = false;
			}	

			if (c.cpuBus->spriteDataChanged) {
				//TODO: uncomment or delete
				int v = 0;
				c.gpu->spritePixels.fill(0);
				for (Sprite s : c.cpuBus->sprites) {
					if ((s.tileNumber | s.yPosition | s.xPosition | s.spriteFlags) != 0) {
						c.gpu->FillSprite(v);
					}
					v++;
				}
				c.cpuBus->spriteDataChanged = false;
				
			}
			c.gpu->DrawBackground0();
			c.gpu->DrawSprites();
			c.gpu->DrawGUI();
			c.gpu->DrawPalletes();
			for (auto o : objects) {
				o->Draw(c.gpu->GetRenderer());
			}
			c.gpu->PostDraw();
		}
	}

	dispose();

	return 0;
}

void dispose() {
	for (auto o : objects) {
		delete o;
	}
	objects.clear();
}

void frame() {
	int most = c.tClock + 70224;
	do {
		int mClockOld = c.mClock;
		c.Update();
		timerinc(mClockOld - c.mClock);
	} while (c.tClock < most);
}

void fpscalc() {
	ptime = SDL_GetTicks() - ptime;
	ftime += ptime;
	ptime = SDL_GetTicks();
	fps_a++;
	if (ftime >= 1000) {
		ftime = 0;
		fps = fps_a;
		fps_a = 0;
	}
}

void timerinc(int mTemp) {
	tData.div = c.cpuBus->Read(0xFF04);
	tData.tima = c.cpuBus->Read(0xFF05);
	tData.tma = c.cpuBus->Read(0xFF06);
	tData.tac = c.cpuBus->Read(0xFF07);

	int oldclk = tData.cmain;
	tData.csub += mTemp;
	if (tData.csub > 3) {
		tData.cmain++;
		tData.csub -= 4;
		tData.cdiv++;
		if (tData.cdiv == 16) {
			tData.cdiv = 0;
			c.cpuBus->Write(0xFF04, (tData.div + 1) & 0xFF);
		}
		if (tData.tac & 4) {
			switch (tData.tac & 3) {
			case 0:
				if (tData.cmain >= 64) timerstep();
			break;
			case 1:
				if (tData.cmain >= 1) timerstep();
			break;
			case 2:
				if (tData.cmain >= 4) timerstep();
			break;
			case 3:
				if (tData.cmain >= 16) timerstep();
			break;
			}
		}
	}
}

void timerstep() {
	tData.tima = c.cpuBus->Read(0xFF05) + 1;
	tData.cmain = 0;
	if (tData.tima > 0xFF) {
		c.cpuBus->Write(0xFF05, tData.tma);
		c.cpuBus->SetIF(c.cpuBus->GetIF() | 4);
	}
	else {
		c.cpuBus->Write(0xFF05, tData.tima);
	}
}