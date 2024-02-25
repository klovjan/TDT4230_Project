#version 430 core

#define MAX_LIGHTS 10

// Definitions corresponding to SceneNodeType enum
#define GEOMETRY 0
#define GEOMETRY_2D 1
#define NORMAL_MAPPED 2
#define POINT_LIGHT 3
#define SPOT_LIGHT 4

struct LightSource {
    vec3 coord;
    vec3 color;
};

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 modelPos;
in layout(location = 3) mat3 TBN;

uniform layout(location = 6) int numLights;
uniform layout(location = 10) vec3 eyePos;
uniform layout(location = 11) vec3 ballPos;
uniform layout(location = 12) float ballRadius;
uniform layout(location = 13) int renderMode;  // 0 --> 3D, 1 --> 2D
uniform layout(binding = 0) sampler2D colorSampler;
uniform layout(binding = 1) sampler2D normalMapSampler;
uniform layout(binding = 2) sampler2D roughnessMapSampler;
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
float specularFactor = 64.0f;  // Note: NOT const because of potential roughness maps

// Attenuation factor
float atten = 1.0f;

// Attenuation coefficients
float attenCoeffA = 0.009f;
float attenCoeffB = 0.001f;
float attenCoeffC = 0.001f;

// Dithering
vec3 noise = vec3(0.0f);

// Shadows
bool noShadow = false;
float softShadowBallRadius = ballRadius + 1.5;
float softShadowFactor = 1.0f;

// Surface color
vec3 surfaceColor = vec3(1.0f);

// Fragment normal, which may be different from input (interpolated) normal
// E.g. If the surface has a normal map
vec3 fragmentNormal = normal;
vec3 correctedFragmentNormal = vec3(1.0f);

// Roughness
float roughness = 64.0f;


void render3D()
{
    vec3 normNormal = normalize(fragmentNormal);  // Normalize interpolated normals

    // Vector from fragment to ball -- for ball shadows
    vec3 ballDir = ballPos - modelPos;

    // Ambient
    ambientColor = vec3(0.05f);

    // Diffuse, Specular
    for (int i = 0; i < 3; i++) {
        vec3 lightDir = lightSource[i].coord - modelPos;
        vec3 normLightDir = normalize(lightDir);

        // Ball shadows
        softShadowFactor = 1.0f;

        noShadow = false;
        if (length(lightDir) < length(ballDir) || dot(lightDir, ballDir) < 0.0f) {
            noShadow = true;
        }

        float rejectionLength = length(reject(ballDir, lightDir));
        if (rejectionLength < ballRadius && !noShadow) {
            // The light is obscured by the ball; jump to next light source
            continue;
        }
        else if (rejectionLength < softShadowBallRadius && !noShadow) {
            // The light ray is close enough to the ball; make soft shadow
            softShadowFactor = (rejectionLength - ballRadius) / (softShadowBallRadius - ballRadius);
        }

        // Attenuation
        float lightDistance = length(lightDir);
        atten = 1.0f / (attenCoeffA + lightDistance * attenCoeffB + pow(lightDistance, 2) * attenCoeffC);


        // Diffuse contribution
        float diffuseIntensity = max(dot(normNormal, normLightDir), 0.0f);

        totDiffuseIntensityRGB += diffuseIntensity * atten * lightSource[i].color * softShadowFactor;


        // Specular contribution
        vec3 normReflLightDir = reflect(-normLightDir, normNormal);
        vec3 normEyeDir = normalize(eyePos-modelPos);

        float specularIntensityRGB = pow(max(dot(normReflLightDir, normEyeDir), 0.0f), specularFactor);

        totSpecularIntensityRGB += specularIntensityRGB * atten * lightSource[i].color * softShadowFactor;
    }

    ambientColor = ambientColor * surfaceColor;
    diffuseColor = totDiffuseIntensityRGB * surfaceColor;
    specularColor = totSpecularIntensityRGB;

    // Dithering
    noise = vec3(dither(textureCoordinates));
    
    // Phong equation
    color = vec4(emittedColor + ambientColor + diffuseColor*diffuseCoeff + specularColor*specularCoeff + noise, 1.0f);

    //color = vec4(normNormal * 0.5f + vec3(0.5f), 1.0f);
}

void render2D() {
    color = texture(colorSampler, textureCoordinates);
}

void renderNormalMapped() {
    // Assign surface color and normal vector from textures
    surfaceColor = vec3(texture(colorSampler, textureCoordinates));
    fragmentNormal = TBN * (vec3(texture(normalMapSampler, textureCoordinates)) * 2 - 1);  // "* 2 - 1" takes us from [0, 1] to [-1, 1]

    // For roughness
    roughness = float(texture(roughnessMapSampler, textureCoordinates));
    specularFactor = 5.0f / pow(roughness, 2);

    // Apply 3D lighting
    render3D();
}

void main()
{   
    if (renderMode == GEOMETRY) {
        render3D();
    }
    else if (renderMode == GEOMETRY_2D) {
        render2D();
    }
    else if (renderMode == NORMAL_MAPPED) {
        renderNormalMapped();
    }
}