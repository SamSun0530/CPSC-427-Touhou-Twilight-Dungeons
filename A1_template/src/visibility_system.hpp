#pragma once

#include <deque>
#include <unordered_set>
#include <glm/gtx/hash.hpp>

#include "common.hpp"
#include "components.hpp"
#include "global.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"

enum class VISIBILITY_STATE {
	VISIBLE, // 0
	NOT_VISIBLE // 1
};

class VisibilitySystem {
public:
	// copy of the world_map size
	// 0 - visible, no quad rendered on top
	// 1 - not visible, black quad rendered on top
	std::vector<std::vector<int>> map;
	// copy of the world_map size
	// mapping between map and entity id, allows for deletion
	// each cell is either:
	// -1 - no entity
	// number != -1 - entity id
	std::vector<std::vector<int>> reference_map;

	// restart visibility and reference maps to default 0 and -1 respectively
	// IMPORTANT ORDER OF OPERATIONS:
	// (1) Call VisibilitySystem::restart_map -> map to 0, reference_map to -1
	// (2) Call MapSystem::generate_all_tiles -> createTile -> populates reference_map with entities
	// (3) Call VisibilitySystem::init_visibility -> populates map with 1 for floors/walls
	// (4) Call RenderSystem::set_visibility_tiles_instance_buffer_max (takes info from (2))
	// Note: 
	// - Calling VisibilitySystem::restart_map not at (1) will result in error
	// - (3) and (4) are interchangeable
	void restart_map();
	// sets 1s and 0s based on world_map
	void init_visibility();
	void step(float elapsed_ms);
	void init(RenderSystem* renderer_arg);

	// Utilities
	void print_visibility_map();
	void print_reference_map();
private:
	// BFS floodfill
	// keeps track of next grid positions to reveal and number of tiles
	std::deque<coord> next_pos;
	int next_num = 0; // number of neighbors added
	int curr_num = 0; // current numbers left
	std::unordered_set<coord> close_list; // prevent rechecking tiles already in deque

	// list of actions for getting neighbor cells - usually for rooms
	const std::vector<coord> ACTIONS = {
		vec2(0, -1),	// UP
		vec2(0, 1),		// DOWN
		vec2(-1, 0),	// LEFT
		vec2(1, 0),		// RIGHT
	};

	// with diagonals - usually for door candidates
	const std::vector<coord> ACTIONS_DIAGONALS = {
		vec2(0, -1),	// UP
		vec2(0, 1),		// DOWN
		vec2(-1, 0),	// LEFT
		vec2(1, 0),		// RIGHT
		vec2(-1, -1),	// UP LEFT
		vec2(1, -1),	// UP RIGHT
		vec2(-1, 1),	// DOWN LEFT
		vec2(1, 1)		// DOWN RIGHT
	};

	// used for stopping floodfill when player is in a corridor
	bool is_door_found = false;

	// Limit frequency of flood fill
	float counter_ms = 0;
	float counter_ms_default = 60;

	// set tile to be visible by removing visibility tile
	void set_tile_visible(coord grid_pos);

	// Game state
	RenderSystem* renderer;
};