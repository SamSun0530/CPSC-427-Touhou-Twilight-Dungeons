// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	// Update flow field
	flow_field.update_timer_ms -= elapsed_ms;
	if (flow_field.update_timer_ms < 0) {
		assert(registry.players.size() == 1 && "One player should exist");
		Entity player = registry.players.entities[0];
		Motion& motion = registry.motions.get(player);
		coord grid_pos = convert_world_to_grid(motion.position);
		if (flow_field.last_position != grid_pos && is_valid_cell(grid_pos.x, grid_pos.y)) {
			update_flow_field_map();
			flow_field.last_position = grid_pos;
		}
		flow_field.update_timer_ms = flow_field.update_base;
	}

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
			else if (registry.bosses.has(entity)) {
				Boss& boss = registry.bosses.get(entity);
				if (boss.boss_id == BOSS_ID::CIRNO) {
					cirno_boss_tree.update(entity);
				}
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

		// Check if entity has reached the last node in the path
		vec2 grid_pos = convert_world_to_grid(motion.position);
		if (fp.next_path_index == fp.path.size() - 1 && grid_pos == fp.path[fp.next_path_index]) {
			// If path is directed towards player, do not stop early
			if (fp.is_player_target) {
				Entity& player = registry.players.entities[0];
				Motion& player_motion = registry.motions.get(player);
				Collidable& entity_collidable = registry.collidables.get(entity);
				Collidable& player_collidable = registry.collidables.get(player);
				kinematic.direction = normalize((player_motion.position + player_collidable.shift) - (motion.position + entity_collidable.shift));
			}
			else {
				kinematic.direction = { 0, 0 };
			}
			registry.followpaths.remove(entity);
			continue;
		}
		vec2 world_next_pos = convert_grid_to_world(fp.path[fp.next_path_index]);
		vec2 dp = world_next_pos - motion.position;
		vec2 direction_normalized = normalize(dp);
		float distance_squared = dot(dp, dp);
		dp = direction_normalized * kinematic.speed_modified;
		float velocity_magnitude = dot(dp, dp);

		// Do not account for lerped velocity as don't want entity to slow down-speed up-slow...
		// Check if entity's velocity is able to reach next position and direct entity to next path
		if (velocity_magnitude > distance_squared && fp.next_path_index + 1 < fp.path.size()) {
			fp.next_path_index++;
			world_next_pos = convert_grid_to_world(fp.path[fp.next_path_index]);
			direction_normalized = normalize(world_next_pos - motion.position);
		}

		kinematic.direction = direction_normalized;
	}

	ComponentContainer<FollowFlowField>& ff_components = registry.followFlowField;
	for (uint i = 0; i < ff_components.components.size(); i++) {
		FollowFlowField& ff = ff_components.components[i];
		Entity& entity = ff_components.entities[i];

		Motion& motion = registry.motions.get(entity);
		Kinematic& kinematic = registry.kinematics.get(entity);

		coord grid_pos = convert_world_to_grid(motion.position);
		if (!is_valid_cell(grid_pos.x, grid_pos.y)) {
			registry.followFlowField.remove(entity);
			continue;
		}

		int current_dist = flow_field_map[grid_pos.y][grid_pos.x];
		if (current_dist == 0) {
			// If path is directed towards player, do not stop early
			if (ff.is_player_target) {
				Entity& player = registry.players.entities[0];
				Motion& player_motion = registry.motions.get(player);
				Collidable& entity_collidable = registry.collidables.get(entity);
				Collidable& player_collidable = registry.collidables.get(player);
				kinematic.direction = normalize((player_motion.position + player_collidable.shift) - (motion.position + entity_collidable.shift));
			}
			else {
				kinematic.direction = { 0, 0 };
			}
			registry.followpaths.remove(entity);
			continue;
		}

		// Initialize flow field for the first time
		if (ff.next_grid_pos.x == -1 && ff.next_grid_pos.y == -1) {
			int retFlag;
			coord best_grid_pos = get_best_grid_pos(grid_pos, current_dist, retFlag);
			if (retFlag == 3) continue;
			ff.next_grid_pos = best_grid_pos;
		}

		vec2 world_next_pos = convert_grid_to_world(ff.next_grid_pos);
		vec2 dp = world_next_pos - motion.position;
		vec2 direction_normalized = normalize(dp);
		float distance_squared = dot(dp, dp);
		dp = direction_normalized * kinematic.speed_modified;
		float velocity_magnitude = dot(dp, dp);

		// Do not account for lerped velocity as don't want entity to slow down-speed up-slow...
		// Check if entity's velocity is able to reach next position and direct entity to next grid cell
		if (velocity_magnitude > distance_squared) {
			int retFlag;
			coord best_grid_pos = get_best_grid_pos(grid_pos, current_dist, retFlag);
			if (retFlag == 3) continue;
			ff.next_grid_pos = best_grid_pos;

			world_next_pos = convert_grid_to_world(best_grid_pos);
			direction_normalized = normalize(world_next_pos - motion.position);
		}

		kinematic.direction = direction_normalized;
	}
}

// helper function to get next best grid position on flow field
// input grid position, current grid dist/value on flow field, and return flag (if 3, then error)
coord AISystem::get_best_grid_pos(coord& grid_pos, int current_dist, int& retFlag)
{
	retFlag = 1;
	coord best_grid_pos = { -1, -1 };
	for (const coord& action : ACTIONS) {
		vec2 temp = grid_pos + action;
		int candidate_dist = flow_field_map[temp.y][temp.x];
		if (candidate_dist != -1 && candidate_dist < current_dist) {
			best_grid_pos = temp;
			break;
		}
	}
	if (best_grid_pos.x == -1 || best_grid_pos.y == -1) {
		retFlag = 3;
	};
	return best_grid_pos;
}

AISystem::AISystem() {
	rng = std::default_random_engine(std::random_device()());
}