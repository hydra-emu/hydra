#ifndef GPU_H
#define GPU_H
#include <SDL.h>
#include <vector>
#include <array>
#include "Bus/bus.h"

// Register locations in VRAM

#define LCD_CONTROL 0xFF40

// Specifies the position in the 256x256 pixels BG map (32x32 tiles) which is to be
// displayed at the upper / left LCD display position.
// Values in range from 0-255 may be used for X/Y each, the video controller automatically
// wraps back to the upper(left) position in BG map when drawing exceeds the lower
// (right) border of the BG map area.
#define SCROLLY 0xFF42
#define SCROLLX 0xFF43

// Indicates the vertical line to which the present data is transferred to the lcs driver. 
// Values: 0-153. 144-153 is the V-Blank period. Writing will reset the counter.
#define CURSCANLINE 0xFF44

// The gameboy permanently compares LYC and LY registers.
// If identical, the coincident bit in the STAT register becomes set and (if enabled) a STAT interrupt is requested.
#define LYC 0xFF45

// Specifies the upper/left positions of the Window area. (The window is an alternate
// background area which can be displayed above of the normal background.)
// OBJs (sprites) may be still displayed above or behind the window, just as for normal BG.
// The window becomes visible (if enabled) when positions are set in range WX = 0-166 and WY = 0-143
// Top left is WX = 7, WY = 0, which completely covers the normal background
#define WY 0xFF4A
#define WX 0xFF4B

// VRAM areas
#define TILE_DATA_START 0x8000
#define TILE_DATA_END 0x97FF

// Where to render the gameboy
#define RENDERX 50
#define RENDERY 50

#define GAMEBOY_WIDTH 160
#define GAMEBOY_HEIGHT 144

using TKP::Devices::Bus;
using TKP::Devices::Sprite;
class GPU
{
private:
	int screenWidth;
	int screenHeight;

	// 00 H-Blank
	// 01 V-Blank
	// 10 Searching OAM
	// 11 Transferring data to LCD Driver
	int mode = 0;
	int modeClock = 0;
	//int tileBase = 0x8000;

	std::shared_ptr<Bus> cpuBus = NULL;
	SDL_Window* gWindow = NULL;
	SDL_Renderer* gRenderer = NULL;
	SDL_Texture* backgroundTexture0 = NULL;
	SDL_Texture* spriteTexture = NULL;

	SDL_Rect bgView;
	SDL_Rect rect;

	// VRAM contents:
	// Background Tile Map (32x32)
	// Numbers of tiles to be displayed. Each byte contains a number of a tile to be displayed
	// Tiles taken from 0x8000 - 0x8FFF or 0x8800 - 0x97FF TILE_DATA_TABLE (tile pattern table)
	// First for sprites, bg, window disp. Second for bg and window disp.
	// In the first case, patterns are numberd with unumbers from 0 - 255 (pattern 0 lies at 0x8000)
	// In the second case, patterns have signed numbers from -128 - 127 (pattern 0 lies at 0x9000)
	// Tile Data Table address for the bg can be selected by setting the LCDC register

	// Two background tile maps: 0x9800 - 0x9BFF, 0x9C00 - 0x9FFF
	// Only one at a time
	// Can be selected by setting the LCDC register

	// There is a "window" overlaying the bg. Always displayed from top left.
	// Screen coordinates of top left corner of a window are WINDPOSX - 7, WINDPOSY

	// Both background and window can be disabled or enabled separately via bits in the LCDC register.
	// Tile images are stored in Tile Pattern Table

public:
	std::array<unsigned char, 0x100 * 0x100 * 4> backgroundPixels0;
	std::array<unsigned char, 0x100 * 0x100 * 4> spritePixels;

	GPU(std::shared_ptr<Bus>& cpuBus, int screenWidth = 640, int screenHeight = 480);
	~GPU();
	void Update(int mTemp);
	void Reset();
	void FillTile(int index, int x, int y);
	void FillSprite(int spriteIndex);
	void DrawBackground0();
	void DrawSprites();
	void DrawPalletes();
	void PreDraw();
	void PostDraw();

	// 0 = off, 1 = on
	bool GetBackgroundDisplay();
	bool GetBackgroundMap();
	bool GetBackgroundTile();
	bool GetSwitchLCD();
	int GetScrollY();
	int GetScrollX();
	int GetLine();
	bool GetObjOn();
	bool GetObjSize();
	bool GetWindowDisplayEnable();
	SDL_Renderer* GetRenderer();

	void SetBackgroundDisplay();
	void SetBackgroundMap();
	void SetBackgroundTile();
	void SetSwitchLCD();
	void SetObjOn();
	void SetObjSize();
	void SetScrollY(int value);
	void SetScrollX(int value);
	void SetLine(int value);
};
#endif