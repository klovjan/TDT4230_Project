#include <iostream>
#include "glfont.h"

Mesh generateTextGeometryBuffer(std::string text, float characterHeightOverWidth, float totalTextWidth) {
    float characterWidth = totalTextWidth / float(text.length());
    float characterHeight = characterHeightOverWidth * characterWidth;

    // Hardcoded for now
    // Could be inferred from the texture, but that would require passing it to this function
    float texCharacterWidth = 29.0f/3712.0f;
    float texCharacterHeight = 1.0f;

    unsigned int vertexCount = 4 * text.length();
    unsigned int indexCount = 6 * text.length();
    unsigned int texCoordCount = 4 * text.length();

    Mesh mesh;

    mesh.vertices.resize(vertexCount);
    mesh.indices.resize(indexCount);
    mesh.textureCoordinates.resize(texCoordCount);

    for(unsigned int i = 0; i < text.length(); i++)
    {
        float baseXCoordinate = float(i) * characterWidth;

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};
        mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};


        mesh.indices.at(6 * i + 0) = 4 * i + 0;
        mesh.indices.at(6 * i + 1) = 4 * i + 1;
        mesh.indices.at(6 * i + 2) = 4 * i + 2;
        mesh.indices.at(6 * i + 3) = 4 * i + 0;
        mesh.indices.at(6 * i + 4) = 4 * i + 2;
        mesh.indices.at(6 * i + 5) = 4 * i + 3;


        float baseTexXCoordinate = int(text.at(i)) / 128.0f;
        
        mesh.textureCoordinates.at(4 * i + 0) = glm::vec2(baseTexXCoordinate, 0.0f);
        mesh.textureCoordinates.at(4 * i + 1) = glm::vec2(baseTexXCoordinate + texCharacterWidth, 0.0f);
        mesh.textureCoordinates.at(4 * i + 2) = glm::vec2(baseTexXCoordinate + texCharacterWidth, texCharacterHeight);
        mesh.textureCoordinates.at(4 * i + 3) = glm::vec2(baseTexXCoordinate, texCharacterHeight);
    }

    return mesh;
}