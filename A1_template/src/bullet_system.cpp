// internal
#include "bullet_system.hpp"

void BulletSystem::init(RenderSystem* renderer_arg, GLFWwindow* window, Audio* audio) {
	this->renderer = renderer_arg;
	this->window = window;
	this->audio = audio;
}

void BulletSystem::step(float elapsed_ms)
{
	for (Entity entity : registry.bulletSpawners.entities) {
		BulletSpawner& bullet_spawner = registry.bulletSpawners.get(entity);
		bullet_spawner.last_fire_time -= elapsed_ms;
		bullet_spawner.last_update -= elapsed_ms;
		bullet_spawner.last_cooldown -= bullet_spawner.is_cooldown ? elapsed_ms : 0;

		if (bullet_spawner.number_to_fire != -1 && bullet_spawner.last_cooldown < 0) {
			bullet_spawner.last_cooldown = bullet_spawner.cooldown_rate * 100.f;
			bullet_spawner.number_current_fired = 0;
			bullet_spawner.is_cooldown = false;
		}

		if (!bullet_spawner.is_firing || bullet_spawner.is_cooldown) continue;

		if (bullet_spawner.last_update < 0) {
			if (abs(bullet_spawner.spin_rate) > abs(bullet_spawner.max_spin_rate)) {
				float rate_sign = sign(bullet_spawner.spin_rate);
				bullet_spawner.spin_rate = rate_sign * bullet_spawner.max_spin_rate;
				if (bullet_spawner.invert) {
					bullet_spawner.spin_delta = -bullet_spawner.spin_delta;
				}
			}
			else {
				bullet_spawner.spin_rate += bullet_spawner.spin_delta;
			}

			bullet_spawner.start_angle += bullet_spawner.spin_rate;
			bullet_spawner.last_update = bullet_spawner.update_rate * 50.f;
		}

		if (bullet_spawner.last_fire_time < 0) {
			// Set intial bullet directions
			assert(registry.players.size() == 1 && "Only one player should exist");
			Entity& player = registry.players.entities[0];
			Motion& motion = registry.motions.get(entity);
			Kinematic& kinematic = registry.kinematics.get(entity);

			Transform transform;
			transform.rotate(radians(bullet_spawner.start_angle));
			vec2 initial_dir = transform.mat * vec3(1, 0, 1);

			if (entity == player) {
				// player fires bullet towards mouse position
				double mouse_pos_x;
				double mouse_pos_y;
				glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
				last_mouse_position = vec2(mouse_pos_x, mouse_pos_y) - window_px_half + motion.position;
				float x = last_mouse_position.x - motion.position.x;
				float y = last_mouse_position.y - motion.position.y;
				mouse_rotation_angle = -atan2(x, y) - glm::radians(90.0f);

				// shift origin of bullet in front and down a bit so player can shoot vertically and horizontally when against a wall
				vec2 shift_down = vec2(0, 15);
				vec2 shift_forward = 30.f * normalize(last_mouse_position - motion.position);

				Mix_PlayChannel(-1, audio->firing_sound, 0);
				player_bullet_spawn_pos = motion.position + shift_forward + shift_down;
				initial_dir = transform.mat * vec3(last_mouse_position - motion.position - shift_down, 1.0f);
			}
			// need to check boss first, since boss also have deadly component
			else if (registry.bosses.has(entity)) {
				initial_dir = transform.mat * vec3(1, 0, 1);
			}
			else if (registry.deadlys.has(entity)) {
				Motion& player_motion = registry.motions.get(player);
				vec2 player_dir = player_motion.position - motion.position;
				//float enemy_fire_angle = -atan2(x, y) - glm::radians(90.0f);
				initial_dir = transform.mat * vec3(normalize(player_dir), 1);
			}

			std::vector<vec2> initial_bullet_directions = { normalize(initial_dir) };
			set_bullet_directions(bullet_spawner.total_bullet_array, bullet_spawner.spread_between_array, transform, initial_dir, initial_bullet_directions);
			// Set relative bullet directions
			std::vector<vec2> bullet_direcions;
			for (int i = 0; i < initial_bullet_directions.size(); ++i) {
				set_bullet_directions(bullet_spawner.bullets_per_array, bullet_spawner.spread_within_array, transform, initial_bullet_directions[i], bullet_direcions);
			}
			// Spawn bullets
			if (entity == player) {
				spawn_bullets(renderer, initial_bullet_directions, bullet_spawner, player_bullet_spawn_pos, kinematic, true);
				spawn_bullets(renderer, bullet_direcions, bullet_spawner, player_bullet_spawn_pos, kinematic, true);
			}
			else if (registry.bosses.has(entity)) {
				Boss& boss = registry.bosses.get(entity);
				bool has_patterns = boss.bullet_patterns.size() != 0;
				BulletPattern* bullet_pattern = nullptr;
				if (has_patterns) bullet_pattern = &boss.bullet_patterns[boss.bp_index];
				spawn_bullets(renderer, initial_bullet_directions, bullet_spawner, motion.position, kinematic, false, bullet_pattern);
				spawn_bullets(renderer, bullet_direcions, bullet_spawner, motion.position, kinematic, false, bullet_pattern);
			}
			else {
				spawn_bullets(renderer, initial_bullet_directions, bullet_spawner, motion.position, kinematic, false);
				spawn_bullets(renderer, bullet_direcions, bullet_spawner, motion.position, kinematic, false);
			}

			if (bullet_spawner.number_to_fire != -1) {
				bullet_spawner.number_current_fired++;
				bullet_spawner.is_cooldown = bullet_spawner.number_current_fired == bullet_spawner.number_to_fire;
			}
			bullet_spawner.last_fire_time = bullet_spawner.fire_rate * 50.f;
		}
	}

	// Progress bullet delay timers
	ComponentContainer<BulletDelayTimer>& delay_container = registry.bulletDelayTimers;
	int delay_container_size = delay_container.components.size();
	for (int i = delay_container_size - 1; i >= 0; --i) {
		BulletDelayTimer& bullet_delay_timer = delay_container.components[i];
		bullet_delay_timer.delay_counter_ms -= elapsed_ms;
		if (bullet_delay_timer.delay_counter_ms < 0) {
			Entity entity = delay_container.entities[i];
			delay_container.remove(entity);
		}
	}

	// Process bullet pattern
	// Iterate backwards to remove without interfering with next entity
	ComponentContainer<BulletPattern>& pattern_container = registry.bulletPatterns;
	int pattern_container_size = pattern_container.components.size();
	for (int i = pattern_container_size - 1; i >= 0; --i) {
		Entity& entity = pattern_container.entities[i];
		if (registry.bulletDelayTimers.has(entity)) continue;
		BulletPattern& bullet_pattern = pattern_container.components[i];
		std::vector<BulletCommand>& commands = bullet_pattern.commands;
		int commands_size = commands.size();
		// Execute all commands until delay command or reached end of list
		while (bullet_pattern.bc_index < commands_size) {
			switch (commands[bullet_pattern.bc_index].action) {
			case BULLET_ACTION::SPEED: {
				Kinematic& kinematic = registry.kinematics.get(entity);
				kinematic.speed_modified = commands[bullet_pattern.bc_index].value;
				break;
			}
			case BULLET_ACTION::DELAY: {
				BulletDelayTimer& bdt = registry.bulletDelayTimers.emplace(entity);
				bdt.delay_counter_ms = commands[bullet_pattern.bc_index].value;
				break;
			}
			case BULLET_ACTION::DEL: {
				BulletDeathTimer& bdt = registry.bulletDeathTimers.emplace(entity);
				bdt.death_counter_ms = commands[bullet_pattern.bc_index].value;
			}
			default:
				break;
			}

			if (commands[bullet_pattern.bc_index++].action == BULLET_ACTION::DELAY) {
				break;
			}
		}

		if (bullet_pattern.bc_index >= commands_size) {
			pattern_container.remove(entity);
		}
	}

	// Remove bullet death timers
	ComponentContainer<BulletDeathTimer>& bullet_death_container = registry.bulletDeathTimers;
	int bullet_delay_container_size = bullet_death_container.components.size();
	for (int i = bullet_delay_container_size - 1; i >= 0; --i) {
		BulletDeathTimer& bullet_death_timer = bullet_death_container.components[i];
		bullet_death_timer.death_counter_ms -= elapsed_ms;
		if (bullet_death_timer.death_counter_ms < 0) {
			registry.remove_all_components_of(bullet_death_container.entities[i]);
		}
	}
}

void spawn_bullets(RenderSystem* renderer, std::vector<vec2>& initial_bullet_directions, BulletSpawner& bullet_spawner, vec2 spawn_position, Kinematic& kinematic, bool is_player_bullet, BulletPattern* bullet_pattern)
{
	for (int i = 0; i < initial_bullet_directions.size(); ++i) {
		createBullet(renderer, kinematic.speed_modified, spawn_position, 0, initial_bullet_directions[i], bullet_spawner.bullet_initial_speed, is_player_bullet, bullet_pattern);
	}
}

void set_bullet_directions(int number_bullets, float spread_angle, Transform& transform, glm::vec2& initial_dir, path& bullet_directions)
{
	for (int i = 1; i < number_bullets; ++i) {
		// equally distribute above/below initial direction
		int angle_factor;
		if (i % 2 == 0) {
			angle_factor = -i / 2;
		}
		else {
			angle_factor = (i + 1) / 2;
		}
		float angle = spread_angle * angle_factor;
		transform.mat = mat3(1.0f);
		transform.rotate(radians(angle));
		vec2 ith_direction = transform.mat * vec3(initial_dir, 1.0);
		bullet_directions.push_back(normalize(ith_direction));
	}
}

