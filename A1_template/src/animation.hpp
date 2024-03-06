#pragma once

#include <vector>
#include <random>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_system.hpp"
#include <glm/trigonometric.hpp> // for glm::radians

class Animation
{

	GLFWwindow* window;
	RenderSystem* renderer;
public:
	void init(RenderSystem* renderer, GLFWwindow* window);

	void step(float elapsed_ms);
};