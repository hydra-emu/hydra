#version 150 core
in vec2 in_Vertex;
in vec2 vertTexCoord;
out vec2 fragTexCoord;
void main(void)
{
    gl_Position = vec4(in_Vertex, 0.0, 1.0);
    fragTexCoord = vertTexCoord;
}