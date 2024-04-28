#pragma once

#include <vector>
#include <random>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <glm/gtx/hash.hpp>
#include <limits>

#include "global.hpp"
#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "decision_tree.hpp"
#include "visibility_system.hpp"

class AISystem
{
public:
	AISystem();

	// Initialize decision trees
	void init(VisibilitySystem* visibility_arg, RenderSystem* renderer_arg);
	// Initialize flow field - call this whenever a new map is generated
	void restart_flow_field_map();
	// Returns value at specified grid coordinates
	int get_flow_field_value(vec2 grid_pos);

	void step(float elapsed_ms);
private:
	VisibilitySystem* visibility_system;
	RenderSystem* renderer;

	// Decision trees for different ai entities
	DecisionTree bee_tree;
	DecisionTree bomber_tree;
	DecisionTree wolf_tree;
	DecisionTree lizard_tree;
	DecisionTree bee2_tree;
	DecisionTree worm_tree;
	DecisionTree gargoyle_tree;
	DecisionTree cirno_boss_tree;
	DecisionTree flandre_boss_tree;
	DecisionTree sakuya_boss_tree;
	DecisionTree remilia_boss_tree;

	// Flow field for controlling crowd movement
	std::vector<std::vector<int>> flow_field_map;
	void update_flow_field_map();
	coord get_best_grid_pos(coord& grid_pos, int current_dist, int& retFlag);

	// Update flow field includes timer
	struct FlowField {
		float update_timer_ms = -1;
		float update_base = 300;
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

	// Decision tree function
	bool canSeePlayer(Entity& entity);

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist{ -1,1 }; // number between -1..1
};