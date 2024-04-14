#version 430 core
in layout(location = 0) vec3 position;
in layout(location = 1) vec2 textureCoordinates_in;

out layout(location = 0) vec2 textureCoordinates_out;

void main()
{
    textureCoordinates_out = textureCoordinates_in;
    gl_Position = vec4(position, 1.0);
}
