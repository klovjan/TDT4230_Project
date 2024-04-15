#version 430 core

in layout(location = 0) vec3 position;

uniform layout(location = 0) mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(position, 1.0f);
}