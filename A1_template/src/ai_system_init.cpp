#include "ai_system.hpp"

bool isValidCell(int row, int col) {
	return (row >= 0 && col >= 0 && row < world_height && col < world_width);
}

bool canSeePlayer(Entity& entity) {
	int centerX = world_width >> 1;
	int centerY = world_height >> 1;
	printf("=============\n");
	// assume we have a player
	Entity& player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);
	Motion& entity_motion = registry.motions.get(entity);
	int e_row = (entity_motion.position.y / world_tile_size) + centerY;
	int e_col = (entity_motion.position.x / world_tile_size) + centerX;
	int p_row = (player_motion.position.y / world_tile_size) + centerY;
	int p_col = (player_motion.position.x / world_tile_size) + centerX;
	//printf("entity position (x,y)=(%f,%f)\n", entity_motion.position.x, entity_motion.position.y);
	//printf("player position (x,y)=(%f,%f)\n", player_motion.position.x, player_motion.position.y);	
	printf("entity pos grid (x,y)=(%d,%d)\n", e_row, e_col);
	printf("player pos grid (x,y)=(%d,%d)\n", p_row, p_col);

	// https://gamedev.stackexchange.com/a/75179
	// chebyshev distance
	float length = max(abs(player_motion.position.y - entity_motion.position.y), abs(player_motion.position.x - entity_motion.position.x)) / world_tile_size;
	for (float i = 0.f; i <= length; ++i) {
		float delta = i / length;
		//printf("i %f\n", i);
		//printf("delta %f\n", delta);
		vec2 position_to_check = vec2_lerp(entity_motion.position, player_motion.position, delta);
		//printf("position_to_check (x,y)=(%f,%f)\n", position_to_check.x, position_to_check.y);
		int row = (position_to_check.y / world_tile_size) + centerY;
		int col = (position_to_check.x / world_tile_size) + centerX;
		printf("(row,col) to check (x,y)=(%d,%d)\n", row, col);
		createLine(position_to_check, vec2(10, 10));

		//int p_row = (player_motion.position.y / world_tile_size) + centerY;
		//int p_col = (player_motion.position.x / world_tile_size) + centerX;
		//printf("player (row, col): (%d,%d)\n", p_row, p_col);

		//printf("(row, col): (%d,%d)\n", row, col);

		if (!isValidCell(row, col)) break; // can't see outside of room
		//printf("here0\n");

		if (WorldSystem::world_map[row][col] == (int)TILE_TYPE::WALL) {
			printf("blocked by wall\n");
			// line of sight blocked by wall
			return false; 
		}
	}
	printf("not blocked by wall\n");
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

	can_see_player->setTrue(fire_at_player);
	can_see_player->setFalse(stop_firing);

	this->ghost.setRoot(can_see_player);
}