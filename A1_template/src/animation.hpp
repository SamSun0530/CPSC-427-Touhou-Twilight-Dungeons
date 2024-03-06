#pragma once

#include <vector>
#include <random>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include <glm/trigonometric.hpp> // for glm::radians

class Animation
{

	GLFWwindow* window;
public:
	void init(GLFWwindow* window);

	void step(float elapsed_ms);
};