#version 430 core

in layout(location = 0) vec2 textureCoordinates;

uniform sampler2D gColor;

out vec4 color;

void main() {
    //vec4 fragColor = texture(gColor, textureCoordinates);
    //color = vec4(normalize(vec3(textureCoordinates.x)), 1.0f);
}
