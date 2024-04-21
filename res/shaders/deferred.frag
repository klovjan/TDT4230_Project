#version 430 core

in layout(location = 0) vec2 textureCoordinates;

uniform layout(location = 20) vec3 eyePos;
uniform layout(location = 14) vec3 bhPos;
uniform layout(location = 15) vec3 bhScreenPos;
uniform layout(location = 16) float bhRadius;
uniform layout(location = 17) float bhScreenPercent;
uniform layout(location = 18) vec2 screenDimensions;

uniform layout(binding = 0) sampler2D gColor;
uniform layout(binding = 1) sampler2D gPosition;
uniform layout(binding = 2) sampler2D gNormal;
uniform layout(binding = 3) sampler2D gStencil;

out vec4 color;

vec3 reject(vec3 from, vec3 onto) { return from - onto*(dot(from, onto)/dot(onto, onto)); }

float hit = 0.0f;
vec3 hitPos = vec3(0.0f);
vec3 hitNormal = vec3(0.0f);

void main() {
    // Sample the textures
    vec4 modelColor = texture(gColor, textureCoordinates);
    vec3 modelPos = texture(gPosition, textureCoordinates).rgb;
    vec3 modelNormal = texture(gNormal, textureCoordinates).rgb;
    float stencilVal = texture(gStencil, textureCoordinates).r;
    
    color = modelColor;

    // If this pixel should be affected by the black hole ...
    if (stencilVal == 1.0f) {
        color = clamp(vec4(modelColor.r+0.4f, modelColor.gba), 0.0f, 1.0f);

        vec3 viewModelVector = eyePos - modelPos;
        vec3 bhModelVector = bhPos - modelPos;

        float bhEHRadius = bhRadius / 2.0f;  // Event horizon radius

        float distortion_simple = 1 - acos(dot(modelNormal, normalize(viewModelVector)));

        if (distortion_simple > 0.5) {
            color = vec4(vec3(0.0f), 1.0f);
        }
        else {
            color = vec4(vec3(distortion_simple), 1.0f);
            
            vec2 screen_modelPos = textureCoordinates * screenDimensions;
            vec2 screen_modelBHVector = bhScreenPos.xy - screen_modelPos;
            vec2 screen_modelBHVector_norm = normalize(screen_modelBHVector);

            float distortion = min(distortion_simple, 0.0f);

            vec2 distortedUVSample = textureCoordinates + distortion * screen_modelBHVector_norm;
            color = texture(gColor, distortedUVSample);
        }

//        if (length(reject(viewModelVector, bhModelVector)) < bhShadowRadius) {
//            color = vec4(vec3(0.0f), 1.0f);
//        }
    }
//    vec3 viewModelVector = eyePos - modelPos;
//
//    color = vec4(normalize(viewModelVector), 1.0f);

    //color = vec4(normalVal, 1.0f);

    //color = vec4(vec3(stencilVal), 1.0f);

//    color = vec4(abs(normalize(modelPos)), 1.0f);

    // color = vec4(vec3((textureCoordinates.x+textureCoordinates.y) / 2), 1.0f);
}
