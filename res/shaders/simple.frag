#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 modelPos;

uniform layout(location = 6) int numLights;
uniform layout(location = 7) vec3 lightPos1;
uniform layout(location = 8) vec3 lightPos2;
uniform layout(location = 9) vec3 lightPos3;
uniform layout(location = 10) vec3 eyePos;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 ambientColor = vec3(0.0f);
vec3 diffuseColor = vec3(0.0f);
vec3 specularColor = vec3(0.0f);
vec3 emittedColor = vec3(0.0f);

float totDiffuseIntensity = 0.0f;
float totSpecularIntensity = 0.0f;

// Constants
const vec3 surfaceColor = vec3(1.0f);

const float diffuseCoeff = 0.9f;
const float specularCoeff = 0.9f;
const int specularFactor = 80;

void main()
{
    vec3 normNormal = normalize(normal);
    vec3 lightPos[] = {lightPos1, lightPos2, lightPos3};

    // Ambient
    ambientColor = vec3(0.1f, 0.1f, 0.1f);

    // Diffuse, Specular
    for (int i = 2; i < numLights; i++) {
        vec3 normLightDir = normalize(lightPos[i] - modelPos);

        // Diffuse contribution
        float diffuseIntensity = max(dot(normNormal, normLightDir), 0.0f);

        totDiffuseIntensity += diffuseIntensity;

        // Specular contribution
        vec3 reflLightDir = reflect(-normLightDir, normNormal);
        vec3 normEyeDir = normalize(eyePos-modelPos);

        float specularIntensity = max(pow(dot(reflLightDir, normEyeDir), specularFactor), 0.0f);

        totSpecularIntensity += specularIntensity;
    }

    ambientColor = ambientColor * surfaceColor;
    diffuseColor = totDiffuseIntensity * surfaceColor;
    specularColor = totSpecularIntensity * surfaceColor;
    
    color = vec4(emittedColor + ambientColor + diffuseColor*diffuseCoeff + specularColor*specularCoeff, 1.0f);

    //color = vec4(normNormal * 0.5f + vec3(0.5f), 1.0f);
}