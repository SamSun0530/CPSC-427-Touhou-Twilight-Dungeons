// internal
#include "bullet_system.hpp"

void BulletSystem::init(RenderSystem* renderer_arg, GLFWwindow* window) {
	this->renderer = renderer_arg;
	this->window = window;
}

void BulletSystem::step(float elapsed_ms)
{
	// Manages fire rate for spawning bullet/enemy-bullets
	if (registry.bullets.components.size() + registry.enemyBullets.components.size() <= MAX_BULLETS) {
		for (Entity entity : registry.bulletFireRates.entities) {
			BulletFireRate& fireRate = registry.bulletFireRates.get(entity);

			// Fire rate will use time to become independent of FPS
			// Adapted from: https://forum.unity.com/threads/gun-fire-rate-is-frame-rate-dependent.661588/
			float current_time = glfwGetTime();
			if (fireRate.is_firing && current_time - fireRate.last_time >= fireRate.fire_rate) {
				Motion& motion = registry.motions.get(entity);
				assert(registry.players.size() == 1 && "Only one player should exist");
				for (Entity& player : registry.players.entities) {
					if (entity == player) {
						// player fires bullet towards mouse position
						double mouse_pos_x;
						double mouse_pos_y;
						glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
						last_mouse_position = vec2(mouse_pos_x, mouse_pos_y) - window_px_half + motion.position;
						float x = last_mouse_position.x - motion.position.x;
						float y = last_mouse_position.y - motion.position.y;
						mouse_rotation_angle = -atan2(x, y) - glm::radians(90.0f);

						//Mix_PlayChannel(-1, firing_sound, 0); // TODO: Refactor into sound/audio system
						createBullet(renderer, motion.speed_modified, motion.position, mouse_rotation_angle, last_mouse_position - motion.position, true);
					}
					else {
						// Spawn enemy bullets here
						Motion& player_motion = registry.motions.get(player);
						float x = player_motion.position.x - motion.position.x;
						float y = player_motion.position.y - motion.position.y;
						float enemy_fire_angle = -atan2(x, y) - glm::radians(90.0f);
						createBullet(renderer, motion.speed_modified, motion.position, enemy_fire_angle, { x, y });
					}
					fireRate.last_time = current_time;
				}
			}
		}
	}
}

