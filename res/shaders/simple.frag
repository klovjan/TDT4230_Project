#version 430 core

#define MAX_LIGHTS 10

struct LightSource {
    vec3 coord;
    vec3 color;
};

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 modelPos;

uniform layout(location = 6) int numLights;
uniform layout(location = 7) vec3 lightPos0;
uniform layout(location = 8) vec3 lightPos1;
uniform layout(location = 9) vec3 lightPos2;
uniform layout(location = 10) vec3 eyePos;
uniform layout(location = 11) vec3 ballPos;
uniform layout(location = 12) float ballRadius;
uniform LightSource lightSource[MAX_LIGHTS];

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }
// vec3 reject(vec3 from, vec3 normOnto) { return from - normOnto*dot(from, normOnto); }
vec3 reject(vec3 from, vec3 onto) { return from - onto*(dot(from, onto)/dot(onto, onto)); }

// Phong intensities
vec3 totDiffuseIntensityRGB = vec3(0.0f, 0.0f, 0.0f);
vec3 totSpecularIntensityRGB = vec3(0.0f, 0.0f, 0.0f);

// Phong terms
vec3 ambientColor = vec3(0.0f);
vec3 diffuseColor = vec3(0.0f);
vec3 specularColor = vec3(0.0f);
vec3 emittedColor = vec3(0.0f);

// Phong coefficients
const float diffuseCoeff = 0.6f;
const float specularCoeff = 0.5f;
const int specularFactor = 80;

// Attenuation
float atten = 1.0f;
float attenCoeffA = 0.005f;
float attenCoeffB = 0.003f;
float attenCoeffC = 0.001f;

// Dithering
vec3 noise = vec3(0.0f);

// Constants
const vec3 surfaceColor = vec3(1.0f);

void main()
{   
    vec3 normNormal = normalize(normal);  // Normalize interpolated normals

//    vec3 lightPos[] = {lightPos0, lightPos1, lightPos2};

    // Vector from fragment to ball -- for ball shadows
    vec3 ballDir = ballPos - modelPos;

    // Ambient
    ambientColor = vec3(0.02f);

    // Diffuse, Specular
    for (int i = 0; i < 3; i++) {
        vec3 lightDir = lightSource[i].coord - modelPos;
        vec3 normLightDir = normalize(lightDir);

        // Ball shadows
        float rejectionLength = length(reject(ballDir, lightDir));
        if (rejectionLength < ballRadius) {
            if (length(lightDir) < length(ballDir) || dot(lightDir, ballDir) < 0.0f) {
                ;
            }
            else {
                continue;
            }
        }


        // Attenuation
        float lightDistance = length(lightDir);
        atten = 1.0f / (attenCoeffA + lightDistance * attenCoeffB + pow(lightDistance, 2) * attenCoeffC);


        // Diffuse contribution
        float diffuseIntensity = max(dot(normNormal, normLightDir), 0.0f);

        totDiffuseIntensityRGB += diffuseIntensity * atten * lightSource[i].color;


        // Specular contribution
        vec3 normReflLightDir = reflect(-normLightDir, normNormal);
        vec3 normEyeDir = normalize(eyePos-modelPos);

        float specularIntensityRGB = pow(max(dot(normReflLightDir, normEyeDir), 0.0f), specularFactor);

        totSpecularIntensityRGB += specularIntensityRGB * atten * lightSource[i].color;
    }

    ambientColor = ambientColor * surfaceColor;
    diffuseColor = totDiffuseIntensityRGB * surfaceColor;
    specularColor = totSpecularIntensityRGB;

    // Dithering
    noise = vec3(dither(textureCoordinates));
    
    color = vec4(emittedColor + ambientColor + diffuseColor*diffuseCoeff + specularColor*specularCoeff + noise, 1.0f);

    //color = vec4(normNormal * 0.5f + vec3(0.5f), 1.0f);
}