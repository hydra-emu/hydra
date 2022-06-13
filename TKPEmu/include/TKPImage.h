#pragma once
#ifndef TKP_TOOLS_IMAGE_H
#define TKP_TOOLS_IMAGE_H
#include <GL/glew.h>
#include "../imgui/imgui.h"
namespace TKPEmu::Tools {
    struct TKPImage {
        GLuint texture = 0;
        GLuint width = 0;
        GLuint height = 0;
        GLuint format = GL_RGBA;
        GLuint type = GL_FLOAT;
        ImVec2 topleft;
        ImVec2 botright;
    };
}
#endif
