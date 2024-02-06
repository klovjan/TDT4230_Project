#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 ambient;
vec3 diffuse;
vec3 specular;
vec3 emitted;

void main()
{
    vec3 normalizedNormal;
    normalizedNormal = normalize(normal);

    ambient = vec3(0.2f, 0.2f, 0.2f);

    color = vec4(0.5 * normalizedNormal + 0.5, 1.0);
    // color = vec4(ambient, 1.0f);
}