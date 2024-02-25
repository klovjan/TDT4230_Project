#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

double padPositionX = 0;
double padPositionZ = 0;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

// 3D geometry nodes
SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* ballNode;
SceneNode* padNode;
// 2D geometry nodes
SceneNode* textbox0Node;
SceneNode* textbox1Node;
// Light nodes
SceneNode* light0Node;
SceneNode* light1Node;
SceneNode* light2Node;


unsigned int NUM_LIGHTS;;

double ballRadius = 3.0f;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
sf::Sound* sound;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);

// Moved to global scope to facilitate use in renderNode()
glm::mat4 perspVP;
glm::mat4 orthoVP;

CommandLineOptions options;

bool hasStarted        = false;
bool hasLost           = false;
bool jumpedToNextFrame = false;
bool isPaused          = false;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;
void mouseCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    padPositionX -= mouseSensitivity * deltaX / windowWidth;
    padPositionZ -= mouseSensitivity * deltaY / windowHeight;

    if (padPositionX > 1) padPositionX = 1;
    if (padPositionX < 0) padPositionX = 0;
    if (padPositionZ > 1) padPositionZ = 1;
    if (padPositionZ < 0) padPositionZ = 0;

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}

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

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    shader->activate();

    // Create meshes
    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);
    Mesh sphere = generateSphere(1.0, 40, 40);

    // Fill buffers
    unsigned int ballVAO = generateBuffer(sphere);
    unsigned int boxVAO  = generateBuffer(box);
    unsigned int padVAO  = generateBuffer(pad);

    // Construct scene
    rootNode = createSceneNode();
    boxNode  = createSceneNode();
    padNode  = createSceneNode();
    ballNode = createSceneNode();
    
    rootNode->children.push_back(boxNode);
    rootNode->children.push_back(padNode);
    rootNode->children.push_back(ballNode);

    boxNode->vertexArrayObjectID     = boxVAO;
    boxNode->VAOIndexCount           = box.indices.size();
    boxNode->nodeType                = NORMAL_MAPPED;

    padNode->vertexArrayObjectID     = padVAO;
    padNode->VAOIndexCount           = pad.indices.size();

    ballNode->vertexArrayObjectID    = ballVAO;
    ballNode->VAOIndexCount          = sphere.indices.size();


    /* Add textures for walls */
    // Load textures
    PNGImage wallDiffuseImage = loadPNGFile("../res/textures/Brick03_col.png");
    PNGImage wallNormalMapImage = loadPNGFile("../res/textures/Brick03_nrm.png");

    // Set up and configure OpenGL textures for the wall's diffuse and normal maps
    boxNode->textureID = setUpTexture(wallDiffuseImage);
    boxNode->normalMapID = setUpTexture(wallNormalMapImage);
    /* Add textures for walls */


    /* Add point lights */
    NUM_LIGHTS = 3;

    light0Node = createSceneNode();
    light1Node = createSceneNode();
    light2Node = createSceneNode();

    //rootNode->children.push_back(light0Node);
    //rootNode->children.push_back(light1Node);
    padNode->children.push_back(light2Node);  // light2Node moves with the paddle

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


    /* Add textboxes */
    // Make the textbox mesh
    std::string textbox0String = "Left click to start";
    Mesh textbox0 = generateTextGeometryBuffer(textbox0String, 39.0f/29.0f, textbox0String.length()*29.0f);

    std::string textbox1String = "Right click to pause";
    Mesh textbox1 = generateTextGeometryBuffer(textbox1String, 39.0f / 29.0f, textbox1String.length() * 29.0f);

    // Create VAO for textbox mesh
    unsigned int textbox0VAO = generateBuffer(textbox0);
    unsigned int textbox1VAO = generateBuffer(textbox1);

    // Create SceneNode for textbox
    textbox0Node = createSceneNode();
    textbox1Node = createSceneNode();

    rootNode->children.push_back(textbox0Node);
    textbox0Node->children.push_back(textbox1Node);

    textbox0Node->vertexArrayObjectID    = textbox0VAO;
    textbox0Node->VAOIndexCount          = textbox0.indices.size();

    textbox1Node->vertexArrayObjectID    = textbox1VAO;
    textbox1Node->VAOIndexCount          = textbox1.indices.size();

    textbox0Node->position               = glm::vec3(0.0f,
                                                    windowHeight-39.0f,
                                                    -1.0f);
    textbox1Node->position               = glm::vec3(0.0f,
                                                     -39.0f,
                                                     0.0f);  // Note: textbox1Node is a child of textbox0Node

    textbox0Node->nodeType               = GEOMETRY_2D;
    textbox1Node->nodeType               = GEOMETRY_2D;

    // Load character map texture
    PNGImage charMapImage = loadPNGFile("../res/textures/charmap.png");

    // Set up and configure OpenGL texture for character map
    textbox0Node->textureID = textbox1Node->textureID = setUpTexture(charMapImage);

    // Bind charmap texture to texture unit 3
    glBindTextureUnit(3, textbox0Node->textureID);
    /* Add textboxes */


    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    // Pass number of lights to 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    const float ballBottomY = boxNode->position.y - (boxDimensions.y/2) + ballRadius + padDimensions.y;
    const float ballTopY    = boxNode->position.y + (boxDimensions.y/2) - ballRadius;
    const float BallVerticalTravelDistance = ballTopY - ballBottomY;

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

    if(!hasStarted) {
        if (mouseLeftPressed) {
            if (options.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            gameElapsedTime = debug_startTime;
            hasStarted = true;
        }

        ballPosition.x = ballMinX + (1 - padPositionX) * (ballMaxX - ballMinX);
        ballPosition.y = ballBottomY;
        ballPosition.z = ballMinZ + (1 - padPositionZ) * ((ballMaxZ+cameraWallOffset) - ballMinZ);
    } else {
        totalElapsedTime += timeDelta;
        if(hasLost) {
            if (mouseLeftReleased) {
                hasLost = false;
                hasStarted = false;
                currentKeyFrame = 0;
                previousKeyFrame = 0;
            }
        } else if (isPaused) {
            if (mouseRightReleased) {
                isPaused = false;
                if (options.enableMusic) {
                    sound->play();
                }
            }
        } else {
            gameElapsedTime += timeDelta;
            if (mouseRightReleased) {
                isPaused = true;
                if (options.enableMusic) {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (gameElapsedTime < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = gameElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            // Synchronize ball with music
            if (currentOrigin == BOTTOM && currentDestination == BOTTOM) {
                ballYCoord = ballBottomY;
            } else if (currentOrigin == TOP && currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance;
            } else if (currentDestination == BOTTOM) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * (1 - fractionFrameComplete);
            } else if (currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * fractionFrameComplete;
            }

            // Make ball move
            const float ballSpeed = 60.0f;
            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;

            // Make ball bounce
            if (ballPosition.x < ballMinX) {
                ballPosition.x = ballMinX;
                ballDirection.x *= -1;
            } else if (ballPosition.x > ballMaxX) {
                ballPosition.x = ballMaxX;
                ballDirection.x *= -1;
            }
            if (ballPosition.z < ballMinZ) {
                ballPosition.z = ballMinZ;
                ballDirection.z *= -1;
            } else if (ballPosition.z > ballMaxZ) {
                ballPosition.z = ballMaxZ;
                ballDirection.z *= -1;
            }

            if(options.enableAutoplay) {
                padPositionX = 1-(ballPosition.x - ballMinX) / (ballMaxX - ballMinX);
                padPositionZ = 1-(ballPosition.z - ballMinZ) / ((ballMaxZ+cameraWallOffset) - ballMinZ);
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            if (jumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP) {
                double padLeftX  = boxNode->position.x - (boxDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x);
                double padRightX = padLeftX + padDimensions.x;
                double padFrontZ = boxNode->position.z - (boxDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z);
                double padBackZ  = padFrontZ + padDimensions.z;

                if (   ballPosition.x < padLeftX
                    || ballPosition.x > padRightX
                    || ballPosition.z < padFrontZ
                    || ballPosition.z > padBackZ
                ) {
                    hasLost = true;
                    if (options.enableMusic) {
                        sound->stop();
                        delete sound;
                    }
                }
            }
        }
    }

    glm::mat4 perspProjection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::mat4 orthoProjection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight), 0.1f, 350.f);

    glm::vec3 cameraPosition = glm::vec3(0, 2, -20);

    // Some math to make the camera move in a nice way
    float lookRotation = -0.6 / (1 + exp(-5 * (padPositionX-0.5))) + 0.3;
    glm::mat4 cameraTransform =
                    glm::rotate(0.3f + 0.2f * float(-padPositionZ*padPositionZ), glm::vec3(1, 0, 0)) *
                    glm::rotate(lookRotation, glm::vec3(0, 1, 0)) *
                    glm::translate(-cameraPosition);

    // Pass camera position to fragment shader, for specular lighting
    glm::vec3 eyePosition = glm::vec3(cameraTransform * glm::vec4(0, 0, 0, 1));
    glUniform3fv(10, 1, glm::value_ptr(eyePosition));

    perspVP = perspProjection * cameraTransform;
    //orthoVP = orthoProjection * cameraTransform;
    orthoVP = orthoProjection;

    // Move and rotate various SceneNodes
    boxNode->position = { 0, -10, -80 };

    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = { 0, totalElapsedTime*2, 0 };

    padNode->position  = {
        boxNode->position.x - (boxDimensions.x/2) + (padDimensions.x/2) + (1 - padPositionX) * (boxDimensions.x - padDimensions.x),
        boxNode->position.y - (boxDimensions.y/2) + (padDimensions.y/2),
        boxNode->position.z - (boxDimensions.z/2) + (padDimensions.z/2) + (1 - padPositionZ) * (boxDimensions.z - padDimensions.z)
    };

    updateNodeTransformations(rootNode, glm::mat4(1.0f));
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
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
        case POINT_LIGHT: {
            glm::vec4 lightCoord = node->currentTransformationMatrix * glm::vec4(0, 0, 0, 1);

            GLint coordLocation = shader->getUniformFromName(fmt::format("lightSource[{}].coord", node->lightID));
            GLint colorLocation = shader->getUniformFromName(fmt::format("lightSource[{}].color", node->lightID));

            glUniform3fv(coordLocation, 1, glm::value_ptr(lightCoord));
            glUniform3fv(colorLocation, 1, glm::value_ptr(node->lightColor));

            break;
        }
        case SPOT_LIGHT: break;
    }

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
                // Calculate MVP matrix (perspective)
                glm::mat4 MVP = perspVP * node->currentTransformationMatrix;
                // Pass MVP matrix
                glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(MVP));

                // Draw the model
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            };
            break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    renderNode(rootNode);
}
