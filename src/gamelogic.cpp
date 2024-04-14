#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "utilities/camera.hpp"

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

// 3D geometry nodes
SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* ballNode;
// 2D geometry nodes
SceneNode* textbox0Node;
SceneNode* textbox1Node;
// BH nodes
SceneNode* bhNode;
// Light nodes
SceneNode* light0Node;
SceneNode* light1Node;
SceneNode* light2Node;
// Screen-filling quad node
SceneNode* screenQuadNode;

unsigned int gBuffer;

unsigned int NUM_LIGHTS;

double ballRadius = 3.0f;

// These are heap allocated, because they should not be initialised at the start of the program
Gloom::Shader* gBufferShader;
Gloom::Shader* bhShader;
Gloom::Shader* deferredShader;
Gloom::Camera* camera;

const glm::vec3 boxDimensions(180, 90, 90);

glm::vec3 ballPosition(0.0f, 0.0f, 0.0f);

// Moved to global scope to facilitate use in renderNode()
glm::mat4 perspVP;
glm::mat4 orthoVP;

CommandLineOptions options;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

//// A few lines to help you if you've never used c++ structs
// struct LightSource {
//     bool a_placeholder_value;
// };
// LightSource lightSources[/*Put number of light sources you want here*/];

int setUpTexture(PNGImage image) {
    unsigned int textureID = -1;
    
    // Generate and populate texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.data());

    // Anti-aliasing settings
    glGenerateMipmap(GL_TEXTURE_2D);
    // -- minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    // -- magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

// Static wrapper function to allow passing it as an argument
static void keyboardInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Retrieve the camera instance
    //Gloom::Camera* camera = static_cast<Gloom::Camera*>(glfwGetWindowUserPointer(window));
    if (camera)
    {
        camera->handleKeyboardInputs(key, action);
    }
}

// Static wrapper function to allow passing it as an argument
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // Retrieve the camera instance
    //Gloom::Camera* camera = static_cast<Gloom::Camera*>(glfwGetWindowUserPointer(window));
    if (camera)
    {
        camera->handleMouseButtonInputs(button, action);
    }
}

// Static wrapper function to allow passing it as an argument
static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    // Retrieve the camera instance
    //Gloom::Camera* camera = static_cast<Gloom::Camera*>(glfwGetWindowUserPointer(window));
    if (camera)
    {
        camera->handleCursorPosInput(xpos, ypos);
    }

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    options = gameOptions;

    // Initialise camera object
    camera = new Gloom::Camera(glm::vec3(0, 2, -20), 15.0f, 0.005f);
    glfwSetWindowUserPointer(window, camera);

    // Set up keyboard and mouse callbacks for camera operation
    glfwSetKeyCallback(window, keyboardInputCallback);

    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    gBufferShader = new Gloom::Shader();
    gBufferShader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");

    bhShader = new Gloom::Shader();
    bhShader->makeBasicShader("../res/shaders/bhSimple.vert", "../res/shaders/deferred.frag");

    deferredShader = new Gloom::Shader();
    deferredShader->makeBasicShader("../res/shaders/deferred.vert", "../res/shaders/deferred.frag");

    // Create meshes
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);
    Mesh sphere = generateSphere(1.0, 40, 40, false);

    // Fill buffers
    unsigned int boxVAO  = generateBuffer(box);
    unsigned int ballVAO = generateBuffer(sphere);

    // Construct scene
    rootNode = createSceneNode();
    boxNode  = createSceneNode();
    ballNode = createSceneNode();
    
    rootNode->children.push_back(boxNode);
    rootNode->children.push_back(ballNode);

    boxNode->vertexArrayObjectID     = boxVAO;
    boxNode->VAOIndexCount           = box.indices.size();
    boxNode->nodeType                = NORMAL_MAPPED;

    ballNode->vertexArrayObjectID    = ballVAO;
    ballNode->VAOIndexCount          = sphere.indices.size();


    /* Add textures for walls */
    // Load textures
    PNGImage wallDiffuseImage = loadPNGFile("../res/textures/Brick03_col.png");
    PNGImage wallNormalMapImage = loadPNGFile("../res/textures/Brick03_nrm.png");
    PNGImage roughnessMapImage = loadPNGFile("../res/textures/Brick03_rgh.png");

    // Set up and configure OpenGL textures for the wall's diffuse and normal maps
    boxNode->textureID = setUpTexture(wallDiffuseImage);
    boxNode->normalMapID = setUpTexture(wallNormalMapImage);
    boxNode->roughnessMapID = setUpTexture(roughnessMapImage);
    /* Add textures for walls */

    /* Add BH */
    Mesh bhSphere = generateSphere(2.0f, 40, 40, true);

    unsigned int bhVAO = generateBuffer(bhSphere);

    bhNode = createSceneNode();

    rootNode->children.push_back(bhNode);

    bhNode->vertexArrayObjectID    = bhVAO;
    bhNode->VAOIndexCount          = bhSphere.indices.size();
    bhNode->nodeType               = BLACK_HOLE;
    /* Add BH */

    /* Add screen-filling quad */
    Mesh screenQuad = generateQuad();

    unsigned int quadVAO = generateBuffer(screenQuad);

    screenQuadNode = createSceneNode();

    //rootNode->children.push_back(quadNode);

    screenQuadNode->vertexArrayObjectID = quadVAO;
    screenQuadNode->VAOIndexCount = screenQuad.indices.size();
    screenQuadNode->nodeType = GEOMETRY_2D;
    /* Add screen-filling quad */


    /* Add point lights */
    NUM_LIGHTS = 3;

    light0Node = createSceneNode();
    light1Node = createSceneNode();
    light2Node = createSceneNode();

    rootNode->children.push_back(light0Node);
    rootNode->children.push_back(light1Node);
    rootNode->children.push_back(light2Node);  // TODO: light2Node moves with the camera

    light0Node->position             = glm::vec3(50.0f, 0.0f, -60.0f);
    light0Node->nodeType             = POINT_LIGHT;
    light0Node->lightColor           = glm::vec3(1.0f, 1.0f, 1.0f);
    light0Node->lightID              = 0;

    light1Node->position             = glm::vec3(-50.0f, 0.0f, -60.0f);
    light1Node->nodeType             = POINT_LIGHT;
    light1Node->lightColor           = glm::vec3(1.0f, 1.0f, 1.0f);
    light1Node->lightID              = 1;

    light2Node->position             = glm::vec3(0.0f, 25.0f, 20.0f);
    light2Node->nodeType             = POINT_LIGHT;
    light2Node->lightColor           = glm::vec3(1.0f, 1.0f, 1.0f);
    light2Node->lightID              = 2;

    //light0Node->position = glm::vec3(5.0f, 25.0f, 20.0f);
    //light1Node->position = glm::vec3(-5.0f, 25.0f, 20.0f);

    glUniform1i(6, NUM_LIGHTS);  // Note: doing this here assumes NUM_LIGHTS is constant
    /* Add point lights */


    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    gBuffer = initGBuffer();
}

void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // double timeDelta = getTimeDeltaSeconds();

    const float ballBottomY = boxNode->position.y - (boxDimensions.y/2) + ballRadius;

    const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    const float ballMinX = boxNode->position.x - (boxDimensions.x/2) + ballRadius;
    const float ballMaxX = boxNode->position.x + (boxDimensions.x/2) - ballRadius;
    const float ballMinZ = boxNode->position.z - (boxDimensions.z/2) + ballRadius;
    const float ballMaxZ = boxNode->position.z + (boxDimensions.z/2) - ballRadius - cameraWallOffset;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    ballPosition.x = boxNode->position.x;
    ballPosition.y = boxNode->position.y;
    ballPosition.z = boxNode->position.z;

    glm::mat4 perspProjection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::mat4 orthoProjection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight), 0.1f, 350.f);

    glm::vec3 cameraPosition = glm::vec3(0, 2, -20);

    // Some math to make the camera move in a nice way
    
    camera->updateCamera(getTimeDeltaSeconds());
    glm::mat4 cameraTransform = camera->getViewMatrix();

    // Pass camera position to fragment shader, for specular lighting
    glm::vec3 eyePosition = glm::vec3(cameraTransform * glm::vec4(0, 0, 0, 1));
    glUniform3fv(10, 1, glm::value_ptr(eyePosition));

    perspVP = perspProjection * cameraTransform;
    orthoVP = orthoProjection;

    // Move and rotate various SceneNodes
    boxNode->position = { 0, -10, -80 };

    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = { 0, totalElapsedTime*2, 0 };

    updateNodeTransformations(rootNode, glm::mat4(1.0f));
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    gBufferShader->activate();

    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;

    switch(node->nodeType) {
        case GEOMETRY: break;
        case GEOMETRY_2D: break;
        case NORMAL_MAPPED: break;
        case POINT_LIGHT: {
            glm::vec4 lightCoord = node->currentTransformationMatrix * glm::vec4(0, 0, 0, 1);

            GLint coordLocation = gBufferShader->getUniformFromName(fmt::format("lightSource[{}].coord", node->lightID));
            GLint colorLocation = gBufferShader->getUniformFromName(fmt::format("lightSource[{}].color", node->lightID));

            glUniform3fv(coordLocation, 1, glm::value_ptr(lightCoord));
            glUniform3fv(colorLocation, 1, glm::value_ptr(node->lightColor));

            break;
        }
        case SPOT_LIGHT: break;
    }

    gBufferShader->deactivate();

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(SceneNode* node) {
    // Pass model matrix
    glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    // Calculate and pass normal matrix
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(node->currentTransformationMatrix));
    glUniformMatrix3fv(4, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    
    // For shadow calculation
    glm::vec3 ballPos = glm::vec3(ballNode->currentTransformationMatrix * glm::vec4(0, 0, 0, 1));
    glUniform3fv(11, 1, glm::value_ptr(ballPos));
    glUniform1f(12, float(ballRadius));

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                // Pass renderMode uniform
                glUniform1i(13, GEOMETRY);
                // Calculate MVP matrix (perspective)
                glm::mat4 MVP = perspVP * node->currentTransformationMatrix;
                // Pass MVP matrix
                glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(MVP));

                // Draw the model
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            };
            break;
        case GEOMETRY_2D:
            if(node->vertexArrayObjectID != -1) {
                // Pass renderMode uniform
                glUniform1i(13, GEOMETRY_2D);
                // Bind texture unit
                glBindTextureUnit(0, node->textureID);
                // Calculate MVP matrix (orthogonal)
                glm::mat4 MVP = orthoVP * node->currentTransformationMatrix;
                // Pass MVP matrix
                glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(MVP));

                // Draw the model
                glBindVertexArray(node->vertexArrayObjectID);

                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            };
            break;
        case NORMAL_MAPPED:
            if (node->vertexArrayObjectID != -1) {
                // Pass renderMode uniform
                glUniform1i(13, NORMAL_MAPPED);
                // Bind texture units
                glBindTextureUnit(0, node->textureID);
                glBindTextureUnit(1, node->normalMapID);
                glBindTextureUnit(2, node->roughnessMapID);
                // Calculate MVP matrix (perspective)
                glm::mat4 MVP = perspVP * node->currentTransformationMatrix;
                // Pass MVP matrix
                glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(MVP));

                // Draw the model
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            };
            break;
        case BLACK_HOLE:
            if (node->vertexArrayObjectID != -1) {
                bhShader->activate();  // TODO: Fix all of this

                // Pass renderMode uniform
                glUniform1i(13, BLACK_HOLE);
                // Calculate MVP matrix (perspective)
                glm::mat4 MVP = perspVP * node->currentTransformationMatrix;
                // Pass MVP matrix
                glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(MVP));

                // Draw the model
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set stencil buffer to 1 wherever the black hole is rendered
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glStencilMask(0xFF);
                glClear(GL_STENCIL_BUFFER_BIT);  // Clear previous stencil data

                bhShader->deactivate();
            };
            break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderToGBuffer(GLFWwindow* window) {
    gBufferShader->activate();

    // Bind the framebuffer to gBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // TODO

    renderNode(rootNode);

    gBufferShader->deactivate();
}

void renderToScreen(GLFWwindow* window) {
    deferredShader->activate();

    // Bind the framebuffer to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    deferredShader->deactivate();

    renderNode(screenQuadNode);
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    // First render pass
    renderToGBuffer(window);

    // Deferred render pass
    //renderToScreen(window);
}