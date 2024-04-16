#version 430 core

in layout(location = 0) vec2 textureCoordinates;

uniform layout(binding = 0) sampler2D gColor;
uniform layout(binding = 1) sampler2D gPosition;
uniform layout(binding = 2) sampler2D gNormal;
uniform layout(binding = 3) sampler2D gStencil;

out vec4 color;

void main() {
    vec4 colorVal = texture(gColor, textureCoordinates);
    vec3 positionVal = texture(gPosition, textureCoordinates).rgb;
    vec3 normalVal = texture(gNormal, textureCoordinates).rgb;
    float stencilVal = texture(gStencil, textureCoordinates).r;
    
    color = colorVal;

    if (stencilVal == 0.0f) {
        color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }

    // color = vec4(normalize(position), 1.0f);
    // color = vec4(position, 1.0f);

    // color = vec4(vec3((textureCoordinates.x+textureCoordinates.y) / 2), 1.0f);
}
