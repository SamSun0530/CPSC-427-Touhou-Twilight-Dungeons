#pragma once

#include <stack>
#include <vector>
#include <random>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <glm/gtx/hash.hpp>
#include <limits>

#include "common.hpp"
#include "components.hpp"
#include <iostream>
#include "global.hpp"


class BSPNode {
public:
	unsigned int id;
	static unsigned int count_id;
	// Current partition cell grid position
	vec2 min;
	vec2 max;

	// Room associated with this node
	Room_struct* room = nullptr;

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

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
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
	void generate_rooms_random(BSPNode* node); // sets world tiles when called
	void generate_corridors(BSPNode* node); // sets world tiles when called
	void generate_corridor_between_two_points(vec2 start, vec2 end);
	BSPNode* generate_partitions(BSPNode* node);
	void print_tree(BSPNode* node); // for debugging
	BSPNode* get_random_leaf_node(BSPNode* node);
	std::vector<Room_struct> rooms;

private:
	// Utilities:
	std::mt19937 gen;

	// Modified A-star for corridors in an attempt to solve overlap
	// Taken from ai_system_init.cpp

	float astar_heuristic(coord n, coord goal) {
		// Manhattan - 4 direction
		return abs(goal.x - n.x) + abs(goal.y - n.y);
	}

	// get all possible actions and their costs (1 for direct, 1.4 for diagonal since sqrt(2)=1.4)
	// Bug (potential): entities should not search diagonal when there are walls there
	// Consider the scenario where a wall is to the left and below the entity, the entity should not check through the cracks
	// This can be fixed if we seal the cracks
	std::vector<std::pair<float, coord>> astar_actioncosts() {
		return (std::vector<std::pair<float, coord>>{
			std::make_pair(1.f, vec2(0, -1)), // UP
				std::make_pair(1.f, vec2(0, 1)), // DOWN
				std::make_pair(1.f, vec2(-1, 0)), // LEFT
				std::make_pair(1.f, vec2(1, 0)), // RIGHT
		});
	}

	// backtrack and reconstruct the path stored in came_from
	path reconstruct_path(std::unordered_map<coord, coord>& came_from, coord& current, coord& start) {
		path optimal_path;
		while (current != start) {
			optimal_path.push_back(current);
			current = came_from[current];
		}

		// Debug line by visualizing the path
		//for (int i = 0; i < optimal_path.size(); i++) {
		//	createLine(convert_grid_to_world(optimal_path[i]), vec2(world_tile_size / 2));
		//}

		std::reverse(optimal_path.begin(), optimal_path.end());
		return optimal_path;
	}

	// comparator for sorting min heap
	struct CompareGreater
	{
		bool operator()(const std::pair<float, coord>& l, const std::pair<float, coord>& r) const { return l.first > r.first; }
	};

	// set default value of infinity
	struct GScore
	{
		float score = std::numeric_limits<float>::max();
		operator float() { return score; }
		void operator=(float x) { score = x; }
	};

	bool is_out_of_bounds(int x, int y) {
		return !(y < 0 || x < 0 || y >= world_height || x >= world_width
			|| world_map[y][x] == (int)TILE_TYPE::WALL);
	}

	// A-star path finding algorithm (does not store paths inside frontier)
	// Adapted from: https://en.wikipedia.org/wiki/A*_search_algorithm
	// Note: parameter coords are in grid coordinates NOT world coordinates
	// e.g. input (x,y) should be (1,1) on the grid instead of (550.53, -102.33)
	path astar(coord start, coord goal) {
		// unlikely to find path between floats, so round
		start = round(start);
		goal = round(goal);
		std::priority_queue<std::pair<float, coord>, std::vector<std::pair<float, coord>>, CompareGreater> open_list;
		std::unordered_set<coord> close_list; // visited set
		std::unordered_map<coord, coord> came_from;
		std::unordered_map<coord, GScore> g_score;

		g_score[start] = 0.f;
		open_list.push(std::make_pair(astar_heuristic(start, goal), start));

		while (!open_list.empty()) {
			std::pair<float, coord> current = open_list.top();
			if (current.second == goal) return reconstruct_path(came_from, current.second, start);
			open_list.pop();
			close_list.insert(current.second);
			for (std::pair<float, coord>& actioncost : astar_actioncosts()) {
				vec2 candidate = current.second + actioncost.second;
				if (close_list.count(candidate) || (!is_out_of_bounds(candidate.x, candidate.y))) continue;
				// Tunnel through empty space only if necessary
				// Idea from: https://waxproject.blogspot.com/2020/03/procedural-dungeon-generation.html
				float cost = world_map[candidate.y][candidate.x] == (int)TILE_TYPE::EMPTY ? 100.f : 1.f;
				float candidate_g = current.first - astar_heuristic(current.second, goal) + cost;
				if (candidate_g < g_score[candidate] || g_score.find(candidate) == g_score.end()) {
					came_from[candidate] = current.second;
					g_score[candidate] = candidate_g;
					open_list.push(std::make_pair(candidate_g + astar_heuristic(candidate, goal), candidate));
				}
			}
		}
		return path();
	}
};