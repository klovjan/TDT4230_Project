#pragma once

#include <utilities/window.hpp>
#include "sceneGraph.hpp"

enum ViewMode {
	REGULAR, NORMALS, POSITION, DISTANCE, STENCIL
};

extern ViewMode viewMode;

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar);
void initScene(GLFWwindow* window, CommandLineOptions options);
void updateFrame(GLFWwindow* window);
void renderFrame(GLFWwindow* window);