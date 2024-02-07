#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 3) mat4 modelMatrix;
uniform layout(location = 4) mat3 normalMatrix;
uniform layout(location = 5) mat4 MVP;
uniform layout(location = 6) vec3 lightPos1;
uniform layout(location = 7) vec3 lightPos2;
uniform layout(location = 8) vec3 lightPos3;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;

void main()
{
    normal_out = normalize(normalMatrix * normal_in);
    textureCoordinates_out = textureCoordinates_in;
    gl_Position = MVP * vec4(position, 1.0f);
}
