// internal
#include "animation.hpp"
#include <iostream>

void Animation::step(float elapsed_ms)
{
	float animation_frame_rate = 200.f;
	float offset = 0;
	for (Entity& animation_entity : registry.animation.entities) {
		EntityAnimation& animation = registry.animation.get(animation_entity);
		Kinematic& kin = registry.kinematics.get(animation_entity);
		if (kin.direction != vec2(0)) {
			animation.state = State::MOVE;
		}
		else {
			animation.state = State::IDLE;
			offset = 4;
		}
		animation.frame_rate_ms -= elapsed_ms;

		if (animation.frame_rate_ms < animation_frame_rate) {
			animation_frame_rate = animation.frame_rate_ms;
		}

		if (animation.frame_rate_ms < 0) {
			animation.render_pos.x += animation.spritesheet_scale.x;
			if (animation.render_pos.x > 1.0) {
				animation.render_pos.x = animation.spritesheet_scale.x;
			}
			animation.frame_rate_ms = 200.f;
		}
	}
	double mouse_pos_x;
	double mouse_pos_y;
	glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
	Motion player_motion;
	for (Entity& player : registry.players.entities) {
		Motion& motion = registry.motions.get(player); 
		vec2 last_mouse_position = vec2(mouse_pos_x, mouse_pos_y) - window_px_half + motion.position;
		float x = last_mouse_position.x - motion.position.x;
		float y = last_mouse_position.y - motion.position.y;
		float mouse_angle_degree = (-atan2(x, y) + M_PI) * (180.0 / M_PI);
		EntityAnimation& player_ani = registry.animation.get(player);
		if (mouse_angle_degree <= 45 || mouse_angle_degree >= 325) {
			player_ani.render_pos.y = (4 + offset) * player_ani.spritesheet_scale.y;
		}
		else if (mouse_angle_degree <= 135) {
			player_ani.render_pos.y = (3 + offset) * player_ani.spritesheet_scale.y;
		}
		else if (mouse_angle_degree <= 225) {
			player_ani.render_pos.y = (1 + offset) * player_ani.spritesheet_scale.y;
		}
		else {
			player_ani.render_pos.y = (2 + offset) * player_ani.spritesheet_scale.y;
		}
		player_motion = motion;
	}
	for (Entity& enemy : registry.deadlys.entities) {
		EntityAnimation& enemy_ani = registry.animation.get(enemy);
		Motion& enemy_motion = registry.motions.get(enemy);
		float x = player_motion.position.x - enemy_motion.position.x;
		float y = player_motion.position.y - enemy_motion.position.y;
		float facing_degree = (-atan2(x, y) + M_PI) * (180.0 / M_PI);
		if (facing_degree <= 45 || facing_degree >= 325) {
			enemy_ani.render_pos.y = 4 * enemy_ani.spritesheet_scale.y;
		}
		else if (facing_degree <= 135) {
			enemy_ani.render_pos.y = 2 * enemy_ani.spritesheet_scale.y;
		}
		else if (facing_degree <= 225) {
			enemy_ani.render_pos.y = 1 * enemy_ani.spritesheet_scale.y;
		}
		else {
			enemy_ani.render_pos.y = 3 * enemy_ani.spritesheet_scale.y;
		}
	}
	
}

void Animation::init(GLFWwindow* window) {
	this->window = window;
}