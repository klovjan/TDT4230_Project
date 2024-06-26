#pragma once

#include <utilities/window.hpp>
#include "sceneGraph.hpp"

enum ViewMode {
	REGULAR, COLOR, POSITION, DISTANCE, NORMALS, STENCIL, BH_NORMALS
};

extern ViewMode viewMode;

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar);
void initScene(GLFWwindow* window, CommandLineOptions options);
void updateFrame(GLFWwindow* window);
void renderFrame(GLFWwindow* window);