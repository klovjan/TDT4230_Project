#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 modelPosition;

uniform layout(location = 6) int numLights;
uniform layout(location = 7) vec3 lightPos1;
uniform layout(location = 8) vec3 lightPos2;
uniform layout(location = 9) vec3 lightPos3;
uniform layout(location = 10) vec3 cameraPos;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 ambientColor = vec3(0.0f);
vec3 diffuseColor = vec3(0.0f);
vec3 specularColor = vec3(0.0f);
vec3 emitted = vec3(0.0f);

void main()
{
    vec3 normNormal;
    normNormal = normalize(normal);

    vec3 lightPos[] = {lightPos1, lightPos2, lightPos3};

    vec3 surfaceColor = vec3(0.7f, 0.7f, 0.7f);

    // Ambient
    ambientColor = vec3(0.1f, 0.1f, 0.1f);

    // Diffuse, Specular
    float totDiffuseIntensity = 0.0f;

    float totSpecularIntensity = 0.0f;
    const int specularFactor = 32;

    for (int i = 2; i < numLights; i++) {
        vec3 normLightDir = normalize(lightPos[i] - modelPosition);

        // Diffuse contribution
        float diffuseIntensity = dot(normLightDir, normNormal);

        if (diffuseIntensity < 0) {
            diffuseIntensity = 0;
        }

        totDiffuseIntensity += diffuseIntensity;

        // Specular contribution
        vec3 reflLightDir = reflect(-normLightDir, normNormal);
        vec3 normEyeDir = normalize(cameraPos);

        float specularIntensity = pow(dot(normLightDir, normEyeDir), specularFactor);

        if (specularIntensity < 0) {
            specularIntensity = 0;
        }

        totSpecularIntensity += specularIntensity;
    }

    diffuseColor = totDiffuseIntensity * surfaceColor;
    specularColor = totSpecularIntensity * vec3(1.0f);
    
    color = vec4(ambientColor + diffuseColor + specularColor, 1.0f);
}