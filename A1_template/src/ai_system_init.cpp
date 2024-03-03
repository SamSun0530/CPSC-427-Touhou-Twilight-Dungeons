#include "ai_system.hpp"

bool isValidCell(int row, int col) {
	return (row >= 0 && col >= 0 && row < world_height && col < world_width);
}

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
	ActionNode* cant_see_player = new ActionNode([](Entity& entity) {
		printf("can't see player! \n");
		});

	ConditionalNode* is_near_player = new ConditionalNode([](Entity& entity) {
		float distance_to_attack = 90000.f; // sqrt(90000)=300 pixels
		Motion& motion = registry.motions.get(entity);
		// asusme there is only one player
		for (Entity& player_entity : registry.players.entities) {
			Motion& player_motion = registry.motions.get(player_entity);
			vec2 dp = player_motion.position - motion.position;
			if (dot(dp, dp) < distance_to_attack) return true;
		}
		return false;
		});
	ActionNode* stop_firing = new ActionNode([](Entity& entity) { registry.bulletFireRates.remove(entity); });
	ActionNode* fire_at_player = new ActionNode([](Entity& entity) {
		if (!registry.bulletFireRates.has(entity)) {
			auto& fire_rate = registry.bulletFireRates.emplace(entity);
			fire_rate.is_firing = true;
			fire_rate.fire_rate = 3;
		}
		});
	//is_near_player->setTrue(fire_at_player);
	//is_near_player->setFalse(stop_firing);

	//can_see_player->setFalse(cant_see_player);
	//can_see_player->setTrue(is_near_player);

	can_see_player->setTrue(stop_firing);
	can_see_player->setFalse(stop_firing);

	this->ghost.setRoot(can_see_player);
}