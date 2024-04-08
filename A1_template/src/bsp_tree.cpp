#include "bsp_tree.hpp"
#include "world_init.hpp"

unsigned int BSPNode::count_id = 0;

BSPNode::~BSPNode() {
	delete left_node;
	delete right_node;
	delete room;
}
BSPTree::~BSPTree() { delete root; }

void BSPTree::init(vec2 max_room_size, vec2 world_size) {
	this->max_room_size = max_room_size;
	// No need to create padding yet, that will be handled when generating tiles
	root = new BSPNode(vec2(0), world_size);
	//root = new BSPNode(vec2(1), world_size - vec2(1));
	gen.seed(std::random_device{}());
}

BSPNode* BSPTree::generate_partitions(BSPNode* node) {
	if (!node) return nullptr;
	vec2 size = node->max - node->min;
	bool can_split_x = max_room_size.x * 2 < size.x;
	bool can_split_y = max_room_size.y * 2 < size.y;

	// return leaf node if can't fit two rooms in both axes
	if (!can_split_x && !can_split_y) {
		return node;
	}

	// randomly choose to split x or y
	if (can_split_x && can_split_y) {
		std::uniform_real_distribution<> real_distrib(0, 1);
		if (real_distrib(gen) < 0.5) {
			can_split_x = false;
		}
		else {
			can_split_y = false;
		}
	}

	int split;
	BSPNode* left_temp;
	BSPNode* right_temp;
	if (can_split_x) {
		std::uniform_int_distribution<> int_distrib(node->min.x + max_room_size.x, node->max.x - max_room_size.x);
		split = int_distrib(gen);
		left_temp = new BSPNode(node->min, vec2(split, node->max.y));
		right_temp = new BSPNode(vec2(split, node->min.y), node->max);
	}
	else {
		std::uniform_int_distribution<> int_distrib(node->min.y + max_room_size.y, node->max.y - max_room_size.y);
		split = int_distrib(gen);
		left_temp = new BSPNode(node->min, vec2(node->max.x, split));
		right_temp = new BSPNode(vec2(node->min.x, split), node->max);
	}

	node->left_node = generate_partitions(left_temp);
	node->right_node = generate_partitions(right_temp);

	return node;
}

void BSPTree::generate_rooms_random(BSPNode* node) {
	if (!node) return;

	if (!node->left_node && !node->right_node) {
		node->room = new Room_struct();
		// remove split between partitions
		vec2 min_temp = node->min + vec2(1);
		vec2 max_temp = node->max - vec2(1);
		vec2 size = max_temp - min_temp;
		// randomly generate room size based on constraint
		std::uniform_real_distribution<> float_distrib(0.6, 0.8);
		vec2 room_size = vec2(round(size.x * float_distrib(gen)), round(size.y * float_distrib(gen)));

		// randomly generate top left corner
		// will only generate within partition bounds to fit room
		std::uniform_int_distribution<> int_distrib_x(min_temp.x, max_temp.x - room_size.x);
		std::uniform_int_distribution<> int_distrib_y(min_temp.y, max_temp.y - room_size.y);
		node->room->top_left.x = int_distrib_x(gen);
		node->room->top_left.y = int_distrib_y(gen);
		node->room->bottom_right = node->room->top_left + room_size;

		rooms.push_back(*node->room);

		// Populates the map with floors
		for (int i = node->room->top_left.y; i <= node->room->bottom_right.y; ++i) {
			for (int j = node->room->top_left.x; j <= node->room->bottom_right.x; ++j) {
				world_map[i][j] = (int)TILE_TYPE::FLOOR;
			}
		}
	}

	generate_rooms_random(node->left_node);
	generate_rooms_random(node->right_node);
}

vec4 addSingleDoor(int row, int col, DIRECTIONS dir, int room_index, Room_struct& room, std::vector<std::vector<int>>& map) {
	map[row][col] = (int)TILE_TYPE::DOOR;
	vec4 door_info = { col, row, dir, room_index};
	//room.door_locations.push_back(door_info);
	// Creates a door
	//createDoor

	return door_info;
}

/*
* Adds doors to the grid map based on if the room edges is a wall
* MUST BE AFTER WALL GENERATION
*/
std::vector<vec4> BSPTree::generateDoorInfo(std::vector<Room_struct>& rooms, std::vector<std::vector<int>>& map) {
	std::vector<vec4> doors;

	int map_height = map.size();
	int map_width = map[0].size();
	assert(map_height > 0 && map_width > 0 && "Map should have at least one cell");
	// Create padding of empty tile in the edges by copy
	auto map_copy = std::vector<std::vector<int>>(map_height + 4, std::vector<int>(map_width + 4, 0));
	for (int y = 0; y < map_height; ++y) {
		for (int x = 0; x < map_width; ++x) {
			map_copy[y + 2][x + 2] = map[y][x];
		}
	}

	//for (Room_struct& room : rooms) 
	for(int i = 0; i < rooms.size(); i++)
	{
		Room_struct& room = rooms[i];
		// Duplicated code. A point to optimize

		// Check top of room
		//int row = room.top_left.y - 1;
		//int col = room.top_left.x - 1;
		/*
		for (int row = room.top_left.y - 1; row < room.bottom_right.y + 1; row++) {
			for (int col = room.top_left.x - 1 ; col < room.bottom_right.x + 1; col++) {
				if (map[row][col] == (int)TILE_TYPE::FLOOR) {
					//addSingleDoor(row, col, 0, room, map);
				}
				// Checks only the edges of each room
				// Ignores interriors
				if (row != room.top_left.y - 1 || row != room.bottom_right.y + 1 &&
					map[row][room.bottom_right.x+1] == (int)TILE_TYPE::FLOOR) {
					//addSingleDoor(row, col, 0, room, map);
					row++; // Skips current row
					col = room.bottom_right.x + 1;
				}
			}
		}*/
			// Checks top of room
			for (int col = room.top_left.x - 1 + 2; col <= room.bottom_right.x + 1 + 2; col++) {
				if (map_copy[room.top_left.y - 1 + 2][col] == (int)TILE_TYPE::FLOOR) {
					// Found a floor tile on the outer edges of the room
					doors.push_back(addSingleDoor(room.top_left.y - 1, col - 2, DIRECTIONS::DOWN, i, room, map_copy));

				}
			}
			// Check bottom of room
			//row = room.bottom_right.y + 1;
			for (int col = room.top_left.x - 1 + 2; col <= room.bottom_right.x + 1 + 2; col++) {
				if (map_copy[room.top_left.y - 1 + 2][col] == (int)TILE_TYPE::FLOOR) {
					// Found a floor tile on the outer edges of the room
					doors.push_back(addSingleDoor(room.top_left.y - 1, col - 2, DIRECTIONS::UP, i, room, map_copy));
				}
			}
			// Check left of room
			//col = room.top_left.x - 1;
			for (int row = room.top_left.y - 1 + 2; row <= room.bottom_right.y + 1 + 2; row++) {
				if (map_copy[row][room.top_left.x - 1 + 2] == (int)TILE_TYPE::FLOOR) {
					// Found a floor tile on the outer edges of the room
					doors.push_back(addSingleDoor(row - 2, room.top_left.x - 1, DIRECTIONS::RIGHT, i, room, map_copy));

				}
			}
			// Check right of room
			//col = room.bottom_right.x + 1;
			for (int row = room.top_left.y - 1 + 2; row <= room.bottom_right.y + 1 + 2; row++) {
				if (map_copy[row][room.top_left.x - 1 + 2] == (int)TILE_TYPE::FLOOR) {
					// Found a floor tile on the outer edges of the room
					doors.push_back(addSingleDoor(row - 2, room.top_left.x - 1, DIRECTIONS::LEFT, i, room, map_copy));
				}
			}
	}

	return doors;
}

void BSPTree::generate_corridors(BSPNode* node) {
	if (!node) return;

	generate_corridors(node->left_node);
	generate_corridors(node->right_node);

	if (node->left_node && node->right_node) {
		BSPNode* rNode = get_random_leaf_node(node->right_node);
		BSPNode* lNode = get_random_leaf_node(node->left_node);

		std::uniform_int_distribution<> int_distrib_rx(rNode->room->top_left.x, rNode->room->bottom_right.x);
		std::uniform_int_distribution<> int_distrib_ry(rNode->room->top_left.y, rNode->room->bottom_right.y);
		std::uniform_int_distribution<> int_distrib_lx(lNode->room->top_left.x, lNode->room->bottom_right.x);
		std::uniform_int_distribution<> int_distrib_ly(lNode->room->top_left.y, lNode->room->bottom_right.y);

		// random point in both room gives more variety
		vec2 start = vec2(int_distrib_rx(rng), int_distrib_ry(rng));
		vec2 end = vec2(int_distrib_lx(rng), int_distrib_ly(rng));

		// center point of both room
		//vec2 start = vec2((rNode->room->bottom_left + rNode->room->top_left) / 2.f);
		//vec2 end = vec2((lNode->room->bottom_left + lNode->room->top_left) / 2.f);

		generate_corridor_between_two_points(start, end);
	}
}

void BSPTree::generate_corridor_between_two_points(vec2 start, vec2 end) {
	const path optimal_path = astar(start, end);
	coord next_pos;
	for (int i = 0; i < optimal_path.size(); ++i) {
		next_pos = optimal_path[i];
		world_map[next_pos.y][next_pos.x] = (int)TILE_TYPE::FLOOR;
	}
}

BSPNode* BSPTree::get_random_leaf_node(BSPNode* node) {
	if (!node) return nullptr;

	if (!node->left_node && !node->right_node) {
		return node;
	}

	std::uniform_real_distribution<> float_distrib(0, 1);
	return float_distrib(gen) < 0.5 ? get_random_leaf_node(node->left_node) : get_random_leaf_node(node->right_node);
}

void BSPTree::set_map_walls(std::vector<std::vector<int>>& map) {
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

void BSPTree::print_tree(BSPNode* node) {
	if (!node) return;

	if (!node->left_node && !node->right_node) {
		printf("id %d\n", node->id);
		printf("min: (%f,%f)\n", node->min.x, node->min.y);
		printf("max: (%f,%f)\n", node->max.x, node->max.y);
		if (node->room) {
			printf("top_left: (%f,%f)\n", node->room->top_left.x, node->room->top_left.y);
		}
		printf("left_node: %d\n", node->left_node ? 1 : 0);
		printf("right_node: %d\n", node->right_node ? 1 : 0);
		printf("=========\n");
	}

	print_tree(node->left_node);
	print_tree(node->right_node);
}

