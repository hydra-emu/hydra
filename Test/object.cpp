#include "object.h"

void Object::Update(char data) {

}

void Object::Draw(SDL_Renderer* gRenderer) {

}

bool Object::IsSelected() {
	return selected;
}

Type Object::GetType() {
	return type;
}

void Object::SetSelected(bool f) {
	selected = f;
}

bool Object::CheckMouse(int mouse_x, int mouse_y) {
	if (mouse_x >= rect.x && mouse_x < rect.x + rect.w && mouse_y >= rect.y && mouse_y < rect.y + rect.h) {
		return true;
	}
	return false;
}

void Object::Click() {
	SetSelected(true);
}