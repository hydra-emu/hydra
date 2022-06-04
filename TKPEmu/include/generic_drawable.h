#pragma once
#ifndef TKP_GENERIC_DRAWABLE_H
#define TKP_GENERIC_DRAWABLE_H
namespace TKPEmu::Applications {
    // Holds the needed public methods to pass
    // the generic drawable windows around
    // as a Drawable*
    class Drawable {
    public:
        virtual void Draw() = 0;
        virtual void Reset() = 0;
        virtual bool* IsDrawing() = 0;
        virtual const char* GetName() {
            return "Invalid-Name";
        };
    };
}
#endif