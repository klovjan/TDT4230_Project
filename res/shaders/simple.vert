#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent_in;
in layout(location = 4) vec3 bitangent_in;

uniform layout(location = 3) mat4 modelMatrix;
uniform layout(location = 4) mat3 normalMatrix;
uniform layout(location = 5) mat4 MVP;
uniform layout(location = 13) int renderMode;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec3 modelPos;
out layout(location = 3) mat3 TBN;
out layout(location = 10) vec3 tangent_out;

void main()
{
    normal_out = normalize(normalMatrix * normal_in);

    textureCoordinates_out = textureCoordinates_in;

    modelPos = vec3(modelMatrix * vec4(position, 1.0f));
    
    TBN = mat3(
        normalize(mat3(modelMatrix) * tangent_in),
        normalize(mat3(modelMatrix) * bitangent_in),
        normalize(mat3(modelMatrix) * normal_in)
    );

    gl_Position = MVP * vec4(position, 1.0f);
}