#pragma once
#ifndef TKP_TOOLS_IMAGE_H
#define TKP_TOOLS_IMAGE_H
#include "../lib/imgui.h"
namespace TKPEmu::Tools {
    struct TKPImage {
        unsigned int texture = -1;
        int width = 0;
        int height = 0;
        ImVec2 topleft;
        ImVec2 botright;
    };
}
#endif
