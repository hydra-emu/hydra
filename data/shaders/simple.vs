#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;

out vec2 frag_uv;

void main()
{
    gl_Position = vec4(pos, 0.0, 1.0);
    frag_uv = uv;
    // I hate this but whatever
    frag_uv.y = 1.0 - frag_uv.y;
}