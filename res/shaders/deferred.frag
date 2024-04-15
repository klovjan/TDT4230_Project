#version 430 core

in layout(location = 0) vec2 textureCoordinates;

uniform sampler2D gColor;

out vec4 color;

void main() {
    color = texture(gColor, textureCoordinates);
    //color = vec4(vec3((textureCoordinates.x+textureCoordinates.y) / 2), 1.0f);
}