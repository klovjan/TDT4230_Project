#version 430 core

in layout(location = 0) vec2 textureCoordinates;

uniform layout(location = 10) vec3 eyePos;
uniform layout(location = 14) vec3 bhPos;
uniform layout(location = 15) vec2 bhNdcPos;
uniform layout(location = 16) float bhRadius;

uniform layout(binding = 0) sampler2D gColor;
uniform layout(binding = 1) sampler2D gPosition;
uniform layout(binding = 2) sampler2D gNormal;
uniform layout(binding = 3) sampler2D gStencil;

out vec4 color;

vec3 reject(vec3 from, vec3 onto) { return from - onto*(dot(from, onto)/dot(onto, onto)); }

void main() {
    // Sample the textures
    vec4 colorVal = texture(gColor, textureCoordinates);
    vec3 positionVal = texture(gPosition, textureCoordinates).rgb;
    vec3 normalVal = texture(gNormal, textureCoordinates).rgb;
    float stencilVal = texture(gStencil, textureCoordinates).r;
    
    color = colorVal;

    // If this pixel should be affected by the black hole ...
    if (stencilVal == 1.0f) {
        color = clamp(vec4(colorVal.r+0.4f, colorVal.gba), 0.0f, 1.0f);

        vec3 normEyeDir = normalize(eyePos - positionVal);
        vec3 normBHDir = normalize(eyePos - bhPos);
//        if (length(reject(normEyeDir, normBHDir)) < bhRadius - 10) {
//            color = vec4(vec3(0.0f), 1.0f);
//        }
    }

    //color = vec4(normalVal, 1.0f);

    //color = vec4(stencilVal, 1.0f);

    // color = vec4(normalize(position), 1.0f);
    // color = vec4(position, 1.0f);

    // color = vec4(vec3((textureCoordinates.x+textureCoordinates.y) / 2), 1.0f);
}
