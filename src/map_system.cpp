// internal
#include "map_system.hpp"
#include "map_system.hpp"
#include "world_init.hpp"
#include "common.hpp"
#include "components.hpp"
#include <iostream>


int room_size = 11; // Must be at least >= 4

MapSystem::MapSystem() {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

void MapSystem::init(RenderSystem* renderer_arg, VisibilitySystem* visibility_arg) {
	this->renderer = renderer_arg;
	this->visibility_system = visibility_arg;
}

void MapSystem::step(float elapsed_ms_since_last_update) {
	if (game_info.in_room != -1) {
		Room_struct& cur_room = game_info.room_index[game_info.in_room];

		// Lock door when in uncleared room

		// Marks room as cleared if it was uncleared and no enemies remain in room
		if (!cur_room.is_cleared) {
			if (cur_room.enemies.empty())
			{
				// Room is cleared
				cur_room.is_cleared = true;
				// Unlocks all doors
				for (Entity& door_entity : cur_room.doors) {
					Door& door = registry.doors.get(door_entity);
					door.is_locked = false;

					if (door.is_visited) {
						renderer->switch_door_texture(door_entity, false);
					}
				}
			}
			else {
				// Room has just been entered
				// Lock all doors
				for (Entity& door_entity : cur_room.doors) {
					Door& door = registry.doors.get(door_entity);
					door.is_locked = true;
					renderer->switch_door_texture(door_entity, true);
				}
			}
		}
		else {
			if (cur_room.type == ROOM_TYPE::BOSS) {
				// only one teleporter in the map at a time
				if (registry.teleporters.size() < 1) {
					createTeleporter(renderer, convert_grid_to_world((cur_room.top_left + cur_room.bottom_right) / 2.f), 2.f);
					createChest(renderer, convert_grid_to_world((cur_room.top_left + cur_room.bottom_right) / 2.f+ vec2(0.0f, 4.0f)));
				}
			}
		}

	}

}

void MapSystem::restart_map() {
	world_map = std::vector<std::vector<int>>(world_height, std::vector<int>(world_width, 0));
}

bool MapSystem::is_valid_map(std::vector<std::vector<int>>& map) {
	if (room_size <= 3) return true;
	int map_height = map.size();
	int map_width = map[0].size();
	assert(map_height > 0 && map_width > 0 && "Map should have at least one cell");
	// Create padding of empty tile in the edges by copy
	auto map_copy = std::vector<std::vector<int>>(map_height + 2, std::vector<int>(map_width + 2, 0));
	for (int y = 0; y < map_height; ++y) {
		for (int x = 0; x < map_width; ++x) {
			map_copy[y + 1][x + 1] = map[y][x];
		}
	}

	for (int y = 0; y < map_height; ++y) {
		for (int x = 0; x < map_width; ++x) {
			if (map[y][x] == (int)TILE_TYPE::EMPTY || map[y][x] == (int)TILE_TYPE::FLOOR) continue;
			const int U = map_copy[y - 1 + 1][x + 1];	// Up
			const int D = map_copy[y + 1 + 1][x + 1];	// Down
			const int L = map_copy[y + 1][x - 1 + 1];	// Left
			const int R = map_copy[y + 1][x + 1 + 1];	// Right
			const int F = (int)TILE_TYPE::FLOOR;
			// False if there is floor on both sides (too many cases to handle + no tile set for this situation)
			if ((U == F && D == F) || (L == F && R == F)) return false;
		}
	}
	return true;
}

void MapSystem::spawnEnemiesInRooms() {
	Room_struct room;
	for (int room_num = 0; room_num < bsptree.rooms.size(); room_num++) {
		room = bsptree.rooms[room_num];
		spawnEnemiesInRoom(room);
	}
}

// spawn enemies inside room
void MapSystem::spawnEnemiesInRoom(Room_struct& room)
{
	if (room.type == ROOM_TYPE::NORMAL) {
		std::vector<vec2> spawn_points;
		std::uniform_int_distribution<> int_distrib(2, 3);
		int enemy_num = static_cast<int>(int_distrib(rng) * combo_mode.combo_meter * 2);
		for (int i = 0; i < enemy_num; i++) {
			std::uniform_real_distribution<> real_distrib_x(room.top_left.x + 0.3f, room.bottom_right.x - 0.3f);
			float loc_x = real_distrib_x(rng);
			std::uniform_real_distribution<> real_distrib_y(room.top_left.y + 0.3f, room.bottom_right.y - 0.3f);
			float loc_y = real_distrib_y(rng);
			spawn_points.push_back(convert_grid_to_world(vec2(loc_x, loc_y)));
		}
		std::uniform_real_distribution<> real_dist(0, 1);
		for (vec2 point : spawn_points) {

			float random_number = real_dist(rng);
			if (map_info.level == MAP_LEVEL::LEVEL1) {
				if (random_number <= 0.50) {
					room.enemies.push_back(createGargoyleEnemy(renderer, point));
				}
				else if (random_number <= 0.75) {
					room.enemies.push_back(createWolfEnemy(renderer, point));
				}
				else {
					room.enemies.push_back(createBomberEnemy(renderer, point));
				}
			}
			else if (map_info.level == MAP_LEVEL::LEVEL2) {
				if (random_number <= 0.50) {
					room.enemies.push_back(createBeeEnemy(renderer, point));
				}
				else if (random_number <= 0.7) {
					room.enemies.push_back(createBee2Enemy(renderer, point));
				}
				else if (random_number <= 0.9) {
					room.enemies.push_back(createWormEnemy(renderer, point));
				}
				else {
					room.enemies.push_back(createLizardEnemy(renderer, point));
				}
			}


			//if (random_number <= 0.50) {
			//	room.enemies.push_back(createBeeEnemy(renderer, point));
			//}
			//else if (random_number <= 0.75) {
			//	room.enemies.push_back(createWolfEnemy(renderer, point));
			//}
			//else if (random_number <= 1.0) {
			//	room.enemies.push_back(createBomberEnemy(renderer, point));
			//}

			//room.enemies.push_back(createLizardEnemy(renderer, point));
			//room.enemies.push_back(createWormEnemy(renderer, point));
			//room.enemies.push_back(createBee2Enemy(renderer, point));
			//room.enemies.push_back(createGargoyleEnemy(renderer, point));

			//if (random_number <= 0.25) {
			//	room.enemies.push_back(createLizardEnemy(renderer, point));
			//}
			//else if (random_number <= 0.50) {
			//	room.enemies.push_back(createBee2Enemy(renderer, point));
			//}
			//else if (random_number <= 0.75) {
			//	room.enemies.push_back(createWormEnemy(renderer, point));
			//}
			//else {
			//	room.enemies.push_back(createGargoyleEnemy(renderer, point));
			//}
		}
	}
	else if (room.type == ROOM_TYPE::BOSS) {
		if (map_info.level == MAP_LEVEL::LEVEL1) {
			room.enemies.push_back(createBoss(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f), "Cirno, the Ice Fairy", BOSS_ID::CIRNO, vec3(0, 0, 1)));
		}
		else if (map_info.level == MAP_LEVEL::LEVEL2) {
			room.enemies.push_back(createBoss(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f), "Flandre, the Scarlet Devil", BOSS_ID::FLANDRE, vec3(1, 0, 0)));
		}
	}
	else if (room.type == ROOM_TYPE::START) {
		// spawn nothing
	}
	else if (room.type == ROOM_TYPE::SHOP) {
		createNPC(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f));
		createPurchasableHealth(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f + vec2(1.414f, 1.414f)) );
		createPurchasableHealth(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f + vec2(-1.414f, 1.414f)) );
		createPurchasableHealth(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f + vec2(1.414f, -1.414f)) );
		createPurchasableHealth(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f + vec2(-1.414f, -1.414f)) );
		createTreasure(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f - vec2(1.f, 0.f)));
		createTreasure(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f + vec2(1.f, 0.f)));
		createTreasure(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f - vec2(0.f, 1.f)));
		createTreasure(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f + vec2(0.f, 1.f)));
		createTreasure(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f - vec2(2 * 1.f, 0.f)));
		createTreasure(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f + vec2(2 * 1.f, 0.f)));
		createTreasure(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f - vec2(0.f, 2 * 1.f)));
		createTreasure(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f + vec2(0.f, 2 * 1.f)));
	}
}

Entity MapSystem::spawnPlayerInRoom(int room_number) {
	if (room_number < 0 || room_number >= bsptree.rooms.size()) assert(false && "Room number out of bounds");
	bsptree.rooms[room_number].type = ROOM_TYPE::START;
	Room_struct& room = game_info.room_index[room_number];
	room.type = ROOM_TYPE::START;
	room.is_cleared = true;
	room.need_to_spawn = false;

	//room_number = bsptree.rooms.size() - 1;
	//return createPlayer(renderer, convert_grid_to_world(bsptree.rooms[room_number].top_left - vec2(2, 0)));
	return createPlayer(renderer, convert_grid_to_world((bsptree.rooms[room_number].top_left + bsptree.rooms[room_number].bottom_right) / 2.f));
}

// Getting out of map results? Consider that there is empty padding in the world map.
Entity MapSystem::spawnPlayer(coord grid_coord) {
	return createPlayer(renderer, convert_grid_to_world(grid_coord));
}

void addRoomToMap(const Room& room, std::vector<std::vector<int>>& map) {
	int room_height = room.grid.size();
	for (int row = room.y; row < room_height + room.y; row++) {
		int room_width = room.grid[row - room.y].size();
		for (int col = room.x; col < room_width + room.x; col++) {
			map[row][col] = room.grid[row - room.y][col - room.x];
		}
	}
}

void MapSystem::generateTutorialMap() {

	std::vector<std::vector<int>> grid = {
		{}, // originally was full of walls, needed since keys are placed before this removing this
		{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
		{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,0,2,2,2,2,2,2,2,2,2,2,2},
		{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,0,2,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,0,2,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,0,2,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,0,2,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,2,1,1,2,0,0,2,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,2,1,1,2,2,2,2,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,2,2,2,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2},
		{2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,2},
		{2,1,1,2,0,0,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
		{2,1,1,2,0,0,2,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,2,0,0,2,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,2,0,0,2,1,1,1,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,2,0,0,2,2,2,2,2,2,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,2,0,0,0,0,0,0,0,0,2,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
		{2,1,1,2,0,0,0,0,0,0,0,0,2,1,1,2},
		{2,1,1,2,2,2,2,2,2,2,2,2,2,1,1,2},
		{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
		{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
		{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
	};

	////temporarily disable walls to test placement of entities
	//for (int y = 0; y < grid.size(); ++y) {
	//	for (int x = 0; x < grid[y].size(); ++x) {
	//		if (grid[y][x] == 2) grid[y][x] = 1;
	//	}
	//}

	// get max width of grid
	size_t max_width = 0;
	for (int y = 0; y < grid.size(); ++y) {
		max_width = max(max_width, grid[y].size());
	}
	// manually restart world map (with edge padding of 1 cell)
	world_map = std::vector<std::vector<int>>(grid.size() + 2, std::vector<int>(max_width + 2, 0));
	// adjust world map attributes
	world_center = { 9, 16 };

	// place entities relative to grid
	auto create_wasd = [](KEYS key, vec2 pos, int key_size) {
		createKey(vec2(0, -key_size + key_size / 4.f) + pos, { key_size, key_size }, KEYS::W, false, key == KEYS::W ? true : false);
		createKey(vec2(-key_size + key_size / 4.f, 0) + pos, { key_size, key_size }, KEYS::A, false, key == KEYS::A ? true : false);
		createKey(vec2(0, 0) + pos, { key_size, key_size }, KEYS::S, false, key == KEYS::S ? true : false);
		createKey(vec2(key_size - key_size / 4.f, 0) + pos, { key_size, key_size }, KEYS::D, false, key == KEYS::D ? true : false);
		};

	create_wasd(KEYS::D, convert_grid_to_world({ 12, 16 }), 60);
	create_wasd(KEYS::S, convert_grid_to_world({ 14.5, 20 }), 60);
	create_wasd(KEYS::A, convert_grid_to_world({ 9, 23 }), 60);
	create_wasd(KEYS::W, convert_grid_to_world({ 2.5, 20 }), 60);

	// shift key focus mode
	createKey(convert_grid_to_world({ 3.5, 8.5 }), vec2(150), KEYS::SHIFT, false, true, 1300);
	createText(convert_grid_to_world({ 3.5, 9.5 }), vec3(0.8), "Hold to reduce\nhitbox to a dot", vec3(1, 1, 1), true, true);

	// E to bomb
	createKey(convert_grid_to_world({ 16, 4 }), vec2(120), KEYS::E, false, true, 1300);
	createText(convert_grid_to_world({ 16, 5 }), vec3(0.8), "Press Q to clear all enemy bullets\n Need one bomb", vec3(1, 1, 1), true, true);

	// hardcoded bullet for this specific grid only
	for (int j = 0; j < 7; ++j) {
		Entity entity = createInvisible(renderer, convert_grid_to_world({ 21.25, 1.6 + j / 1.6f + 1 }));
		BulletSpawner& spawner = registry.bulletSpawners.emplace(entity);
		spawner.start_angle = 180.f; // face the left direction
		spawner.fire_rate = 60;
		spawner.is_firing = true;
	}

	// space/mouse 1 key attack
	createKey(convert_grid_to_world({ 23.f, 10.f }), vec2(150), KEYS::SPACE, false, true, 1300);
	createText(convert_grid_to_world({ 24.f, 10.f }), vec3(3), "/", vec3(1, 1, 1), true, true);
	createKey(convert_grid_to_world({ 25.f, 10.f }), vec2(90), KEYS::MOUSE_1, false, true, 1500);
	createText(convert_grid_to_world({ 24.f, 11.f }), vec3(1.f), "Hold to shoot", vec3(1, 1, 1), true, true);

	createText(convert_grid_to_world({ 29.f, 10.f }), vec3(1.f), "Combo Meter on top right\nIncreases game speed", vec3(1, 1, 1), true, true);

	// hardcoded dummy enemy spawn
	Entity entity1 = createDummyEnemySpawner(renderer, convert_grid_to_world({ 29, 5 }));
	DummyEnemySpawner& spawner1 = registry.dummyenemyspawners.get(entity1);
	spawner1.max_spawn = 5;

	// remaining buttons
	createKey(convert_grid_to_world({ 38, 16 }), vec2(120), KEYS::SCROLL, false, true);
	createText(convert_grid_to_world({ 38.f, 17.5f }), vec3(1.f), "Zoom camera in/out", vec3(1, 1, 1), true, true);

	createKey(convert_grid_to_world({ 43, 16 }), vec2(120), KEYS::P, false, true);
	createText(convert_grid_to_world({ 43.f, 17.5f }), vec3(1.f), "Toggle camera offset", vec3(1, 1, 1), true, true);

	createKey(convert_grid_to_world({ 48, 16 }), vec2(120), KEYS::F, false, true);
	createText(convert_grid_to_world({ 48.f, 17.5f }), vec3(1.f), "Show fps", vec3(1, 1, 1), true, true);

	createKey(convert_grid_to_world({ 59, 16 }), vec2(120), KEYS::R, false, true);
	createText(convert_grid_to_world({ 59.f, 17.5f }), vec3(1.f), "Return to main world", vec3(1, 1, 1), true, true);

	// Add grid to map
	for (int y = 0; y < grid.size(); ++y) {
		for (int x = 0; x < grid[y].size(); ++x) {
			world_map[y + 1][x + 1] = grid[y][x];
		}
	}
	//generateAllEntityTiles(world_map);
	generate_all_tiles(world_map);
}

Room MapSystem::generateBossRoom() {
	rooms.clear();
	Room room;
	if (map_info.level == MAP_LEVEL::LEVEL2) {
		room.grid = {
			{1, 1, 1, 1, 1, 1, 2, 2, 0, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 2, 2, 0, 2, 2, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2},
			{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 2, 2, 0, 2, 2, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 2, 2, 0, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1},
		};
	}
	else {
		room.grid = {
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1},
			{1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1},
			{1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		};
	}
	room.size = { room.grid[0].size(), room.grid.size() };
	room.x = world_width - room.size.x - 1;
	room.y = world_height / 2 - room.size.y / 2 - 1;

	rooms.push_back(room);
	addRoomToMap(room, world_map);
	return room;
}

void MapSystem::generateRandomMap(float room_size) {
	assert(room_size > 3 && "Room too small!");
	bool is_valid = false;

	while (!is_valid) {
		// Resets the map
		restart_map();
		bsptree.rooms.clear();
		Room boss_room = generateBossRoom();

		bsptree.init(vec2(room_size), vec2(world_width - (boss_room.size.x + 2), world_height));
		bsptree.generate_partitions(bsptree.root);
		bsptree.generate_rooms_random(bsptree.root);
		bsptree.generate_corridors(bsptree.root);

		// Connect boss room
		assert(bsptree.rooms.size() > 0);
		Room_struct boss_room2;
		boss_room2.top_left = vec2(boss_room.x, boss_room.y);
		boss_room2.bottom_right = boss_room2.top_left + boss_room.size - 1.f; // round issues probably
		boss_room2.type = ROOM_TYPE::BOSS;
		std::uniform_int_distribution<> int_distrib(0, bsptree.rooms.size() - 1);
		int random_num = int_distrib(rng);
		vec2 start = (bsptree.rooms[random_num].bottom_right +
			bsptree.rooms[random_num].top_left) / 2.f;
		vec2 end = (boss_room2.top_left + boss_room2.bottom_right) / 2.f;

		bsptree.rooms.push_back(boss_room2);
		bsptree.generate_corridor_between_two_points(start, end);
		std::uniform_int_distribution<> shop_distrib(1, static_cast<int>((bsptree.rooms.size() - 2)/2));
		random_num = shop_distrib(rng);
		bsptree.rooms[random_num].type = ROOM_TYPE::SHOP;
		set_map_walls(world_map);
		is_valid = is_valid_map(world_map);
	}

	// ignore these - for visibility
	// ORDER IS IMPORTANT!
	// for order of operations, please see VisibilitySystem class
	visibility_system->restart_map();
	generate_all_tiles(world_map);
	visibility_system->init_visibility();
	// set buffer data for visibility tile instance rendering
	renderer->set_visibility_tiles_instance_buffer_max();
	// set buffer data for tile instance rendering
	renderer->set_tiles_instance_buffer();

	// add all rooms to component
	for (int i = 0; i < bsptree.rooms.size(); ++i) {
		game_info.add_room(bsptree.rooms[i]);
	}

	// generates door info then the tiles
	generate_door_tiles(world_map);
	// Generates rocks in room
	generate_rocks(world_map);
}

vec4 addSingleDoor(int row, int col, DIRECTION dir, int room_index, Room_struct& room, std::vector<std::vector<int>>& map) {
	map[row][col] = (int)TILE_TYPE::DOOR;
	vec4 door_info = { col, row, dir, room_index };
	return door_info;
}

/*
* Adds doors to the grid map based on if the room edges is a wall
* MUST BE AFTER WALL GENERATION
*/
std::vector<vec4> MapSystem::generateDoorInfo(std::vector<Room_struct>& rooms, std::vector<std::vector<int>>& map) {
	std::vector<vec4> doors;

	int map_height = map.size();
	int map_width = map[0].size();
	assert(map_height > 0 && map_width > 0 && "Map should have at least one cell");
	// Create padding of empty tile in the edges by copy
	auto map_copy = std::vector<std::vector<int>>(map_height + 2, std::vector<int>(map_width + 2, 0));
	for (int y = 0; y < map_height; ++y) {
		for (int x = 0; x < map_width; ++x) {
			map_copy[y + 1][x + 1] = map[y][x];
		}
	}

	for (int i = 0; i < rooms.size(); i++)
	{
		Room_struct& room = rooms[i];

		// Check edges of room which are walls, when there is a floor, add a door
		for (int x = room.top_left.x - 1; x <= room.bottom_right.x + 1; x++) {
			// top edge
			if (map_copy[room.top_left.y][x + 1] == (int)TILE_TYPE::FLOOR) {
				doors.push_back(addSingleDoor(room.top_left.y - 1, x, DIRECTION::DOWN, i, room, world_map));
			}
			// bottom edge
			if (map_copy[room.bottom_right.y + 2][x + 1] == (int)TILE_TYPE::FLOOR) {
				doors.push_back(addSingleDoor(room.bottom_right.y + 1, x, DIRECTION::UP, i, room, world_map));
			}
		}

		for (int y = room.top_left.y - 1; y <= room.bottom_right.y + 1; y++) {
			// left edge
			if (map_copy[y + 1][room.top_left.x] == (int)TILE_TYPE::FLOOR) {
				doors.push_back(addSingleDoor(y, room.top_left.x - 1, DIRECTION::RIGHT, i, room, world_map));
			}
			// right edge
			if (map_copy[y + 1][room.bottom_right.x + 2] == (int)TILE_TYPE::FLOOR) {
				doors.push_back(addSingleDoor(y, room.bottom_right.x + 1, DIRECTION::LEFT, i, room, world_map));
			}
		}
	}

	return doors;
}

void MapSystem::set_map_walls(std::vector<std::vector<int>>& map) {
	const std::vector<coord> ACTIONS = {
		vec2(0, -1),	// UP
		vec2(0, 1),		// DOWN
		vec2(-1, 0),	// LEFT
		vec2(1, 0),		// RIGHT
		vec2(-1, -1),	// UP LEFT
		vec2(1, -1),	// UP RIGHT
		vec2(-1, 1),	// DOWN LEFT
		vec2(1, 1)		// DOWN RIGHT
	};

	const int map_height = map.size();
	const int map_width = map[0].size();
	assert(map_height > 0 && map_width > 0 && "Adding to empty map");

	// supports edges of the map
	for (int row = 0; row < map.size(); row++) {
		for (int col = 0; col < map[row].size(); col++) {
			// Only check for empty tiles
			if (map[row][col] != (int)TILE_TYPE::EMPTY) continue;
			for (const coord& action : ACTIONS) {
				const vec2 candidate_cell = vec2(col, row) + action;
				if (candidate_cell.x < 0 || candidate_cell.x >= map_width ||
					candidate_cell.y < 0 || candidate_cell.y >= map_height ||
					map[candidate_cell.y][candidate_cell.x] != (int)TILE_TYPE::FLOOR) continue;
				map[row][col] = (int)TILE_TYPE::WALL;
				break;
			}
		}
	}
}

TILE_NAME MapSystem::get_tile_name(int x, int y, std::vector<std::vector<int>>& map) {
	int type = map[y][x];
	// Specify int casted enums
	const int E = (int)TILE_TYPE::EMPTY;
	const int W = (int)TILE_TYPE::WALL;
	const int F = (int)TILE_TYPE::FLOOR;

	if (type == E) return TILE_NAME::NONE;
	else if (type == F) {
		TILE_NAME temp = TILE_NAME::FLOOR_3_1;
		switch (map_info.level) {
		case MAP_LEVEL::LEVEL1:
			temp = TILE_NAME::FLOOR_1_0;
			break;
		case MAP_LEVEL::LEVEL2: {
			std::random_device ran;
			std::mt19937 gen(ran());
			std::uniform_real_distribution<> dis(0.f, 1.f);
			float random_number = dis(gen);
			if (random_number < 0.4f) {
				temp = TILE_NAME::DEFAULT_FLOOR;
			}
			else if (random_number < 0.5) {
				temp = TILE_NAME::FLOOR_1_0;
			}
			else if (random_number < 0.6) {
				temp = TILE_NAME::FLOOR_2_0;
			}
			else if (random_number < 0.7) {
				temp = TILE_NAME::FLOOR_3_0;
			}
			else if (random_number < 0.8) {
				temp = TILE_NAME::FLOOR_3_1;
			}
			else {
				temp = TILE_NAME::FLOOR_0_1;
			}
			break;
		}
		default:
			break;
		}
		return temp;
	}

	// Specify neighbors for hardcoded checks
	const int U = map[y - 1][x];	// Up
	const int D = map[y + 1][x];	// Down
	const int L = map[y][x - 1];	// Left
	const int R = map[y][x + 1];	// Right
	const int UL = map[y - 1][x - 1];	// Up left
	const int UR = map[y - 1][x + 1];	// Up right
	const int DL = map[y + 1][x - 1];	// Down left
	const int DR = map[y + 1][x + 1];	// Down right

	TILE_NAME result = TILE_NAME::NONE;

	if (UR == F && U == W && R == W && D != F) {
		result = TILE_NAME::BOTTOM_LEFT;
	}
	else if (UL == F && U == W && L == W && D != F) {
		result = TILE_NAME::BOTTOM_RIGHT;
	}
	else if ((U == F && L == F && R == W && D == W) ||
		(U == F && L == W && R == W && D == W && DL == F && DR == E)) {
		result = TILE_NAME::CORRIDOR_BOTTOM_LEFT;
	}
	else if ((U == F && L == W && R == F && D == W) ||
		(U == F && L == W && R == W && D == W && DR == F && DL == E)) {
		result = TILE_NAME::CORRIDOR_BOTTOM_RIGHT;
	}
	else if (D == F ||
		(U == W && L == W && D == F && R == F) ||
		(U == W && L == F && D == F && R == W)) {
		result = TILE_NAME::TOP_WALL;
	}
	else if (L == F ||
		(L == W && DL == F && D == W)) {
		result = TILE_NAME::RIGHT_WALL;
	}
	else if (R == F ||
		(R == W && DR == F && D == W)) {
		result = TILE_NAME::LEFT_WALL;
	}
	else if (U == F) {
		result = TILE_NAME::BOTTOM_WALL;
	}

	return result;
}

void MapSystem::generate_door_tiles(std::vector<std::vector<int>>& map) {
	std::vector<vec4> door_info = generateDoorInfo(bsptree.rooms, world_map);

	// Loops through all rooms and creates a door entity for every marked door
	// door_info vec4 order: col, row, direction, room_index
	for (const vec4& door : door_info) {
		// let's convert grid to world inside the createDoor
		// IMPORTANT: createDoor takes in grid coordinates
		createDoor(renderer, { door[0], door[1] }, static_cast<DIRECTION>(door[2]), door[3]);
	}
}

// Sets wall tiles that act like rocks in a room
void MapSystem::generate_rocks(std::vector<std::vector<int>>& map) {
	float rock_spawn_chance = 0.05f;
	float to_spawn = 0.f;
	for (Room_struct& room : bsptree.rooms) {
		if (room.type != ROOM_TYPE::NORMAL) {
			continue;
		}
		for (int row = room.top_left.y + 1; row < room.bottom_right.y; row++) {
			for (int col = room.top_left.x + 1; col < room.bottom_right.x; col++) {
				to_spawn = uniform_dist(rng);
				//map[row][col] = (to_spawn < rock_spawn_chance) ? (int)TILE_TYPE::WALL : (int)TILE_TYPE::FLOOR;
				if (to_spawn < rock_spawn_chance) {
					createRock(renderer, { col, row });
				}

			}
		}
	}
}

void MapSystem::generate_all_tiles(std::vector<std::vector<int>>& map) {
	int map_height = map.size();
	int map_width = map[0].size();
	assert(map_height > 0 && map_width > 0 && "Map should have at least one cell");
	// Create padding of empty tile in the edges by copy
	auto map_copy = std::vector<std::vector<int>>(map_height + 2, std::vector<int>(map_width + 2, 0));
	for (int y = 0; y < map_height; ++y) {
		for (int x = 0; x < map_width; ++x) {
			map_copy[y + 1][x + 1] = map[y][x];
		}
	}

	for (int y = 0; y < map_height; ++y) {
		for (int x = 0; x < map_width; ++x) {
			TILE_NAME result = get_tile_name(x + 1, y + 1, map_copy);
			if (result == TILE_NAME::NONE) continue;
			createTile(renderer, visibility_system, { x, y }, result, map[y][x] == (int)TILE_TYPE::WALL);
		}
	}
}


void MapSystem::printMap() {
	for (int i = 0; i < world_map.size(); i++) {
		for (int j = 0; j < world_map[0].size(); j++) {
			printf("%d ", world_map[i][j]);
		}
		printf("\n");
	}
}