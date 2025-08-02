#version 460

layout (location = 0) in vec2 basePosition;

void main()
{
    gl_Position = vec4(basePosition, 0.f, 1.f);
}
