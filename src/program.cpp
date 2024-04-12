// Local headers
#include "program.hpp"
#include "utilities/window.hpp"
#include "gamelogic.h"
#include <glm/glm.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <SFML/Audio.hpp>
#include <SFML/System/Time.hpp>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/shader.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utilities/timeutils.h>


void runProgram(GLFWwindow* window, CommandLineOptions options)
{
    // Enable depth (Z) buffer (accept "closest" fragment)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Configure miscellaneous OpenGL settings
    glEnable(GL_CULL_FACE);

    // Disable built-in dithering
    glDisable(GL_DITHER);

    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set default colour after clearing the colour buffer
    glClearColor(0.3f, 0.5f, 0.8f, 1.0f);

	initGame(window, options);

    // Rendering Loop
    while (!glfwWindowShouldClose(window))
    {
	    // Clear colour and depth buffers
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        updateFrame(window);
        renderFrame(window);


        // Handle other events
        glfwPollEvents();
        handleKeyboardInput(window);

        // Flip buffers
        glfwSwapBuffers(window);
    }
}

const float defaultMoveSpeed = 0.1f;
const float fastMoveSpeed = 1.0f;
const float angleSpeed = 0.03f;
void handleKeyboardInput(GLFWwindow* window)
{
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // float cPitchSin = sin(cameraState.pitch);
    // float cPitchCos = cos(cameraState.pitch);
    // float cYawSin = sin(cameraState.yaw);
    // float cYawCos = cos(cameraState.yaw);

    // float moveSpeed = defaultMoveSpeed;  // Is overwritten below if right shift key has been pressed
    // if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
    // {
    //     moveSpeed = fastMoveSpeed;
    // }
    // float forwardIncrementX = -cYawSin * (cPitchCos * moveSpeed);
    // float forwardIncrementY = cPitchSin * moveSpeed;
    // float forwardIncrementZ = -cYawCos * (cPitchCos * moveSpeed);

    // // Translate camera sideways
    // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    // {
    //     cameraState.x += cYawCos * moveSpeed;
    //     cameraState.z -= cYawSin * moveSpeed;
    // }
    // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    // {
    //     cameraState.x -= cYawCos * moveSpeed;
    //     cameraState.z += cYawSin * moveSpeed;
    // }
    // // Translate camera y
    // if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    // {
    //     cameraState.y += moveSpeed;
    // }
    // if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    // {
    //     cameraState.y -= moveSpeed;
    // }
    // // Translate camera forward
    // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    // {
    //     cameraState.x += forwardIncrementX;
    //     cameraState.y += forwardIncrementY;
    //     cameraState.z += forwardIncrementZ;
    // }
    // // Translate camera backward
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    // {
    //     cameraState.x -= forwardIncrementX;
    //     cameraState.y -= forwardIncrementY;
    //     cameraState.z -= forwardIncrementZ;
    // }
    // // Rotate camera around z-axis (roll)
    // if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    // {
    //     cameraState.roll += angleSpeed;
    // }
    // if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    // {
    //     cameraState.roll -= angleSpeed;
    // }
    // // Rotate camera around y-axis (yaw)
    // if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    // {
    //     cameraState.yaw += angleSpeed;
    // }
    // if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    // {
    //     cameraState.yaw -= angleSpeed;
    // }
    // // Rotate camera around x-axis (pitch)
    // if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    // {
    //     if (cameraState.pitch < pi / 2.0f) {
    //         cameraState.pitch += angleSpeed;
    //     }
    // }
    // if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    // {
    //     if (cameraState.pitch > -pi / 2.0f) {
    //         cameraState.pitch -= angleSpeed;
    //     }
    // }
    // // Reset the camera
    // if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    // {
    //     cameraState.x = 0.0f;
    //     cameraState.y = 0.0f;
    //     cameraState.z = 0.0f;
    // }
}
