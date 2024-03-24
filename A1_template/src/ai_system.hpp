#pragma once

#include <vector>
#include <random>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <glm/gtx/hash.hpp>
#include <limits>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "decision_tree.hpp"

class AISystem
{
public:
	AISystem();

	// Initialize decision trees
	void init();
	// Initialize flow field - call this whenever a new map is generated
	void restart_flow_field_map();

	void step(float elapsed_ms);
private:
	// Decision trees for different ai entities
	DecisionTree bee_tree;
	DecisionTree bomber_tree;
	DecisionTree wolf_tree;

	// Flow field for controlling crowd movement
	std::vector<std::vector<int>> flow_field_map;
	void update_flow_field_map();

	// Update flow field includes timer
	struct FlowField {
		float update_timer_ms = -1;
		float update_base = 1000;
		// don't update if grid position hasn't moved
		vec2 last_position = { 0, 0 };
		// maximum length to expand
		int max_length = 15;
	} flow_field;

	// actions to take
	const std::vector<coord> ACTIONS = {
		vec2(0, -1),	// UP
		vec2(0, 1),		// DOWN
		vec2(-1, 0),	// LEFT
		vec2(1, 0),		// RIGHT
		//vec2(-1, -1),	// UP LEFT
		//vec2(1, -1),	// UP RIGHT
		//vec2(-1, 1),	// DOWN LEFT
		//vec2(1, 1)		// DOWN RIGHT
	};

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist{ -1,1 }; // number between -1..1
};