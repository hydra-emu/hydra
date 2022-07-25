#version 150 core
uniform sampler2D tex;
in vec2 fragTexCoord;
void main(void)
{
    gl_FragColor = texture2D(tex,fragTexCoord);
}