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

	//bsptree.init(vec2(room_size), vec2(world_width, world_height));
	//bsptree.generate_partitions(bsptree.root);
	//bsptree.print_tree(bsptree.root);
}

void MapSystem::spawnEnemies() {

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

Room generateBasicRoom(int x, int y);
void addRoomToMap(const Room& room, std::vector<std::vector<int>>& map);
void addHallwayBetweenRoom(const Room& room1, const Room& room2, std::vector<std::vector<int>>& map);
void MapSystem::generateBasicMap() {

	std::vector<std::vector<int>> map(world_height, std::vector<int>(world_width, 0));
	rooms.clear();
	//  std::vector<Room> rooms;
	int room_radius = room_size >> 1;
	rooms.push_back(generateBasicRoom(room_radius, room_radius));
	rooms.push_back(generateBasicRoom(room_size + 2 * room_radius, room_radius));
	rooms.push_back(generateBasicRoom(3 * (room_size + room_radius), room_radius));
	rooms.push_back(generateBasicRoom(room_radius, room_size + 2 * room_radius));
	rooms.push_back(generateBasicRoom(room_size + 2 * room_radius, room_size + 2 * room_radius));
	rooms.push_back(generateBasicRoom(3 * (room_size + room_radius), room_size + 2 * room_radius));

	for (Room& room : rooms) {
		addRoomToMap(room, map);
	}

	addHallwayBetweenRoom(rooms[0], rooms[3], map);
	addHallwayBetweenRoom(rooms[0], rooms[1], map);
	addHallwayBetweenRoom(rooms[1], rooms[2], map);
	addHallwayBetweenRoom(rooms[2], rooms[3], map);
	addHallwayBetweenRoom(rooms[2], rooms[5], map);
	addHallwayBetweenRoom(rooms[4], rooms[5], map);

	// // Print the initialized array
	// for (int i = 0; i < map.size(); ++i) {
	//     for (int j = 0; j < map[i].size(); ++j) {
	//         std::cout << map[i][j] << " ";
	//     }
	//     std::cout << std::endl;
	// }

	MapSystem::generateAllEntityTiles(map);

	world_map = map;
}

void MapSystem::generateMap(int floor) {
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
	if (map[row - 1][col] == (int)TILE_TYPE::EMPTY && map[row][col + 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_TOP_CORNER_WALL);
		return textures;
	}

	// Top Wall
	if (map[row - 1][col] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::WALL_SURFACE);
		textures.push_back(TEXTURE_ASSET_ID::TOP_WALL);
		return textures;
	}

	// Bot left corner
	if (map[row + 1][col] == (int)TILE_TYPE::EMPTY && map[row][col - 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Bot right corner
	if (map[row + 1][col] == (int)TILE_TYPE::EMPTY && map[row][col + 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Left wall
	if (map[row][col - 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_WALL);
		return textures;
	}

	// Right wall
	if (map[row][col + 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_WALL);
		return textures;
	}

	// Bot wall
	if (map[row + 1][col] == (int)TILE_TYPE::EMPTY) {
		// textures.push_back((uniform_dist(rng) > 0.5) ? TEXTURE_ASSET_ID::TILE_1 : TEXTURE_ASSET_ID::TILE_2);
		textures.push_back(TEXTURE_ASSET_ID::BOTTOM_WALL);
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
	if (map[row - 1][col] == (int)TILE_TYPE::EMPTY && map[row][col + 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_TOP_CORNER_WALL);
		return textures;
	}

	// Top Wall
	if (map[row - 1][col] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::WALL_SURFACE);
		textures.push_back(TEXTURE_ASSET_ID::TOP_WALL);
		return textures;
	}

	// Bot left corner
	if (map[row + 1][col] == (int)TILE_TYPE::EMPTY && map[row][col - 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Bot right corner
	if (map[row + 1][col] == (int)TILE_TYPE::EMPTY && map[row][col + 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Left wall
	if (map[row][col - 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_WALL);
		return textures;
	}

	// Right wall
	if (map[row][col + 1] == (int)TILE_TYPE::EMPTY) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_WALL);
		return textures;
	}

	// Bot wall
	if (map[row + 1][col] == (int)TILE_TYPE::EMPTY) {
		// textures.push_back((uniform_dist(rng) > 0.5) ? TEXTURE_ASSET_ID::TILE_1 : TEXTURE_ASSET_ID::TILE_2);
		textures.push_back(TEXTURE_ASSET_ID::BOTTOM_WALL);
		return textures;
	}

	// Inner top left
	if (map[row - 1][col] == (int)TILE_TYPE::WALL && map[row][col - 1] == (int)TILE_TYPE::WALL) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Inner top right
	if (map[row - 1][col] == (int)TILE_TYPE::WALL && map[row][col + 1] == (int)TILE_TYPE::WALL) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_BOTTOM_CORNER_WALL);
		return textures;
	}

	// Inner bot left
	if (map[row + 1][col] == (int)TILE_TYPE::WALL && map[row][col - 1] == (int)TILE_TYPE::WALL) {
		textures.push_back(TEXTURE_ASSET_ID::RIGHT_TOP_CORNER_WALL);
		return textures;
	}

	// Inner bot right
	if (map[row + 1][col] == (int)TILE_TYPE::WALL && map[row][col + 1] == (int)TILE_TYPE::WALL) {
		textures.push_back(TEXTURE_ASSET_ID::LEFT_TOP_CORNER_WALL);
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
			MapSystem::addTile(row, col, textureIDs, map);
		}
	}
}