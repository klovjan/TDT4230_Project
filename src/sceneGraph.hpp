#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stack>
#include <vector>
#include <cstdio>
#include <stdbool.h>
#include <cstdlib> 
#include <ctime> 
#include <chrono>
#include <fstream>

enum SceneNodeType {
	GEOMETRY, GEOMETRY_2D, NORMAL_MAPPED, BLACK_HOLE, POINT_LIGHT, SPOT_LIGHT
};

struct SceneNode {
	SceneNode() {
		position = glm::vec3(0, 0, 0);
		rotation = glm::vec3(0, 0, 0);
		scale = glm::vec3(1, 1, 1);

		color = glm::vec3(1, 1, 1);

        referencePoint = glm::vec3(0, 0, 0);
        vertexArrayObjectID = -1;
        VAOIndexCount = 0;

        nodeType = GEOMETRY;
	}

	// A list of all children that belong to this node.
	// For instance, in case of the scene graph of a human body shown in the assignment text, the "Upper Torso" node would contain the "Left Arm", "Right Arm", "Head" and "Lower Torso" nodes in its list of children.
	std::vector<SceneNode*> children;
	
	// The node's position and rotation relative to its parent
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	// The node's surface color (if not textured)
	glm::vec3 color;

	// A transformation matrix representing the transformation of the node's location relative to its parent. This matrix is updated every frame.
	glm::mat4 currentTransformationMatrix;

	// The location of the node's reference point
	glm::vec3 referencePoint;

	// The ID of the VAO containing the "appearance" of this SceneNode.
	int vertexArrayObjectID;
	unsigned int VAOIndexCount;

	// Node type is used to determine how to handle the contents of a node
	SceneNodeType nodeType;

	// If the SceneNode has an associated texture
	int textureID = -1;

	// If the SceneNode has an associated normal map
	int normalMapID = -1;

	// If the SceneNode has an associated roughness map
	int roughnessMapID = -1;

	// If the SceneNode contains a black hole
	float innerRadius = 0.3f;  // The percentage of the radius of the sphere that is completely black

	// If the SceneNode represents a light source
	int lightID = -1;
	glm::vec3 lightColor;
};

SceneNode* createSceneNode();
void addChild(SceneNode* parent, SceneNode* child);
void printNode(SceneNode* node);
int totalChildren(SceneNode* parent);

// For more details, see SceneGraph.cpp.