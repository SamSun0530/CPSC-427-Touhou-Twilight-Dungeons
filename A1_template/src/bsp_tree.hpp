#pragma once

#include <stack>

#include "common.hpp"

class BSPNode {
public:
	unsigned int id;
	static unsigned int count_id;
	// Current partition cell grid position
	vec2 min;
	vec2 max;

	BSPNode* left_node = nullptr;
	BSPNode* right_node = nullptr;

	BSPNode(vec2 min, vec2 max) : min(min), max(max) {
		id = count_id++;
	};
	~BSPNode();
};

class BSPTree {
private:
	// Maximum room size
	vec2 max_room_size;

public:
	BSPNode* root;
	~BSPTree();
	void init(vec2 max_room_size, vec2 world_size);
	BSPNode* BSPTree::generate_partitions(BSPNode* node);
	void print_tree(BSPNode* node); // for debugging

private:
	// Utilities:
	std::mt19937 gen;
};