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

bool MapSystem::step(float elapsed_ms_since_last_update) {
	if (game_info.in_room != -1) {
		Room_struct& cur_room = game_info.room_index[game_info.in_room];

		// Lock door when in uncleared room

		// Marks room as cleared if it was uncleared and no enemies remain in room
		if (!cur_room.is_cleared) {
			if (cur_room.enemies.empty())
			{
				// Room is cleared
				cur_room.is_cleared == true;
				// Unlocks all doors
				for (Entity& door_entity : cur_room.doors) {
					Door& door = registry.doors.get(door_entity);
					door.is_locked = false;
				}
			}
			else {
				// Room has just been entered
				// Lock all doors
				for (Entity& door_entity : cur_room.doors) {
					Door& door = registry.doors.get(door_entity);
					door.is_locked = true;
				}
			}
		}
	}
	return true;
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

void MapSystem::spawnEnemiesInRoom() {
	Room_struct room;

	for (int room_num = 1; room_num < bsptree.rooms.size(); room_num++) {
		room = bsptree.rooms[room_num];
		if (bsptree.rooms[room_num].type == ROOM_TYPE::NORMAL) {
			std::vector<vec2> spawn_points;
			spawn_points.push_back(convert_grid_to_world(room.top_left + 1.f));
			spawn_points.push_back(convert_grid_to_world(room.bottom_right - 1.f));
			spawn_points.push_back(convert_grid_to_world(vec2(room.bottom_right.x - 1.f, room.top_left.y + 1.f)));
			spawn_points.push_back(convert_grid_to_world(vec2(room.top_left.x + 1.f, room.bottom_right.y - 1.f)));

			for (vec2 point : spawn_points) {
				float random_numer = uniform_dist(rng);
				if (random_numer <= 0.50) {
					createBeeEnemy(renderer, point);
				}
				else if (random_numer <= 0.75) {
					createWolfEnemy(renderer, point);
				}
				else if (random_numer <= 0.99) {
					createBomberEnemy(renderer, point);
				}
			}
		}
		else if (bsptree.rooms[room_num].type == ROOM_TYPE::BOSS) {
			createBoss(renderer, convert_grid_to_world((room.top_left + room.bottom_right) / 2.f));
		}
	}
}

Entity MapSystem::spawnPlayerInRoom(int room_number) {
	if (room_number < 0 || room_number >= bsptree.rooms.size()) assert(false && "Room number out of bounds");
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

	//// temporarily disable walls to test placement of entities
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
	createText(convert_grid_to_world({ 2.3, 9.5 }), vec3(0.8), "Hold to reduce\nhitbox to a dot", vec3(1, 1, 1), true, true);

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
	createText(convert_grid_to_world({ 24.f, 10.4f }), vec3(3), "/", vec3(1, 1, 1), true, true);
	createKey(convert_grid_to_world({ 25.f, 10.f }), vec2(90), KEYS::MOUSE_1, false, true, 1500);
	createText(convert_grid_to_world({ 22.5f, 11.f }), vec3(1.f), "Hold to shoot", vec3(1, 1, 1), true, true);

	// hardcoded dummy enemy spawn
	Entity entity1 = createDummyEnemySpawner(renderer, convert_grid_to_world({ 29, 5 }));
	DummyEnemySpawner& spawner1 = registry.dummyenemyspawners.get(entity1);
	spawner1.max_spawn = 5;

	// remaining buttons
	createKey(convert_grid_to_world({ 38, 16 }), vec2(120), KEYS::SCROLL, false, true);
	createText(convert_grid_to_world({ 36.f, 17.5f }), vec3(1.f), "Zoom camera in/out", vec3(1, 1, 1), true, true);

	createKey(convert_grid_to_world({ 43, 16 }), vec2(120), KEYS::P, false, true);
	createText(convert_grid_to_world({ 41.f, 17.5f }), vec3(1.f), "Toggle camera offset", vec3(1, 1, 1), true, true);

	createKey(convert_grid_to_world({ 48, 16 }), vec2(120), KEYS::F, false, true);
	createText(convert_grid_to_world({ 47.f, 17.5f }), vec3(1.f), "Show fps", vec3(1, 1, 1), true, true);

	createKey(convert_grid_to_world({ 59, 16 }), vec2(120), KEYS::R, false, true);
	createText(convert_grid_to_world({ 57.f, 17.5f }), vec3(1.f), "Return to main world", vec3(1, 1, 1), true, true);

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
	room.size = { room.grid[0].size(), room.grid.size() };
	room.x = world_width - room.size.x - 1;
	room.y = world_height / 2 - room.size.y / 2 - 1;

	rooms.push_back(room);
	addRoomToMap(room, world_map);
	return room;
}

void MapSystem::generateRandomMap() {
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

TILE_NAME_SANDSTONE MapSystem::get_tile_name_sandstone(int x, int y, std::vector<std::vector<int>>& map) {
	int type = map[y][x];
	// Specify int casted enums
	const int E = (int)TILE_TYPE::EMPTY;
	const int W = (int)TILE_TYPE::WALL;
	const int F = (int)TILE_TYPE::FLOOR;

	if (type == E) return TILE_NAME_SANDSTONE::NONE;
	else if (type == F) return TILE_NAME_SANDSTONE::AZTEC_FLOOR;

	// Specify neighbors for hardcoded checks
	const int U = map[y - 1][x];	// Up
	const int D = map[y + 1][x];	// Down
	const int L = map[y][x - 1];	// Left
	const int R = map[y][x + 1];	// Right
	const int UL = map[y - 1][x - 1];	// Up left
	const int UR = map[y - 1][x + 1];	// Up right
	const int DL = map[y + 1][x - 1];	// Down left
	const int DR = map[y + 1][x + 1];	// Down right

	TILE_NAME_SANDSTONE result = TILE_NAME_SANDSTONE::NONE;

	if (UR == F && U == W && R == W && D != F) {
		result = TILE_NAME_SANDSTONE::BOTTOM_LEFT;
	}
	else if (UL == F && U == W && L == W && D != F) {
		result = TILE_NAME_SANDSTONE::BOTTOM_RIGHT;
	}
	else if ((U == F && L == F && R == W && D == W) ||
		(U == F && L == W && R == W && D == W && DL == F && DR == E)) {
		result = TILE_NAME_SANDSTONE::CORRIDOR_BOTTOM_LEFT;
	}
	else if ((U == F && L == W && R == F && D == W) ||
		(U == F && L == W && R == W && D == W && DR == F && DL == E)) {
		result = TILE_NAME_SANDSTONE::CORRIDOR_BOTTOM_RIGHT;
	}
	else if (D == F ||
		(U == W && L == W && D == F && R == F) ||
		(U == W && L == F && D == F && R == W)) {
		result = TILE_NAME_SANDSTONE::TOP_WALL;
	}
	else if (L == F ||
		(L == W && DL == F && D == W)) {
		result = TILE_NAME_SANDSTONE::RIGHT_WALL;
	}
	else if (R == F ||
		(R == W && DR == F && D == W)) {
		result = TILE_NAME_SANDSTONE::LEFT_WALL;
	}
	else if (U == F) {
		result = TILE_NAME_SANDSTONE::BOTTOM_WALL;
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
			TILE_NAME_SANDSTONE result = get_tile_name_sandstone(x + 1, y + 1, map_copy);
			if (result == TILE_NAME_SANDSTONE::NONE) continue;
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