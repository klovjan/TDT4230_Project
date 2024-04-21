#version 430 core

// Definitions corresponding to ViewMode enum
#define REGULAR 0
#define NORMALS 1
#define POSITION 2
#define DISTANCE 3
#define STENCIL 4

in layout(location = 0) vec2 textureCoordinates;

uniform layout(location = 10) vec3 eyePos;
uniform layout(location = 14) vec3 bhPos;
uniform layout(location = 15) vec3 bhScreenPos;
uniform layout(location = 16) float bhRadius;
uniform layout(location = 17) float bhScreenPercent;
uniform layout(location = 18) vec2 screenDimensions;
uniform layout(location = 19) int viewMode;

uniform layout(binding = 0) sampler2D gColor;
uniform layout(binding = 1) sampler2D gPosition;
uniform layout(binding = 2) sampler2D gNormal;
uniform layout(binding = 3) sampler2D gStencil;

out vec4 color;

vec3 reject(vec3 from, vec3 onto) { return from - onto*(dot(from, onto)/dot(onto, onto)); }

float hit = 0.0f;
vec3 hitPos = vec3(0.0f);
vec3 hitNormal = vec3(0.0f);

//void raycast(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float sphereRadius) {
//    float t = 0.0f;
//    vec3 L = sphereCenter - rayOrigin;
//    float tca = dot(L, -rayDir);
//
//    if (tca < 0) {
//        hit = 0.0f;
//        return;
//    }
//
//    float d2 = dot(L, L) - tca * tca;
//    float sphereRadius2 = sphereRadius * sphereRadius;
//
//    if (d2 > sphereRadius2) {
//        hit = 0.0f;
//        return;
//    }
//
//    float thc = sqrt(sphereRadius2 - d2);
//    t = tca - thc;
//
//    hit = 1.0f;
//    hitPos = rayOrigin - rayDir * t;
//    hitNormal = normalize(hitPos - sphereCenter);
//}

void regularRender() {
    // Sample the textures
    vec4 modelColor = texture(gColor, textureCoordinates);
    vec3 modelPos = texture(gPosition, textureCoordinates).rgb;
    vec3 modelNormal = texture(gNormal, textureCoordinates).rgb;
    float stencilVal = texture(gStencil, textureCoordinates).r;
    
    color = modelColor;

    // If this pixel should be affected by the black hole ...
    if (stencilVal == 1.0f) {
        vec3 viewModelVector = eyePos - modelPos;
        vec3 bhModelVector = bhPos - modelPos;

        float bhEHRadius = bhRadius / 2.0f;  // Event horizon radius

        float distortion_simple = 1 - acos(dot(modelNormal, normalize(viewModelVector)));  // Note: modelNormal belongs to bhSphere wherever stencil is 1
        float rejectionLength = length(reject(viewModelVector, bhModelVector));

        if ((length(viewModelVector) < length(bhModelVector)) || (dot(viewModelVector, bhModelVector) < 0.0f)) {
            return;
        }

        if (distortion_simple > 0.75f) {
            color = vec4(vec3(0.0f), 1.0f);
        }
        else {
            vec2 screen_modelPos = textureCoordinates * screenDimensions;
            vec2 screen_modelBHVector = bhScreenPos.xy - screen_modelPos;
            float screen_modelBHDist = length(screen_modelBHVector);
            vec2 screen_modelBHVector_norm = normalize(screen_modelBHVector);

            float distortion = pow(max(distortion_simple + 0.1f, 0.0f), 3.0f);

            vec2 distortedUVSample = textureCoordinates + distortion * 0.5f * bhScreenPercent * screen_modelBHVector_norm;
            color = texture(gColor, distortedUVSample);
        }
    }
}

void main() {
    if (viewMode == REGULAR) {
        regularRender();
        return;
    }

    vec3 modelPos = texture(gPosition, textureCoordinates).rgb;
    vec3 modelNormal = texture(gNormal, textureCoordinates).rgb;
    float stencilVal = texture(gStencil, textureCoordinates).r;

    vec3 viewModelVector = eyePos - modelPos;

    if (viewMode == NORMALS) {
        //color = vec4(modelNormal, 1.0f);
        color = vec4(modelNormal * 0.5f + vec3(0.5f), 1.0f);
    }
    else if (viewMode == POSITION) {
        color = vec4(abs(normalize(modelPos)), 1.0f);
    }
    else if (viewMode == DISTANCE) {
        float viewModelDistance = length(viewModelVector);
        float viewModelDistance_norm = viewModelDistance / 1000.0f;  // Provided the frustum has depth 1000.0f, this is now in range (0, 1)

        color = vec4(vec3(1 - viewModelDistance_norm), 1.0f);
    }
    else if (viewMode == STENCIL) {
        color = vec4(vec3(stencilVal), 1.0f);
    }
}
