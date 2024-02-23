#pragma once

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include <glm/trigonometric.hpp> // for glm::radians

class BulletSystem
{
	const size_t MAX_BULLETS = 999;

	// Keep track of mouse position for firing bullet
	float mouse_rotation_angle = 0.0f;
	vec2 last_mouse_position = { 0, 0 };

	// Misc
	RenderSystem* renderer;
	GLFWwindow* window;
public:
	void BulletSystem::init(RenderSystem* renderer_arg, GLFWwindow* window);

	void step(float elapsed_ms);
};