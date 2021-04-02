#include <sstream>
#include <iomanip>
#include "gpu.h"

GPU::GPU(std::shared_ptr<Bus>& cpuBus, int screenWidth, int screenHeight) {
	GPU::screenWidth = screenWidth;
	GPU::screenHeight = screenHeight;
	GPU::cpuBus = cpuBus;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
	}
	else {
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("GameboySoni", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
			}
		}
	}
	// Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
	}
	rect.x = 0;
	rect.y = 0;
	rect.w = screenWidth;
	rect.h = screenHeight;
	bgView.w = GAMEBOY_WIDTH;
	bgView.h = GAMEBOY_HEIGHT;
	gFont = FC_CreateFont();
	FC_LoadFont(gFont, gRenderer, "PressStart2P-vaV7.ttf", 8, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL);
	//backgroundPixels0 = std::array<unsigned char>(0x100 * 0x100 * 4, 0);
	backgroundTexture0 = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 0xFF, 0xFF);
	//spritePixels = std::vector<unsigned char>(0x100 * 0x100 * 4, 0);
	spriteTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 0xFF, 0xFF);
	SDL_SetTextureBlendMode(spriteTexture, SDL_BLENDMODE_BLEND);
}

GPU::~GPU() {
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	FC_FreeFont(gFont);
	gFont = NULL;
	TTF_Quit();
	SDL_Quit();
}

void GPU::Reset() {

}

void GPU::Update(int mTemp) {
	modeClock += mTemp * 4;
	cpuBus->Write(0xFF41, mode);
	switch (mode) {
		case 0:
		{
			cpuBus->canWriteToOam = true;
			if (modeClock >= 204) {
				modeClock = 0;

				if (GetLine() == 143) {
					mode = 1;
					// TODO: putimagedata here instead
					int ift = cpuBus->GetIF();
					ift |= 1;
					cpuBus->SetIF(ift);
				}
				else {
					mode = 2;
				}
				SetLine(GetLine() + 1);
			}
			break;
		}
		case 1: 
		{
			cpuBus->canWriteToOam = true;
			if (modeClock >= 456) {
				modeClock = 0;
				SetLine(GetLine() + 1);
				if (GetLine() > 153) {
					mode = 2;
					SetLine(0);
				}
			}
			break;
		}
		case 2: 
		{
			cpuBus->canWriteToOam = false;
			if (modeClock >= 80) {
				modeClock = 0;
				mode = 3;
			}
		}
		case 3: 
		{
			cpuBus->canWriteToOam = false;
			if (modeClock >= 172) {
				modeClock = 0;
				mode = 0;
			}
		}
	}
}

#pragma region Drawing functions
void GPU::PreDraw() {
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(gRenderer);
}

void GPU::PostDraw() {
	/*
	currentTime = SDL_GetTicks();
	auto diff = currentTime - lastTime;
	lastTime = SDL_GetTicks();*/
	SDL_RenderPresent(gRenderer);
}

void GPU::DrawGUI() {
	SDL_SetRenderDrawColor(gRenderer, 0x0, 0x0, 0x0, 0xFF);
	bgView.x = GetScrollX() + RENDERX;
	bgView.y = GetScrollY() + RENDERY;
	SDL_RenderDrawRect(gRenderer, &rect);
	SDL_RenderDrawRect(gRenderer, &bgView);
}

void GPU::DrawString(std::string& stringToDraw, int x, int y) {
	FC_Draw(gFont, gRenderer, x, y, stringToDraw.c_str());
}

void GPU::DrawMemoryArea(int startAddress, int endAddress, int x, int y) {
	std::stringstream ss;
	for (int i = startAddress; i <= endAddress; i++) {
		int num = cpuBus->Read(i);
		if (i % 16 == 0)
			ss << "\n";
		if (num < 16)
			ss << "0";
		ss << std::uppercase << std::hex << num << " ";
	}
	FC_Draw(gFont, gRenderer, x, y, ss.str().c_str());
}

void GPU::DrawBackground0() {
	SDL_Rect rect;
	rect.x = RENDERX;
	rect.y = RENDERY;
	rect.w = 0xff;
	rect.h = 0xff;
	SDL_UpdateTexture
	(
		backgroundTexture0,
		NULL,
		backgroundPixels0.data(),
		0xFF * 4
	);
	SDL_RenderCopy(gRenderer, backgroundTexture0, NULL, &rect);
}

void GPU::DrawSprites() {
	SDL_Rect rect;
	rect.x = RENDERX;
	rect.y = RENDERY;
	rect.w = 0xff;
	rect.h = 0xff;
	SDL_UpdateTexture
	(
		spriteTexture,
		NULL,
		spritePixels.data(),
		0xFF * 4
	);
	SDL_RenderCopy(gRenderer, spriteTexture, NULL, &rect);
}

void GPU::DrawPalletes() {
	SDL_Rect recta;
	recta.x = 100;
	recta.y = 400;
	recta.w = 32;
	recta.h = 32;

	for (int i = 0; i < 4; i++) {
		SDL_SetRenderDrawColor(gRenderer, cpuBus->backgroundPalette[i].r, cpuBus->backgroundPalette[i].g , cpuBus->backgroundPalette[i].b, 0xFF);
		SDL_RenderFillRect(gRenderer, &recta);
		recta.x += 32;
	}
	recta.y += 32;
	recta.x = 100;
	for (int i = 0; i < 3; i++) {
		SDL_SetRenderDrawColor(gRenderer, cpuBus->obj0Palette[i].r, cpuBus->obj0Palette[i].g, cpuBus->obj0Palette[i].b, 0xFF);
		SDL_RenderFillRect(gRenderer, &recta);
		recta.x += 32;
	}
	recta.y += 32;
	recta.x = 100;
	for (int i = 0; i < 3; i++) {
		SDL_SetRenderDrawColor(gRenderer, cpuBus->obj1Palette[i].r, cpuBus->obj1Palette[i].g, cpuBus->obj1Palette[i].b, 0xFF);
		SDL_RenderFillRect(gRenderer, &recta);
		recta.x += 32;
	}
}

void GPU::FillTile(int tileIndex, int x, int y) {
	int base = GetBackgroundTile() ? 0x8000 : 0x8800;
	if (base == 0x8800){
		if (tileIndex >= 0x80) {
			tileIndex -= 0x80;
		}
		else {
			tileIndex += 0x80;
		}
	}
	int i = base + tileIndex * 16;
	int l = (x + y * 0xff) * 4;
	for (int k = i; k < i + 16; k += 2) {
		int byte1 = cpuBus->Read(k);
		int byte2 = cpuBus->Read(k + 1);
		for (int j = 0; j < 8; j++) {
			int _tileIndex = (((byte2 >> (7 - j)) & 1) << 1) | ((byte1 >> (7 - j)) & 1);
			backgroundPixels0[l] = cpuBus->backgroundPalette[_tileIndex].b;
			backgroundPixels0[l + 1] = cpuBus->backgroundPalette[_tileIndex].g;
			backgroundPixels0[l + 2] = cpuBus->backgroundPalette[_tileIndex].r;
			backgroundPixels0[l + 3] = SDL_ALPHA_OPAQUE;
			l += 4;
		}
		l -= 0x8 * 4;
		l += 0xff * 4;
	}
}

void GPU::FillSprite(int spriteIndex) {
	Sprite* s = &cpuBus->sprites[spriteIndex];
	int x = ((s->xPosition - 8) & 0xFF) + GetScrollX();
	int y = ((s->yPosition - 16) & 0xFF) + GetScrollY(); 
	if (y >= GAMEBOY_HEIGHT)
		return;
	if (x >= GAMEBOY_WIDTH)
		return;
	
	int _tileIndex = s->tileNumber & 0xFF;
	int i = (0x8000 + _tileIndex * 16) & (0xFFFF);
	int l = ((x + y * 0xff) * 4);

	bool hflip = (s->spriteFlags & 0x20);
	bool vflip = (s->spriteFlags & 0x40);
	bool usepal1 = (s->spriteFlags & 0x10);

	int jInitial = 0;
	int jMax = 8;
	int jInc = 1;

	int kInitial = i;
	int kMax = i + 16;
	int kInc = 2;

	if (hflip) {
		jInitial = 7;
		jMax = -1;
		jInc = -1;
	}

	if (vflip) {
		kInitial = i + 14;
		kMax = i - 2;
		kInc = -2;
	}


	for (int k = kInitial; k != kMax; k += kInc) {
		int byte1 = cpuBus->Read(k);
		int byte2 = cpuBus->Read(k + 1);
		for (int j = jInitial; j != jMax; j += jInc) {
			int tileIndex = (((byte2 >> 7 - j) & 1) << 1) | ((byte1 >> 7 - j) & 1);
			tileIndex--;
			if (tileIndex != -1) {
				if (!usepal1) {
					spritePixels[l] = cpuBus->obj0Palette[tileIndex].b;
					spritePixels[l + 1] = cpuBus->obj0Palette[tileIndex].g;
					spritePixels[l + 2] = cpuBus->obj0Palette[tileIndex].r;
					spritePixels[l + 3] = SDL_ALPHA_OPAQUE;
				}
				else {
					spritePixels[l] = cpuBus->obj1Palette[tileIndex].b;
					spritePixels[l + 1] = cpuBus->obj1Palette[tileIndex].g;
					spritePixels[l + 2] = cpuBus->obj1Palette[tileIndex].r;
					spritePixels[l + 3] = SDL_ALPHA_OPAQUE;
				}
			}
			else {
				spritePixels[l] = 0;
				spritePixels[l + 1] = 0;
				spritePixels[l + 2] = 0;
				spritePixels[l + 3] = 0;
			}
			l += 4;
		}
		l -= 0x8 * 4;
		l += 0xff * 4;
	}
}
#pragma endregion

#pragma region Getters
bool GPU::GetBackgroundDisplay() {
	return (cpuBus->Read(LCD_CONTROL) & 0x1);
}

// BG Tile Map Display Select 
bool GPU::GetBackgroundMap() {
	return (cpuBus->Read(LCD_CONTROL) & 0x8);
}

bool GPU::GetBackgroundTile() {
	return (cpuBus->Read(LCD_CONTROL) & 0x10);
}

bool GPU::GetSwitchLCD() {
	return (cpuBus->Read(LCD_CONTROL) & 0x80);
}

bool GPU::GetObjOn() {
	return (cpuBus->Read(LCD_CONTROL) & 0x2);
}

bool GPU::GetObjSize() {
	return (cpuBus->Read(LCD_CONTROL) & 0x4);
}

bool GPU::GetWindowDisplayEnable() {
	return (cpuBus->Read(LCD_CONTROL) & 0x20);
}

int GPU::GetScrollY() {
	return cpuBus->Read(SCROLLY);
}

int GPU::GetScrollX() {
	return cpuBus->Read(SCROLLX);
}

int GPU::GetLine() {
	return cpuBus->Read(CURSCANLINE);
}

FC_Font* GPU::GetFont() {
	return gFont;
}

SDL_Renderer* GPU::GetRenderer() {
	return gRenderer;
}
#pragma endregion

#pragma region Setters
void GPU::SetBackgroundDisplay() {
	int temp = cpuBus->Read(LCD_CONTROL);
	temp |= 0x1;
	cpuBus->Write(LCD_CONTROL, temp);
}

void GPU::SetBackgroundMap() {
	int temp = cpuBus->Read(LCD_CONTROL);
	temp |= 0x8;
	cpuBus->Write(LCD_CONTROL, temp);
}

void GPU::SetBackgroundTile() {
	int temp = cpuBus->Read(LCD_CONTROL);
	temp |= 0x10;
	cpuBus->Write(LCD_CONTROL, temp);
}

void GPU::SetSwitchLCD() {
	int temp = cpuBus->Read(LCD_CONTROL);
	temp |= 0x80;
	cpuBus->Write(LCD_CONTROL, temp);
}

void GPU::SetObjOn() {
	int temp = cpuBus->Read(LCD_CONTROL);
	temp |= 0x2;
	cpuBus->Write(LCD_CONTROL, temp);
}

void GPU::SetObjSize() {
	int temp = cpuBus->Read(LCD_CONTROL);
	temp |= 0x4;
	cpuBus->Write(LCD_CONTROL, temp);
}

void GPU::SetScrollY(int value) {
	cpuBus->Write(SCROLLY, value);
}

void GPU::SetScrollX(int value) {
	cpuBus->Write(SCROLLX, value);
}

void GPU::SetLine(int value) {
	cpuBus->Write(CURSCANLINE, value);
}
#pragma endregion