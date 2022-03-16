#pragma once
#ifndef TKP_TOOLS_IMAGE_H
#define TKP_TOOLS_IMAGE_H
#include "../imgui/imgui.h"
namespace TKPEmu::Tools {
    struct TKPImage {
        unsigned int texture = 0;
        int width = 0;
        int height = 0;
        ImVec2 topleft;
        ImVec2 botright;
    };
}
#endif
