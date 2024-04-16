#version 430 core

in layout(location = 0) vec2 textureCoordinates;

uniform layout(binding = 0) sampler2D gColor;
uniform layout(binding = 1) sampler2D gPosition;
uniform layout(binding = 2) sampler2D gNormal;

out vec4 color;

void main() {
    color = texture(gColor, textureCoordinates);
    vec3 position = texture(gPosition, textureCoordinates).rgb;
    vec3 normal = texture(gNormal, textureCoordinates).rgb;

    // color = vec4(normal, 1.0f);
    // color = vec4(position, 1.0f);

    // color = vec4(vec3((textureCoordinates.x+textureCoordinates.y) / 2), 1.0f);
}
