// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	// Randomized enemy position based on their state
	for (Entity& entity : registry.idleMoveActions.entities) {
		// progress timer
		IdleMoveAction& action = registry.idleMoveActions.get(entity);
		action.timer_ms = action.timer_ms < elapsed_ms ? 0.f : action.timer_ms - elapsed_ms;
		if (action.timer_ms <= 0) {
			// Prevent entity to continue moving
			if (action.state == State::MOVE) {
				Kinematic& kinematic = registry.kinematics.get(entity);
				action.state = State::IDLE;
				action.timer_ms = action.idle_ms;
				kinematic.direction = { 0, 0 };
			}
		}
	}

	// Limit number of checks based on update timer
	ComponentContainer<AiTimer>& aitimer_components = registry.aitimers;
	for (uint i = 0; i < aitimer_components.components.size(); i++) {
		AiTimer& aitimer = aitimer_components.components[i];
		Entity& entity = aitimer_components.entities[i];

		aitimer.update_timer_ms = aitimer.update_timer_ms < elapsed_ms ? 0.f : aitimer.update_timer_ms - elapsed_ms;
		if (aitimer.update_timer_ms <= 0) {
			if (registry.beeEnemies.has(entity)) {
				bee_tree.update(entity);
			}
			else if (registry.bomberEnemies.has(entity)) {
				bomber_tree.update(entity);
			}
			else if (registry.wolfEnemies.has(entity)) {
				wolf_tree.update(entity);
			}
			aitimer.update_timer_ms = aitimer.update_base;
		}
	}

	// Progress entity to follow next path
	ComponentContainer<FollowPath>& fp_components = registry.followpaths;
	for (uint i = 0; i < fp_components.components.size(); i++) {
		FollowPath& fp = fp_components.components[i];
		Entity& entity = fp_components.entities[i];
		if (fp.path.size() == 0) {
			registry.followpaths.remove(entity);
			continue;
		}

		Motion& motion = registry.motions.get(entity);
		Kinematic& kinematic = registry.kinematics.get(entity);

		vec2 grid_pos = { round((motion.position.x / world_tile_size) + world_center.x),
						round((motion.position.y / world_tile_size) + world_center.y) };
		if (fp.next_path_index == fp.path.size() - 1 && grid_pos == fp.path[fp.next_path_index]) {
			if (fp.is_player_target) {
				Entity& player = registry.players.entities[0];
				Motion& player_motion = registry.motions.get(player);
				Collidable& entity_collidable = registry.collidables.get(entity);
				Collidable& player_collidable = registry.collidables.get(player);
				kinematic.direction = normalize((player_motion.position + player_collidable.shift) - (motion.position + entity_collidable.shift));
			}
			registry.followpaths.remove(entity);
			continue;
		}
		vec2 screen_next_pos = { (fp.path[fp.next_path_index].x - world_center.x) * world_tile_size,
								(fp.path[fp.next_path_index].y - world_center.y) * world_tile_size };
		vec2 dp = screen_next_pos - motion.position;
		vec2 direction_normalized = normalize(dp);
		float distance_squared = dot(dp, dp);
		dp = direction_normalized * kinematic.speed_modified;
		float velocity_magnitude = dot(dp, dp);

		// Do not account for lerped velocity as don't want entity to slow down-speed up-slow...
		if (velocity_magnitude > distance_squared && fp.next_path_index + 1 < fp.path.size()) {
			fp.next_path_index++;
			screen_next_pos = { (fp.path[fp.next_path_index].x - world_center.x) * world_tile_size,
								(fp.path[fp.next_path_index].y - world_center.y) * world_tile_size };
			direction_normalized = normalize(screen_next_pos - motion.position);
		}

		kinematic.direction = direction_normalized;
	}

}

AISystem::AISystem() {
	rng = std::default_random_engine(std::random_device()());
}