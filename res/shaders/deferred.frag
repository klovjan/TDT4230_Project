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

float hit = 0.0f;
vec3 hitPos = vec3(0.0f);
vec3 hitNormal = vec3(0.0f);

void raycast(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float sphereRadius) {
        float t = 0.0f;
        vec3 L = sphereCenter - rayOrigin;
        float tca = dot(L, -rayDir);

        if (tca < 0) {
            hit = 0.0f;
            return;
        }

        float d2 = dot(L, L) - tca * tca;
        float sphereRadius2 = sphereRadius * sphereRadius;

        if (d2 > sphereRadius2) {
            hit = 0.0f;
            return;
        }

        float thc = sqrt(sphereRadius2 - d2);
        t = tca - thc;

        hit = 1.0f;
        hitPos = rayOrigin - rayDir * t;
        hitNormal = normalize(hitPos - bhPos);
}


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

        float bhShadowRadius = bhRadius / 2.0f;
        
//        raycast(eyePos, normalize(viewModelVector), bhPos, bhShadowRadius);
//        if (hit == 1.0f) {
//            color = vec4(vec3(0.0f), 1.0f);
//        }

        if (length(reject(viewModelVector, bhModelVector)) < bhShadowRadius) {
            color = vec4(vec3(0.0f), 1.0f);
        }
    }

    //color = vec4(normalVal, 1.0f);

    //color = vec4(vec3(stencilVal), 1.0f);

    //color = vec4(abs(normalize(modelPos)), 1.0f);

    // color = vec4(vec3((textureCoordinates.x+textureCoordinates.y) / 2), 1.0f);
}
