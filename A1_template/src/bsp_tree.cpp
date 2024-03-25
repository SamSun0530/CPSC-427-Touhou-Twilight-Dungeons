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
	root = new BSPNode(vec2(0, 0), world_size);
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
		vec2 max_temp = node->max + vec2(1);
		vec2 size = max_temp - min_temp;
		// randomly generate room size based on constraint
		std::uniform_real_distribution<> float_distrib(0.6, 0.9);
		vec2 room_size = vec2(size.x * float_distrib(gen), size.y * float_distrib(gen));

		// randomly generate top left corner
		// will only generate within partition bounds to fit room
		std::uniform_int_distribution<> int_distrib_x(min_temp.x, max_temp.x - room_size.x - 1);
		std::uniform_int_distribution<> int_distrib_y(min_temp.y, max_temp.y - room_size.y - 1);
		node->room->top_left.x = int_distrib_x(gen);
		node->room->top_left.y = int_distrib_y(gen);
		node->room->bottom_left = node->room->top_left + room_size;
	}

	generate_rooms_random(node->left_node);
	generate_rooms_random(node->right_node);
}

void BSPTree::get_rooms(BSPNode* node, std::vector<Room2>& rooms) {
	if (!node) return;

	if (!node->left_node && !node->right_node) {
		rooms.push_back(*node->room);
	}

	get_rooms(node->left_node, rooms);
	get_rooms(node->right_node, rooms);
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

