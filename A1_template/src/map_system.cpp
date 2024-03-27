// Adpated from: https://www.gamedeveloper.com/programming/procedural-dungeon-generation-algorithm#close-modal

// internal
#include "map_system.hpp"
#include "map_system.hpp"
#include "world_init.hpp"
#include "common.hpp"
#include <iostream>

int room_size = 11;

MapSystem::MapSystem() {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

void MapSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
}

void MapSystem::restart_map() {
	world_map = std::vector<std::vector<int>>(world_height, std::vector<int>(world_width, 0));
}

void MapSystem::spawnEnemies() {
	// testing boss enemy
	//for (Room& room : rooms) {
	//	coord world_coord = convert_grid_to_world(vec2(room.x + room.size.x / 2, room.y + room.size.y / 2 - 5));		
	//	//coord world_coord2 = convert_grid_to_world(vec2(room.x + room.size.x / 2, room.y + room.size.y / 2 - 4));		
	//	//coord world_coord3 = convert_grid_to_world(vec2(room.x + room.size.x / 2, room.y + room.size.y / 2 - 3));
	//	//coord world_coord4 = convert_grid_to_world(vec2(room.x + room.size.x / 2, room.y + room.size.y / 2 - 2));
	//	createBoss(renderer, world_coord);
	//	//createBeeEnemy(renderer, world_coord);
	//	//createBeeEnemy(renderer, world_coord2);
	//	//createWolfEnemy(renderer, world_coord3);
	//	//createBomberEnemy(renderer, world_coord4);
	//}

	for (Room room : rooms) {
		std::vector<vec2> spawn_points;
		spawn_points.push_back(vec2(room.x + 2, room.y + 2));
		spawn_points.push_back(vec2(room.x + room_size - 2, room.y + 2));
		spawn_points.push_back(vec2(room.x + 2, room.y + room_size - 2));
		spawn_points.push_back(vec2(room.x + room_size - 2, room.y + room_size - 2));
		vec2 world_coord;
		for (vec2 point : spawn_points) {
			world_coord = convert_grid_to_world(point);
			std::random_device ran;
			std::mt19937 gen(ran());
			std::uniform_real_distribution<> dis(0.0, 1.0);
			float random_numer = dis(gen);
			if (random_numer <= 0.33) {
				createBeeEnemy(renderer, world_coord);
			}
			else if (random_numer <= 0.66) {
				createWolfEnemy(renderer, world_coord);
			}
			else if (random_numer <= 0.99) {
				createBomberEnemy(renderer, world_coord);
			}
		}
	}
}

Entity MapSystem::spawnEnemiesInRoom() {
	Room2 room;

	for (int room_num = 1; room_num < bsptree.rooms.size(); room_num++) {
		room = bsptree.rooms[room_num];
		if (bsptree.rooms[room_num].type == ROOM_TYPE::NORMAL) {
			std::vector<vec2> spawn_points;
			spawn_points.push_back(convert_grid_to_world(room.top_left + 1.f));
			spawn_points.push_back(convert_grid_to_world(room.bottom_left - 1.f));
			spawn_points.push_back(convert_grid_to_world(vec2(room.bottom_left.x - 1.f, room.top_left.y + 1.f)));
			spawn_points.push_back(convert_grid_to_world(vec2(room.top_left.x + 1.f, room.bottom_left.y - 1.f)));

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
			createBoss(renderer, convert_grid_to_world((room.top_left + room.bottom_left) / 2.f));
		}
	}
	return createPlayer(renderer, convert_grid_to_world((bsptree.rooms[0].top_left + bsptree.rooms[0].bottom_left) / 2.f));
}

Room generateBasicRoom(int x, int y);
void addRoomToMap(const Room& room, std::vector<std::vector<int>>& map);
void addHallwayBetweenRoom(const Room& room1, const Room& room2, std::vector<std::vector<int>>& map);
void MapSystem::generateBasicMap() {
	rooms.clear();

	int room_radius = room_size >> 1;
	rooms.push_back(generateBasicRoom(room_radius, room_radius));
	rooms.push_back(generateBasicRoom(room_size + 2 * room_radius, room_radius));
	rooms.push_back(generateBasicRoom(3 * (room_size + room_radius), room_radius));
	rooms.push_back(generateBasicRoom(room_radius, room_size + 2 * room_radius));
	rooms.push_back(generateBasicRoom(room_size + 2 * room_radius, room_size + 2 * room_radius));
	rooms.push_back(generateBasicRoom(3 * (room_size + room_radius), room_size + 2 * room_radius));

	for (Room& room : rooms) {
		addRoomToMap(room, world_map);
	}

	addHallwayBetweenRoom(rooms[0], rooms[3], world_map);
	addHallwayBetweenRoom(rooms[0], rooms[1], world_map);
	addHallwayBetweenRoom(rooms[1], rooms[2], world_map);
	addHallwayBetweenRoom(rooms[2], rooms[3], world_map);
	addHallwayBetweenRoom(rooms[2], rooms[5], world_map);
	addHallwayBetweenRoom(rooms[4], rooms[5], world_map);

	// // Print the initialized array
	// for (int i = 0; i < world_map.size(); ++i) {
	//     for (int j = 0; j < world_map[i].size(); ++j) {
	//         std::cout << world_map[i][j] << " ";
	//     }
	//     std::cout << std::endl;
	// }

	generateAllEntityTiles(world_map);
}

Room& MapSystem::generateBossRoom() {
	rooms.clear();
	Room room;
	room.grid = {
		{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2},
		{2, 1, 1, 1, 2, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 2, 1, 1, 1, 2},
		{2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2},
		{2, 1, 1, 1, 2, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 2, 1, 1, 1, 2},
		{2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
	};
	room.size = { room.grid[0].size(), room.grid.size() };
	room.x = world_width - room.size.x - 1;
	room.y = world_height / 2 - room.size.y / 2 - 1;

	rooms.push_back(room);
	addRoomToMap(room, world_map);
	//generateAllEntityTiles(world_map);
	return room;
}

void MapSystem::generateRandomMap() {
	// Resets the map
	bsptree.corridors.clear();
	bsptree.rooms.clear();
	Room& boss_room = generateBossRoom();

	// Connect boss with last room
	bsptree.init(vec2(room_size), vec2(world_width - boss_room.size.x - 1, world_height));
	bsptree.generate_partitions(bsptree.root);
	bsptree.generate_rooms_random(bsptree.root);
	bsptree.generate_corridors(bsptree.root);
	bsptree.get_corridors(bsptree.root, bsptree.corridors);
	bsptree.generate_rooms(bsptree.root, bsptree.rooms);
	//bsptree.print_tree(bsptree.root);

	assert(bsptree.rooms.size() > 0 && bsptree.corridors.size() > 0);
	Corridor boss_corridor;
	Room2 boss_room2;
	boss_room2.top_left = vec2(boss_room.x, boss_room.y);
	boss_room2.bottom_left = boss_room2.top_left + boss_room.size - 1.f; // round issues probably
	boss_room2.type = ROOM_TYPE::BOSS;
	boss_corridor.start = (bsptree.rooms[bsptree.rooms.size() - 1].bottom_left +
		bsptree.rooms[bsptree.rooms.size() - 1].top_left) / 2.f;
	boss_corridor.end = (boss_room2.top_left + boss_room2.bottom_left) / 2.f;
	bsptree.corridors.push_back(boss_corridor);
	bsptree.rooms.push_back(boss_room2);

	// Populates the map with floors
	for (int i = 0; i < bsptree.rooms.size(); i++) {
		Room2& room2 = bsptree.rooms[i];
		for (int i = room2.top_left.y; i < room2.bottom_left.y; i++) {
			for (int j = room2.top_left.x; j < room2.bottom_left.x; j++) {
				world_map[i][j] = (int)TILE_TYPE::FLOOR;
			}
		}
	}

	bsptree.add_corridors_to_map(bsptree.corridors, world_map);
	bsptree.set_map_walls(world_map);

	generateAllEntityTiles(world_map);
}

Room generateBasicRoom(int x, int y) {
	Room room;
	room.id = room_id++; // Increments room id after assignment
	room.x = x;
	room.y = y;

	std::vector<std::vector<int>> grid = {
		{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	};
	room.grid = grid;
	return room;
}

void addRoomToMap(const Room& room, std::vector<std::vector<int>>& map) {
	int room_height = room.grid.size();
	for (int row = room.y; row < room_height + room.y; row++) {
		int room_width = room.grid[row - room.y].size();
		for (int col = room.x; col < room_width + room.x; col++) {
			// int max = room.grid[row-room.y].size() + room.x;
			// int shiftedY = row-room.y;
			// int shiftedX = col-room.x;
			// int roomVal = room.grid[row-room.y][col-room.x];

			map[row][col] = room.grid[row - room.y][col - room.x];
		}
	}
}

void addHallwayBetweenRoom(const Room& room1, const Room& room2, std::vector<std::vector<int>>& map) {
	int room1_half_height = room1.grid.size() >> 1;
	int room1_half_width = room1.grid[0].size() >> 1;

	int room2_half_height = room2.grid.size() >> 1;
	int room2_half_width = room2.grid[0].size() >> 1;

	vec2 midpoint1 = vec2(room2_half_width + room1.x, room1_half_height + room1.y);
	vec2 midpoint2 = vec2(room2_half_width + room2.x, room2_half_height + room2.y);

	if (room1.x == room2.x) { // 2 Rooms on top of each other
		int minY = (min(room1.y, room2.y) == room1.y) ? midpoint1.y + room1_half_height : midpoint2.y + room2_half_height;
		int maxY = (max(room1.y, room2.y) == room1.y) ? midpoint1.y - room1_half_height : midpoint2.y - room2_half_height;

		for (int y = minY; y <= maxY; ++y) {
			map[y][room1.x + room1_half_width] = 1; // Set floor tile
			map[y][room1.x + room1_half_width + 1] = 2; // Set hallway right wall tile
			map[y][room1.x + room1_half_width - 1] = 2; // Set hallway left wall tile
		}
	}
	else if (room1.y == room2.y) { // 2 Rooms on horizontal of each other
		int minX = (min(room1.x, room2.x) == room1.x) ? midpoint1.x + room1_half_width : midpoint2.x + room2_half_width;
		int maxX = (max(room1.x, room2.x) == room1.x) ? midpoint1.x - room1_half_width : midpoint2.x - room2_half_width;

		for (int x = minX; x <= maxX; ++x) {
			map[room1.y + room1_half_height][x] = 1; // Set floor tile
			map[room1.y + room1_half_height + 1][x] = 2; // Set hallway right wall tile
			map[room1.y + room1_half_height - 1][x] = 2; // Set hallway left wall tile
		}
	}
}

std::vector<TEXTURE_ASSET_ID> MapSystem::getTileAssetID(int row, int col, std::vector<std::vector<int>>& map) {
	std::vector<TEXTURE_ASSET_ID> textures;

	// Floor Tile
	if (map[row][col] == (int)TILE_TYPE::FLOOR) {
		textures.push_back((uniform_dist(rng) > 0.5) ? TEXTURE_ASSET_ID::TILE_1 : TEXTURE_ASSET_ID::TILE_2);
		return textures;
	}

	// map permimeter
	if (row * col == 0 || row * col == map.size() * map[0].size()) {
		return textures;
		// The current tile is on the edge of the map
	}

	// TODO: Inefficent case by case checks. Optimize with a map if it takes too much time
	// Top left corner
	if (map[row - 1][col] == (int)TILE_TYPE::EMPTY && map[row][col - 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_TOP_CORNER_WALL);
		return textures;
	}

	// Top Right corner
	if (map[row + 1][col] == (int)TILE_TYPE::WALL && map[row][col - 1] == (int)TILE_TYPE::WALL) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_TOP_CORNER_WALL);
		return textures;
	}

	// Bot left corner
	if (map[row - 1][col] == (int)TILE_TYPE::WALL && map[row][col + 1] == (int)TILE_TYPE::WALL) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Bot right corner
	if (map[row - 1][col] == (int)TILE_TYPE::WALL && map[row][col - 1] == (int)TILE_TYPE::WALL) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Inner top left
	if (map[row + 1][col] == (int)TILE_TYPE::FLOOR && map[row][col + 1] == (int)TILE_TYPE::FLOOR) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Inner top right
	if (map[row + 1][col] == (int)TILE_TYPE::FLOOR && map[row][col - 1] == (int)TILE_TYPE::FLOOR) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Inner bot left
	if (map[row - 1][col] == (int)TILE_TYPE::FLOOR && map[row][col + 1] == (int)TILE_TYPE::FLOOR) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_TOP_CORNER_WALL);
		return textures;
	}

	// Inner bot right
	if (map[row - 1][col] == (int)TILE_TYPE::FLOOR && map[row][col - 1] == (int)TILE_TYPE::FLOOR) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_TOP_CORNER_WALL);
		return textures;
	}

	// Top Wall
	if (map[row + 1][col] == (int)TILE_TYPE::FLOOR) {
		textures.push_back(TEXTURE_ASSET_ID::WALL_SURFACE);
		textures.push_back(TEXTURE_ASSET_ID::TOP_WALL);
		return textures;
	}

	// Left wall
	if (map[row][col + 1] == (int)TILE_TYPE::FLOOR) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_WALL);
		return textures;
	}

	// Right wall
	if (map[row][col - 1] == (int)TILE_TYPE::FLOOR) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_WALL);
		return textures;
	}

	// Bot wall
	if (map[row - 1][col] == (int)TILE_TYPE::FLOOR) {
		// textures.push_back((uniform_dist(rng) > 0.5) ? TEXTURE_ASSET_ID::TILE_1 : TEXTURE_ASSET_ID::TILE_2);
		textures.push_back(TEXTURE_ASSET_ID::BOTTOM_WALL);
		return textures;
	}

	// Should not return here
	return textures;
}

void MapSystem::addTile(int row, int col, std::vector<TEXTURE_ASSET_ID>& textureIDs, std::vector<std::vector<int>>& map) {

	coord world_coord = convert_grid_to_world({ col, row });

	// Gets the proper texture id list given the position
	switch (map[row][col])
	{
	case (int)TILE_TYPE::WALL:
		textureIDs = getTileAssetID(row, col, map);
		createWall(renderer, world_coord, textureIDs);
		break;
	case (int)TILE_TYPE::FLOOR:
		textureIDs = getTileAssetID(row, col, map);
		createFloor(renderer, world_coord, textureIDs);
		break;
	default:
		break;
	}
}

void MapSystem::generateAllEntityTiles(std::vector<std::vector<int>>& map) {
	for (int row = 0; row < map.size(); row++) {
		for (int col = 0; col < map[row].size(); col++) {
			std::vector<TEXTURE_ASSET_ID> textureIDs;
			addTile(row, col, textureIDs, map);
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