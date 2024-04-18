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
#include "bhSimulation.h"
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

// Screen-filling quad for deferred rendering
unsigned int screenQuadVAO;
Mesh screenQuad;

Framebuffer gBuffer;

unsigned int NUM_LIGHTS;

float ballRadius = 3.0f;
float bhRadius = 40.0f;

// These are heap allocated, because they should not be initialised at the start of the program
Gloom::Shader* gBufferShader;
Gloom::Shader* bhShader;
Gloom::Shader* deferredShader;
Gloom::Camera* camera;

const glm::vec3 boxDimensions(360, 360, 360);

glm::vec3 ballPosition(0.0f, 0.0f, 0.0f);
glm::vec3 bhPosition(0.0f, 0.0f, 0.0f);

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

// Static wrapper function to allow passing it as an argument
static void keyboardInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (camera)
    {
        camera->handleKeyboardInputs(key, action);
    }
}

// Static wrapper function to allow passing it as an argument
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (camera)
    {
        camera->handleMouseButtonInputs(button, action);
    }
}


// Static wrapper function to allow passing it as an argument
static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (camera)
    {
        camera->handleCursorPosInput(xpos, ypos);
    }

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}

void initScene(GLFWwindow* window, CommandLineOptions clOptions) {
    options = clOptions;

    // Initialise camera object
    camera = new Gloom::Camera(glm::vec3(0, 2, 100), 15.0f, 0.005f);
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

    boxNode->position = { 0, -10, -80 };


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
    Mesh bhSphere = generateSphere(bhRadius, 40, 40, true);

    unsigned int bhVAO = generateBuffer(bhSphere);

    bhNode = createSceneNode();

    rootNode->children.push_back(bhNode);

    bhNode->vertexArrayObjectID    = bhVAO;
    bhNode->VAOIndexCount          = bhSphere.indices.size();
    bhNode->nodeType               = BLACK_HOLE;
    /* Add BH */

    /* Add screen-filling quad */
    screenQuad = generateQuad();

    screenQuadVAO = generateBuffer(screenQuad);
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

    gBufferShader->activate();
    glUniform1i(6, NUM_LIGHTS);  // Note: doing this here assumes NUM_LIGHTS is constant
    gBufferShader->deactivate();
    /* Add point lights */

    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    gBuffer = initGBuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.fboID);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void updateFrame(GLFWwindow* window) {
    // double timeDelta = getTimeDeltaSeconds();

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

    glm::mat4 perspProjection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 500.f);
    glm::mat4 orthoProjection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight), 0.1f, 350.f);
    
    camera->updateCamera(getTimeDeltaSeconds());
    glm::mat4 cameraTransform = camera->getViewMatrix();

    // Pass camera position to fragment shader, for specular lighting
    glm::vec3 eyePosition = glm::vec3(cameraTransform * glm::vec4(0, 0, 0, 1));
    gBufferShader->activate();
    glUniform3fv(10, 1, glm::value_ptr(eyePosition));
    gBufferShader->deactivate();

    deferredShader->activate();
    glUniform3fv(10, 1, glm::value_ptr(eyePosition));
    deferredShader->deactivate();

    perspVP = perspProjection * cameraTransform;
    orthoVP = orthoProjection;

    // Move and rotate various SceneNodes
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
        case BLACK_HOLE: break;
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
    gBufferShader->activate();

    // Pass model matrix
    glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    // Calculate and pass normal matrix
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(node->currentTransformationMatrix));
    glUniformMatrix3fv(4, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    
    // For shadow calculation
    glm::vec3 ballPos = glm::vec3(ballNode->currentTransformationMatrix * glm::vec4(0, 0, 0, 1));
    glUniform3fv(11, 1, glm::value_ptr(ballPos));
    glUniform1f(12, float(ballRadius));

    gBufferShader->deactivate();

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                gBufferShader->activate();

                // Pass renderMode uniform
                glUniform1i(13, GEOMETRY);
                // Calculate MVP matrix (perspective)
                glm::mat4 MVP = perspVP * node->currentTransformationMatrix;
                // Pass MVP matrix
                glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(MVP));

                // Draw the model
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

                gBufferShader->deactivate();
            };
            break;
        case GEOMETRY_2D:
            if(node->vertexArrayObjectID != -1) {
                gBufferShader->activate();

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

                gBufferShader->deactivate();
            };
            break;
        case NORMAL_MAPPED:
            if (node->vertexArrayObjectID != -1) {
                gBufferShader->activate();

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

                gBufferShader->deactivate();
            };
            break;
        case BLACK_HOLE:
            if (node->vertexArrayObjectID != -1) {
                gBufferShader->activate();
                // Disable all textures except the stencil
                glColorMaski(0, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Color (disable)
                glColorMaski(1, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Position (disable)
                //glColorMaski(2, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Normal (disable)

                // Pass renderMode uniform
                glUniform1i(13, BLACK_HOLE);
                // Update the "stencil" buffer with the black hole
                glm::mat4 MVP = perspVP * bhNode->currentTransformationMatrix;
                glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(MVP));

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, bhNode->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

                // Re-enable all textures
                glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glColorMaski(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                //glColorMaski(2, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                gBufferShader->deactivate();
            }
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

    // Ensure background color doesn't leak into gBuffer
    // glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    renderNode(rootNode);

    gBufferShader->deactivate();
}

void renderToScreen(GLFWwindow* window) {
    deferredShader->activate();
    
    // Clear the screen's color and depth buffers
    glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Pass some uniforms
    glm::vec3 bhPos = glm::vec3(bhNode->currentTransformationMatrix * glm::vec4(0, 0, 0, 1));
    glUniform3fv(14, 1, glm::value_ptr(bhPos));
    
    glm::vec4 bhWorldPos = bhNode->currentTransformationMatrix * glm::vec4(0, 0, 0, 1);
    glm::vec4 bhClipPos = perspVP * bhWorldPos;
    glm::vec3 bhNdcPos = glm::vec3(bhClipPos) / bhClipPos.w;
    glUniform2fv(15, 1, glm::value_ptr(bhNdcPos));

    printf("x: %f, y: %f\n", bhNdcPos.x, bhNdcPos.y);

    glUniform1f(16, bhRadius);

    glBindTextureUnit(0, gBuffer.colorTexture);
    glBindTextureUnit(1, gBuffer.posTexture);
    glBindTextureUnit(2, gBuffer.normalTexture);
    glBindTextureUnit(3, gBuffer.stencilTexture);

    glBindVertexArray(screenQuadVAO);
    glDrawElements(GL_TRIANGLES, screenQuad.indices.size(), GL_UNSIGNED_INT, nullptr);

    deferredShader->deactivate();
}

void renderFrame(GLFWwindow* window) {
    // Bind the gBuffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.fboID);

    // First render pass
    renderToGBuffer(window);

    // Bind the default framebuffer (screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Deferred render pass
    renderToScreen(window);
}