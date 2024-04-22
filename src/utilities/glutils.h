#pragma once

#include "mesh.h"
#include "imageLoader.hpp"

typedef struct Framebuffer {
    unsigned int fboID;           // Framebuffer object ID
    unsigned int colorTexture;    // Color attachment texture ID
    unsigned int posTexture;      // Position attachment texture ID
    unsigned int normalTexture;   // Normal attachment texture ID
    unsigned int stencilTexture;  // Stencil attachment texture ID
    unsigned int bhNormalTexture; // BH normal attachment texture ID
} Framebuffer;

unsigned int generateBuffer(Mesh &mesh);
int setUpTexture(PNGImage image);
Framebuffer initGBuffer();