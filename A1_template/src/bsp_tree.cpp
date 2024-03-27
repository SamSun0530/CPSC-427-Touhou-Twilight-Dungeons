#include "bsp_tree.hpp"

unsigned int BSPNode::count_id = 0;

BSPNode::~BSPNode() {
	delete left_node;
	delete right_node;
	delete room;
}
BSPTree::~BSPTree() { delete root; }

void BSPTree::init(vec2 max_room_size, vec2 world_size) {
	this->max_room_size = max_room_size;
	root = new BSPNode(vec2(1, 1), world_size - vec2(1)); // To create a space between world edge and map
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
		node->room = new Room2();
		// remove split between partitions
		vec2 min_temp = node->min + vec2(1);
		vec2 max_temp = node->max - vec2(1);
		vec2 size = max_temp - min_temp;
		// randomly generate room size based on constraint
		std::uniform_real_distribution<> float_distrib(0.6, 0.9);
		vec2 room_size = vec2(size.x * float_distrib(gen), size.y * float_distrib(gen));

		// randomly generate top left corner
		// will only generate within partition bounds to fit room
		std::uniform_int_distribution<> int_distrib_x(min_temp.x, max_temp.x - room_size.x);
		std::uniform_int_distribution<> int_distrib_y(min_temp.y, max_temp.y - room_size.y);
		node->room->top_left.x = int_distrib_x(gen);
		node->room->top_left.y = int_distrib_y(gen);
		node->room->bottom_left = node->room->top_left + room_size;
	}

	generate_rooms_random(node->left_node);
	generate_rooms_random(node->right_node);
}

void BSPTree::generate_corridors(BSPNode* node) {
	if (!node) return;

	generate_corridors(node->left_node);
	generate_corridors(node->right_node);

	if (node->left_node && node->right_node) {
		BSPNode* rNode = get_random_leaf_node(node->right_node);
		BSPNode* lNode = get_random_leaf_node(node->left_node);

		node->corridor = new Corridor{ vec2((rNode->room->bottom_left + rNode->room->top_left) / vec2(2,2)),
							  vec2((lNode->room->bottom_left + lNode->room->top_left) / vec2(2,2)) };
	}
}

	
void BSPTree::add_corridors_to_map(std::vector<Corridor> corridors, std::vector<std::vector<int>>& map) {
	for (Corridor& corridor : corridors) {
		int start_x = corridor.start.x;
		int start_y = corridor.start.y;
		int end_x = corridor.end.x;
		int end_y = corridor.end.y;

		if (start_x > end_x) {
			if (start_y > end_y) {
				// Start is bot right of end
				for (int col = end_x; col <= start_x; col++) {
					map[end_y][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile
					map[end_y+1][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile

				}
				for (int row = end_y; row <= start_y; row++) {
					map[row][start_x] = (int)TILE_TYPE::FLOOR;
					map[row][start_x+1] = (int)TILE_TYPE::FLOOR;

				}
			}
			else if (start_y < end_y) {
				// Start is top right of end
				for (int col = end_x; col <= start_x; col++) {
					map[end_y][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile
					map[end_y + 1][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile

				}
				for (int row = end_y; row >= start_y; row--) {
					map[row][start_x] = (int)TILE_TYPE::FLOOR;
					map[row][start_x + 1] = (int)TILE_TYPE::FLOOR;

				}
			}
			else { // start_y == end_y
				// Start is directly right of end
				for (int col = end_x; col <= start_x; col++) {
					map[end_y][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile
					map[end_y + 1][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile

				}
			}
		}
		else if(start_x < end_x)
		{
			if (start_y > end_y) {
				// Start is bot left of end
				for (int col = end_x; col >= start_x; col--) {
					map[end_y][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile
					map[end_y + 1][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile

				}
				for (int row = end_y; row <= start_y; row++) {
					map[row][start_x] = (int)TILE_TYPE::FLOOR;
					map[row][start_x - 1] = (int)TILE_TYPE::FLOOR;

				}
			}
			else if (start_y < end_y) {
				// Start is top left of end
				for (int col = end_x; col >= start_x; col--) {
					map[end_y][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile
					map[end_y + 1][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile

				}
				for (int row = end_y; row >= start_y; row--) {
					map[row][start_x] = (int)TILE_TYPE::FLOOR;
					map[row][start_x - 1] = (int)TILE_TYPE::FLOOR;

				}
			}
			else { // start_y == end_y
				// Start is directly left of end
				for (int col = end_x; col >= start_x; col--) {
					map[end_y][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile
					map[end_y + 1][col] = (int)TILE_TYPE::FLOOR; // Sets floor tile

				}
			}
		}
		else { // start_x == end_x
			if (start_y > end_y) {
				// Start is bot of end
				for (int row = end_y; row <= start_y; row++) {
					map[row][start_x] = (int)TILE_TYPE::FLOOR;
					map[row][start_x - 1] = (int)TILE_TYPE::FLOOR;

				}
			}
			else if (start_y < end_y) {
				// Start is top of end
				for (int row = end_y; row >= start_y; row--) {
					map[row][start_x] = (int)TILE_TYPE::FLOOR;
					map[row][start_x - 1] = (int)TILE_TYPE::FLOOR;

				}
			}
			else { // start_y == end_y
				// Start is directly on top of end
				// This is bad
			}
		}
	}
}

void BSPTree::get_corridors(BSPNode* node, std::vector<Corridor>& corridors) {
	if (!node) return;

	if (node->corridor)
		corridors.push_back(*node->corridor);

	get_corridors(node->left_node, corridors);
	get_corridors(node->right_node, corridors);
}

void BSPTree::get_rooms(BSPNode* node, std::vector<Room2>& rooms) {
	if (!node) return;

	if (!node->left_node && !node->right_node) {
		rooms.push_back(*node->room);
	}

	get_rooms(node->left_node, rooms);
	get_rooms(node->right_node, rooms);
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
	for (int row = 1; row < map.size()-1; row++) {
		for (int col = 1; col < map[row].size()-1; col++) {
			if(map[row][col] != (int)TILE_TYPE::EMPTY) {
				continue; // Can't pull walls on floors
			}

			if (map[row][col + 1] == (int)TILE_TYPE::FLOOR || map[row][col - 1] == (int)TILE_TYPE::FLOOR ||
				map[row + 1][col] == (int)TILE_TYPE::FLOOR || map[row - 1][col] == (int)TILE_TYPE::FLOOR ||
				map[row + 1][col + 1] == (int)TILE_TYPE::FLOOR || map[row + 1][col - 1] == (int)TILE_TYPE::FLOOR ||
				map[row - 1][col + 1] == (int)TILE_TYPE::FLOOR || map[row - 1][col - 1] == (int)TILE_TYPE::FLOOR) {
				map[row][col] = (int)TILE_TYPE::WALL; // Adds wall to a tile adjacent to a floor
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

