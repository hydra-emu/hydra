#ifndef BUTTON_H
#define BUTTON_H

#include "object.h"
#include <string>
#include "SDL_FontCache/SDL_FontCache.h"

class Button : public Object
{
private:
	std::string text;
	FC_Font* gFont;
	int characterWidth;
	int clickTimer;
	void(*funct)() = nullptr;

public:
	Button(int x, int y, int h, FC_Font* gFont, int characterWidth, std::string originalValue, void (*funct)());
	~Button();
	void Draw(SDL_Renderer* gRenderer) override;
	void Click() override;
};
#endif