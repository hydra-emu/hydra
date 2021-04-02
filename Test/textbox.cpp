#include <algorithm>
#include "textbox.h"

Textbox::Textbox(int x, int y, int w, int h, FC_Font* gFont, int characterWidth, std::string originalValue) {
	Textbox::rect.x = x;
	Textbox::rect.y = y;
	Textbox::rect.w = w;
	Textbox::rect.h = h;
	Textbox::gFont = gFont;
	Textbox::characterWidth = characterWidth;
	maxChars = w / (characterWidth + 1);
	type = Type::Textbox;
	text = originalValue;
}

Textbox::~Textbox() {

}

void Textbox::Update(char c) {
	Object::Update(c);

	// Check if not backspace
	if (c != SDLK_BACKSPACE)
		text += c;
	else if (text.size() > 0)
		text = text.substr(0, text.size() - 1);
}

void Textbox::Draw(SDL_Renderer* gRenderer) {
	Object::Draw(gRenderer);

	cursorTimer++;
	if (cursorTimer > 60)
		cursorTimer = 0;
	SDL_SetRenderDrawColor(gRenderer, 0x0, 0x0, 0x0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(gRenderer, &rect);
	int length = text.length();
	int start = (length <= maxChars ? 0 : length - maxChars);
	int end = std::min(length, maxChars);
	int diff = std::max(maxChars - length, 0);
	FC_Draw(gFont, gRenderer, rect.x + 3, rect.y + 3, text.substr(start,end).c_str());
	if (IsSelected() && cursorTimer <= 30) {
		// Draw cursor
		SDL_RenderDrawLine(gRenderer, rect.x + rect.w - 3 -	diff * (characterWidth + 1), rect.y + 2, rect.x + rect.w - 3 - diff * (characterWidth + 1), rect.y + rect.h - 3);
	}
}

const std::string& Textbox::GetText() {
	return text;
}