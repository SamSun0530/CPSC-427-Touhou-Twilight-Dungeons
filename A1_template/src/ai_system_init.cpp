#include "ai_system.hpp"

bool canSeePlayer(Entity& entity) {
	int center_x = world_width >> 1;
	int center_y = world_height >> 1;
	// assume we have a player
	Entity& player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);
	Motion& entity_motion = registry.motions.get(entity);
	int e_y = round((entity_motion.position.y / world_tile_size) + center_y);
	int e_x = round((entity_motion.position.x / world_tile_size) + center_x);
	int p_y = round((player_motion.position.y / world_tile_size) + center_y);
	int p_x = round((player_motion.position.x / world_tile_size) + center_x);
	// Bresenham's line Credit: https://www.codeproject.com/Articles/15604/Ray-casting-in-a-2D-tile-based-environment
	// TODO: Optimize with better algorithm: raycast, others (?)
	// Bug: Enemy sometimes shoot even if player still behind a wall
	void (*swap)(int&, int&) = [](int& a, int& b) {
		int temp = a;
		a = b;
		b = temp;
		};
	bool steep = abs(p_y - e_y) > abs(p_x - e_x);
	if (steep) {
		swap(p_y, p_x);
		swap(e_y, e_x);
	}
	if (e_x > p_x) {
		swap(e_x, p_x);
		swap(e_y, p_y);
	}
	int deltax = p_x - e_x;
	int deltay = abs(p_y - e_y);
	int error = 0;
	int ystep;
	int y = e_y;
	if (e_y < p_y) ystep = 1; else ystep = -1;
	for (int x = e_x; x <= p_x; x++) {
		int x_pos;
		int y_pos;
		int x_grid;
		int y_grid;
		if (steep) {
			x_pos = (y - center_x) * world_tile_size;
			y_pos = (x - center_y) * world_tile_size;
			x_grid = y;
			y_grid = x;
		}
		else {
			x_pos = (x - center_x) * world_tile_size;
			y_pos = (y - center_y) * world_tile_size;
			x_grid = x;
			y_grid = y;
		}
		if (y_grid < 0 || x_grid < 0 || y_grid >= world_height || x_grid >= world_width
			|| WorldSystem::world_map[y_grid][x_grid] == (int)TILE_TYPE::WALL) {
			// line of sight blocked by wall
			return false;
		}
		// debugging
		//createLine({ x_pos, y_pos }, vec2(world_tile_size / 2, world_tile_size / 2));
		error += deltay;
		if (2 * error >= deltax) {
			y += ystep;
			error -= deltax;
		}
	}
	return true;
}

void AISystem::init() {
	ConditionalNode* can_see_player = new ConditionalNode(canSeePlayer);
	ConditionalNode* is_in_range = new ConditionalNode([](Entity& entity) {
		float minimum_range_to_check = 1000000; // sqrt(1000000)=1000 pixels
		Motion& motion = registry.motions.get(entity);
		// asusme there is only one player
		for (Entity& player_entity : registry.players.entities) {
			Motion& player_motion = registry.motions.get(player_entity);
			vec2 dp = player_motion.position - motion.position;
			if (dot(dp, dp) < minimum_range_to_check) return true;
		}
		return false;
		});

	ActionNode* do_nothing = new ActionNode([](Entity& entity) {});
	ActionNode* move_random_direction = new ActionNode([](Entity& entity) {
		if (!registry.idleMoveActions.has(entity)) return;
		std::random_device ran;
		std::mt19937 gen(ran());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		IdleMoveAction& action = registry.idleMoveActions.get(entity);
		if (action.timer_ms <= 0) {
			Kinematic& kinematic = registry.kinematics.get(entity);
			switch (action.state) {
			case State::IDLE:
				action.state = State::MOVE;
				action.timer_ms = action.moving_ms;
				kinematic.direction = { dis(gen), dis(gen) };
				break;
			case State::MOVE:
				action.state = State::IDLE;
				action.timer_ms = action.idle_ms;
				kinematic.direction = { 0, 0 };
				break;
			default:
				break;
			}
		}
		});
	ActionNode* stop_firing = new ActionNode([](Entity& entity) { registry.bulletFireRates.get(entity).is_firing = false; });
	ActionNode* fire_at_player = new ActionNode([](Entity& entity) {
		float current_time = glfwGetTime();
		BulletFireRate& fireRate = registry.bulletFireRates.get(entity);
		if (current_time - fireRate.last_time >= fireRate.fire_rate) {
			fireRate.is_firing = true;
		}
		});
	//is_near_player->setTrue(fire_at_player);
	//is_near_player->setFalse(stop_firing);

	//can_see_player->setFalse(cant_see_player);
	//can_see_player->setTrue(is_near_player);

	can_see_player->setTrue(fire_at_player);
	can_see_player->setFalse(move_random_direction);

	this->ghost.setRoot(can_see_player);
}