#pragma once

#include <stack>

#include "common.hpp"
#include "components.hpp"

struct Corridor {
	vec2 start;
	vec2 end;
};

class BSPNode {
public:
	unsigned int id;
	static unsigned int count_id;
	// Current partition cell grid position
	vec2 min;
	vec2 max;

	// Room associated with this node
	Room2* room = nullptr;
	Corridor* corridor = nullptr;

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
	BSPNode* root = nullptr;
	~BSPTree();
	void init(vec2 max_room_size, vec2 world_size);
	/*
	Steps in order:
	(1) Generate partitions
	(2) Generate rooms (either random/premade)
	(3) Generate corridors/hallways
	*/
	BSPNode* generate_partitions(BSPNode* node);
	void generate_rooms_random(BSPNode* node);
	void generate_corridors(BSPNode* node);
	void add_corridors_to_map(std::vector<Corridor> corridors, std::vector<std::vector<int>>& map);
	void set_map_walls(std::vector<std::vector<int>>& map);
	void print_tree(BSPNode* node); // for debugging
	void get_corridors(BSPNode* node, std::vector<Corridor>& corridors);

	void get_rooms(BSPNode* node, std::vector<Room2>& rooms);
	BSPNode* get_random_leaf_node(BSPNode* node);

private:
	// Utilities:
	std::mt19937 gen;
};