#version 330 core

layout(location = 0) out vec4 color;

uniform sampler2D tex;

in vec2 frag_uv;

void main()
{
    color = texture(tex, frag_uv);
}