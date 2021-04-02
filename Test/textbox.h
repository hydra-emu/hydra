#ifndef TEXTBOX_H
#define TEXTBOX_H
#include <string>
#include "object.h"
#include "SDL.h"
#include "SDL_FontCache/SDL_FontCache.h"
class Textbox : public Object
{
private:
	std::string text;
	FC_Font* gFont;
	bool selected = false;
	int characterWidth;
	int maxChars;
	int cursorTimer = 0;
public:
	Textbox(int x, int y, int w, int h, FC_Font* gFont, int characterWidth, std::string originalValue = "");
	~Textbox();
	void Update(char data) override;
	void Draw(SDL_Renderer* gRenderer) override;
	const std::string& GetText();
};
#endif
