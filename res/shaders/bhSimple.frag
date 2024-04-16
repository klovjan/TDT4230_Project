#version 430 core

out layout(location = 3) vec4 gStencil;

void main()
{
    gStencil = vec4(0.0f);
}