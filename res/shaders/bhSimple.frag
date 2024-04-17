#version 430 core

out layout(location = 3) vec4 stencilBuffer;

void main()
{
    stencilBuffer = vec4(1.0f);
}