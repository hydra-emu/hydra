#ifndef OBJECT_H
#define OBJECT_H
#include <SDL.h>

enum class Type {
	Unknown = -1,
	Textbox = 0,
};

class Object 
{
protected:
	Type type = Type::Unknown;
	bool selected = false;
	SDL_Rect rect;
public:
	virtual void Update(char data);
	virtual void Draw(SDL_Renderer* gRenderer);
	virtual bool IsSelected();
	virtual Type GetType();
	virtual void SetSelected(bool f);
	virtual bool CheckMouse(int mouse_x, int mouse_y);
	virtual void Click();
};
#endif