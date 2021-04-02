#include "button.h"

Button::Button(int x, int y, int h, FC_Font* gFont, int characterWidth, std::string originalValue, void (*funct)()) {
	Button::rect.x = x;
	Button::rect.y = y;
	Button::rect.w = originalValue.length() * (characterWidth + 2) + 1;
	Button::rect.h = h;
	Button::gFont = gFont;
	Button::characterWidth = characterWidth;
	text = originalValue;
	Button::funct = funct;
}

Button::~Button() {

}

void Button::Draw(SDL_Renderer* gRenderer) {
	Object::Draw(gRenderer);
	
	if (clickTimer > 20) {
		clickTimer = 0;
	}
	else if (clickTimer > 0) {
		clickTimer++;
	}

	if (clickTimer == 0)
		SDL_SetRenderDrawColor(gRenderer, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
	else
		SDL_SetRenderDrawColor(gRenderer, 0x5, 0x5f, 0x5, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(gRenderer, &rect);

	if (clickTimer == 0)
		SDL_SetRenderDrawColor(gRenderer, 0x0, 0x0, 0x0, SDL_ALPHA_OPAQUE);
	else
		SDL_SetRenderDrawColor(gRenderer, 0x5f, 0x10, 0x10, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(gRenderer, &rect);

	FC_Draw(gFont, gRenderer, rect.x + 3, rect.y + 3, text.c_str());
}

void Button::Click() {
	Object::Click();
	
	if (clickTimer == 0) {
		clickTimer = 1;
		funct();
	}
}