#pragma once

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include "audio.hpp"
#include "global.hpp"

#include <glm/trigonometric.hpp> // for glm::radians
#include <glm/glm.hpp>

class BulletSystem
{
	const size_t MAX_BULLETS = 999;

	// Keep track of mouse position for firing bullet
	float mouse_rotation_angle = 0.0f;
	vec2 last_mouse_position = { 0, 0 };
	vec2 player_bullet_spawn_pos;

	// Misc
	RenderSystem* renderer;
	GLFWwindow* window;
	Audio* audio;
public:
	void init(RenderSystem* renderer_arg, GLFWwindow* window, Audio* audio);

	void step(float elapsed_ms);
};

void set_bullet_directions(int number_bullets, float spread_angle, Transform& transform, glm::vec2& initial_dir, path& bullet_directions);

void spawn_bullets(RenderSystem* renderer, std::vector<vec2>& initial_bullet_directions, float bullet_initial_speed, vec2 spawn_position, Kinematic& kinematic, bool is_player_bullet = false, BulletPattern* bullet_pattern = nullptr);
