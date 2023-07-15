#version 330 core
uniform sampler2D tex;
in vec2 fragTexCoord;
out vec4 fragColor;
void main(void)
{
    fragColor = texture(tex,fragTexCoord);
}